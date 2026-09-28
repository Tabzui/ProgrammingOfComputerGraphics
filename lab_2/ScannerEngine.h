#pragma once
#include <QObject>
#include <QRunnable>
#include <QStringList>
#include "ImageInfo.h"
#include "Parsers.h"

class ScanTask : public QRunnable {
public:
    ScanTask(const QString& path, QObject* receiver);
    void run() override;
private:
    QString m_path;
    QObject* m_receiver;
};

class ScannerEngine : public QObject {
    Q_OBJECT
public:
    explicit ScannerEngine(QObject *parent = nullptr);
    void startScan(const QString& folderPath);

signals:
    void fileProcessed(const ImageInfo& info);
    void progressUpdated(int current, int total);
    void scanFinished();

public slots:
    void onTaskCompleted(const ImageInfo& info);

private:
    int m_totalFiles;
    int m_processedFiles;
};
