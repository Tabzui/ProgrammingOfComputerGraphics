#include "ScannerEngine.h"
#include <QDirIterator>
#include <QThreadPool>
#include <QFileInfo>

ScanTask::ScanTask(const QString& path, QObject* receiver)
    : m_path(path), m_receiver(receiver) {
    setAutoDelete(true);
}

void ScanTask::run() {
    QFileInfo fi(m_path);
    ImageParser* parser = ParserFactory::getParser(fi.suffix());
    ImageInfo info;
    if (parser) {
        info = parser->parse(m_path);
        delete parser;
    } else {
        info.fileName = fi.fileName();
        info.status = "Формат не поддерживается";
    }

    QMetaObject::invokeMethod(m_receiver, "onTaskCompleted", Q_ARG(ImageInfo, info));
}

ScannerEngine::ScannerEngine(QObject *parent) : QObject(parent), m_totalFiles(0), m_processedFiles(0) {
    qRegisterMetaType<ImageInfo>("ImageInfo");
}

void ScannerEngine::startScan(const QString& folderPath) {
    m_processedFiles = 0;
    QStringList filters = {"*.jpg", "*.jpeg", "*.png", "*.bmp", "*.gif", "*.tif", "*.pcx"};
    QDirIterator it(folderPath, filters, QDir::Files, QDirIterator::Subdirectories);

    QStringList filesToProcess;
    while (it.hasNext()) {
        filesToProcess.append(it.next());
    }

    m_totalFiles = filesToProcess.size();
    if (m_totalFiles == 0) {
        emit scanFinished();
        return;
    }

    emit progressUpdated(0, m_totalFiles);

    for (const QString& file : filesToProcess) {
        QThreadPool::globalInstance()->start(new ScanTask(file, this));
    }
}

void ScannerEngine::onTaskCompleted(const ImageInfo& info) {
    emit fileProcessed(info);
    m_processedFiles++;
    emit progressUpdated(m_processedFiles, m_totalFiles);
    if (m_processedFiles >= m_totalFiles) {
        emit scanFinished();
    }
}
