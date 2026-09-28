#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QLabel>
#include "ScannerEngine.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void selectFolder();
    void addImageInfo(const ImageInfo& info);
    void updateProgress(int current, int total);
    void scanDone();

private:
    QTableWidget* m_table;
    QProgressBar* m_progress;
    QPushButton* m_btnScan;
    ScannerEngine* m_engine;
};
