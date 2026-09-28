#include "Parsers.h"
#include <QFileInfo>
#include <QDataStream>

qint32 ImageParser::readInt32LE(QFile& file) {
    qint32 val; file.read(reinterpret_cast<char*>(&val), 4); return val;
}
qint32 ImageParser::readInt32BE(QFile& file) {
    unsigned char b[4]; file.read(reinterpret_cast<char*>(b), 4);
    return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}
qint16 ImageParser::readInt16LE(QFile& file) {
    qint16 val; file.read(reinterpret_cast<char*>(&val), 2); return val;
}
qint16 ImageParser::readInt16BE(QFile& file) {
    unsigned char b[2]; file.read(reinterpret_cast<char*>(b), 2);
    return (b[0] << 8) | b[1];
}

ImageInfo BmpParser::parse(const QString& filePath) {
    ImageInfo info;
    info.fileName = QFileInfo(filePath).fileName();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        info.status = "Ошибка доступа"; return info;
    }

    char magic[2];
    if (file.read(magic, 2) != 2 || magic[0] != 'B' || magic[1] != 'M') {
        info.status = "Файл поврежден (неверная сигнатура BMP)"; return info;
    }

    qint32 fileSize = readInt32LE(file);
    if (file.size() < fileSize) {
        info.status = "Файл поврежден (размер меньше заявленного)"; return info;
    }

    file.seek(18);
    info.width = readInt32LE(file);
    info.height = readInt32LE(file);
    file.seek(28);
    info.depth = readInt16LE(file);

    qint32 compMethod = readInt32LE(file);
    switch (compMethod) {
    case 0: info.compression = "BI_RGB (Без сжатия)"; break;
    case 1: info.compression = "BI_RLE8"; break;
    case 2: info.compression = "BI_RLE4"; break;
    case 3: info.compression = "BI_BITFIELDS"; break;
    default: info.compression = "Неизвестно"; break;
    }
    return info;
}

ImageInfo PngParser::parse(const QString& filePath) {
    ImageInfo info;
    info.fileName = QFileInfo(filePath).fileName();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        info.status = "Ошибка доступа"; return info;
    }

    unsigned char magic[8];
    if (file.read(reinterpret_cast<char*>(magic), 8) != 8 || magic[0] != 0x89 || magic[1] != 0x50) {
        info.status = "Файл поврежден (неверная сигнатура PNG)"; return info;
    }


    file.seek(file.size() - 8);
    char iend[4];
    file.read(iend, 4);
    if (qstrncmp(iend, "IEND", 4) != 0) {
        info.status = "Файл поврежден (отсутствует IEND)"; return info;
    }

    file.seek(8);
    qint32 chunkLen = readInt32BE(file);
    char chunkType[5] = {0};
    file.read(chunkType, 4);

    if (qstrcmp(chunkType, "IHDR") == 0) {
        info.width = readInt32BE(file);
        info.height = readInt32BE(file);
        info.depth = file.read(1)[0];
        info.compression = "Deflate";
    }
    return info;
}

ImageInfo JpgParser::parse(const QString& filePath) {
    ImageInfo info;
    info.fileName = QFileInfo(filePath).fileName();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return info;

    unsigned char magic[2];
    if (file.read(reinterpret_cast<char*>(magic), 2) != 2 || magic[0] != 0xFF || magic[1] != 0xD8) {
        info.status = "Файл поврежден (неверная сигнатура JPG)"; return info;
    }

    file.seek(file.size() - 2);
    if (file.read(reinterpret_cast<char*>(magic), 2) != 2 || magic[0] != 0xFF || magic[1] != 0xD9) {
        info.status = "Файл поврежден (отсутствует EOI)"; return info;
    }

    file.seek(2);
    info.compression = "JPEG (Lossy)";


    while (!file.atEnd()) {
        if (file.read(reinterpret_cast<char*>(magic), 2) != 2) break;
        if (magic[0] != 0xFF) break;

        if (magic[1] >= 0xC0 && magic[1] <= 0xC3) {
            readInt16BE(file);
            info.depth = file.read(1)[0] * 8;
            info.height = readInt16BE(file);
            info.width = readInt16BE(file);
            break;
        } else {
            qint16 len = readInt16BE(file);
            file.seek(file.pos() + len - 2);
        }
    }
    return info;
}

ImageInfo GifParser::parse(const QString& filePath) {
    ImageInfo info;
    info.fileName = QFileInfo(filePath).fileName();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return info;

    char magic[6];
    if (file.read(magic, 6) != 6 || qstrncmp(magic, "GIF", 3) != 0) {
        info.status = "Файл поврежден"; return info;
    }

    info.width = readInt16LE(file);
    info.height = readInt16LE(file);

    char packed;
    file.read(&packed, 1);
    info.depth = ((packed & 0x70) >> 4) + 1;
    info.compression = "LZW";
    return info;
}

ImageParser* ParserFactory::getParser(const QString& extension) {
    QString ext = extension.toLower();
    if (ext == "bmp") return new BmpParser();
    if (ext == "png") return new PngParser();
    if (ext == "jpg" || ext == "jpeg") return new JpgParser();
    if (ext == "gif") return new GifParser();
    return nullptr;
}
