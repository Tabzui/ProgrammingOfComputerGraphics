#include "MainWindow.h"
#include <QVBoxLayout>
#include <QFileDialog>
#include <QHeaderView>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QWidget* central = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(central);

    m_btnScan = new QPushButton("Выбрать папку и начать сканирование", this);
    m_progress = new QProgressBar(this);
    m_progress->setValue(0);

    m_table = new QTableWidget(0, 6, this);
    m_table->setHorizontalHeaderLabels({
        "Имя файла", "Размер", "DPI", "Глубина цвета", "Сжатие", "Статус"
    });
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setSortingEnabled(true);

    layout->addWidget(m_btnScan);
    layout->addWidget(m_progress);
    layout->addWidget(m_table);
    setCentralWidget(central);
    resize(900, 600);

    m_engine = new ScannerEngine(this);
    connect(m_btnScan, &QPushButton::clicked, this, &MainWindow::selectFolder);
    connect(m_engine, &ScannerEngine::fileProcessed, this, &MainWindow::addImageInfo);
    connect(m_engine, &ScannerEngine::progressUpdated, this, &MainWindow::updateProgress);
    connect(m_engine, &ScannerEngine::scanFinished, this, &MainWindow::scanDone);
}

void MainWindow::selectFolder() {
    QString dir = QFileDialog::getExistingDirectory(this, "Выберите папку с изображениями");
    if (!dir.isEmpty()) {
        m_table->setSortingEnabled(false);
        m_table->setRowCount(0);
        m_btnScan->setEnabled(false);
        m_progress->setValue(0);
        m_engine->startScan(dir);
    }
}

void MainWindow::addImageInfo(const ImageInfo& info) {
    int row = m_table->rowCount();
    m_table->insertRow(row);

    m_table->setItem(row, 0, new QTableWidgetItem(info.fileName));

    QString sizeStr = (info.width > 0 && info.height > 0)
                          ? QString("%1x%2").arg(info.width).arg(info.height)
                          : "-";
    m_table->setItem(row, 1, new QTableWidgetItem(sizeStr));

    m_table->setItem(row, 2, new QTableWidgetItem(
                                 info.dpi > 0 ? QString::number(info.dpi) : "-"));

    m_table->setItem(row, 3, new QTableWidgetItem(
                                 info.depth > 0 ? QString::number(info.depth) + " бит" : "-"));

    m_table->setItem(row, 4, new QTableWidgetItem(info.compression));

    QTableWidgetItem* statusItem = new QTableWidgetItem(info.status);
    if (info.status != "ОК") statusItem->setForeground(Qt::red);
    m_table->setItem(row, 5, statusItem);
}

void MainWindow::updateProgress(int current, int total) {
    m_progress->setMaximum(total);
    m_progress->setValue(current);
}

void MainWindow::scanDone() {
    m_btnScan->setEnabled(true);
    m_table->setSortingEnabled(true);
}
