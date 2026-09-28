#pragma once
#include <QString>
#include <QFile>
#include "ImageInfo.h"

class ImageParser {
public:
    virtual ~ImageParser() = default;
    virtual ImageInfo parse(const QString& filePath) = 0;
protected:
    static qint32 readInt32LE(QFile& file);
    static qint32 readInt32BE(QFile& file);
    static qint16 readInt16LE(QFile& file);
    static qint16 readInt16BE(QFile& file);
};

class BmpParser : public ImageParser {
public:
    ImageInfo parse(const QString& filePath) override;
};

class PngParser : public ImageParser {
public:
    ImageInfo parse(const QString& filePath) override;
};

class JpgParser : public ImageParser {
public:
    ImageInfo parse(const QString& filePath) override;
};

class GifParser : public ImageParser {
public:
    ImageInfo parse(const QString& filePath) override;
};

class ParserFactory {
public:
    static ImageParser* getParser(const QString& extension);
};
