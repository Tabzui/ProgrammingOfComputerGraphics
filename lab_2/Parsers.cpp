#include "Parsers.h"
#include <QFileInfo>
#include <QDataStream>

qint32 ImageParser::readInt32LE(QFile& file) {
    unsigned char b[4];
    if (file.read(reinterpret_cast<char*>(b), 4) != 4) return 0;
    return (b[0]) | (b[1] << 8) | (b[2] << 16) | (b[3] << 24);
}

qint32 ImageParser::readInt32BE(QFile& file) {
    unsigned char b[4];
    if (file.read(reinterpret_cast<char*>(b), 4) != 4) return 0;
    return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}

qint16 ImageParser::readInt16LE(QFile& file) {
    unsigned char b[2];
    if (file.read(reinterpret_cast<char*>(b), 2) != 2) return 0;
    return (b[0]) | (b[1] << 8);
}

qint16 ImageParser::readInt16BE(QFile& file) {
    unsigned char b[2];
    if (file.read(reinterpret_cast<char*>(b), 2) != 2) return 0;
    return (b[0] << 8) | b[1];
}

ImageInfo BmpParser::parse(const QString& filePath) {
    ImageInfo info;
    info.fileName = QFileInfo(filePath).fileName();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        info.status = "Ошибка доступа";
        return info;
    }

    if (file.size() < 54) {
        info.status = "Файл поврежден (слишком короткий BMP)";
        return info;
    }

    char magic[2];
    if (file.read(magic, 2) != 2 || magic[0] != 'B' || magic[1] != 'M') {
        info.status = "Файл поврежден (неверная сигнатура BMP)";
        return info;
    }

    qint32 fileSize = readInt32LE(file);
    if (fileSize <= 0 || file.size() < fileSize) {
        info.status = "Файл поврежден (размер меньше заявленного)";
        return info;
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

    if (info.width < 0) info.width = -info.width;
    if (info.height < 0) info.height = -info.height;

    return info;
}

ImageInfo PngParser::parse(const QString& filePath) {
    ImageInfo info;
    info.fileName = QFileInfo(filePath).fileName();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        info.status = "Ошибка доступа";
        return info;
    }

    if (file.size() < 20) {
        info.status = "Файл поврежден (слишком короткий PNG)";
        return info;
    }

    static const unsigned char pngSig[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
    unsigned char magic[8];
    if (file.read(reinterpret_cast<char*>(magic), 8) != 8) {
        info.status = "Файл поврежден (неверная сигнатура PNG)";
        return info;
    }
    for (int i = 0; i < 8; ++i) {
        if (magic[i] != pngSig[i]) {
            info.status = "Файл поврежден (неверная сигнатура PNG)";
            return info;
        }
    }

    file.seek(file.size() - 8);
    char iend[4];
    if (file.read(iend, 4) != 4 || qstrncmp(iend, "IEND", 4) != 0) {
        info.status = "Файл поврежден (отсутствует IEND)";
        return info;
    }

    file.seek(8);
    qint32 chunkLen = readInt32BE(file);
    char chunkType[5] = {0};
    if (file.read(chunkType, 4) != 4) {
        info.status = "Файл поврежден (некорректный IHDR)";
        return info;
    }

    if (qstrcmp(chunkType, "IHDR") == 0 && chunkLen >= 13) {
        info.width = readInt32BE(file);
        info.height = readInt32BE(file);
        unsigned char depth = 0;
        if (file.read(reinterpret_cast<char*>(&depth), 1) == 1) {
            info.depth = depth;
        }
        info.compression = "Deflate";
    } else {
        info.status = "Файл поврежден (отсутствует IHDR)";
    }

    return info;
}

ImageInfo JpgParser::parse(const QString& filePath) {
    ImageInfo info;
    info.fileName = QFileInfo(filePath).fileName();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        info.status = "Ошибка доступа";
        return info;
    }

    if (file.size() < 4) {
        info.status = "Файл поврежден (слишком короткий JPG)";
        return info;
    }

    unsigned char magic[2];
    if (file.read(reinterpret_cast<char*>(magic), 2) != 2 || magic[0] != 0xFF || magic[1] != 0xD8) {
        info.status = "Файл поврежден (неверная сигнатура JPG)";
        return info;
    }

    file.seek(file.size() - 2);
    if (file.read(reinterpret_cast<char*>(magic), 2) != 2 || magic[0] != 0xFF || magic[1] != 0xD9) {
        info.status = "Файл поврежден (отсутствует EOI)";
        return info;
    }

    file.seek(2);
    info.compression = "JPEG (Lossy)";

    int guard = 0;
    while (!file.atEnd() && guard++ < 10000) {
        if (file.read(reinterpret_cast<char*>(magic), 2) != 2) break;
        if (magic[0] != 0xFF) break;

        if (magic[1] >= 0xC0 && magic[1] <= 0xC3) {
            readInt16BE(file);
            unsigned char d = 0;
            if (file.read(reinterpret_cast<char*>(&d), 1) != 1) break;
            info.depth = d * 8;
            info.height = readInt16BE(file);
            info.width = readInt16BE(file);
            break;
        } else {
            qint16 len = readInt16BE(file);
            if (len < 2) break;
            if (!file.seek(file.pos() + len - 2)) break;
        }
    }

    return info;
}

ImageInfo GifParser::parse(const QString& filePath) {
    ImageInfo info;
    info.fileName = QFileInfo(filePath).fileName();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        info.status = "Ошибка доступа";
        return info;
    }

    if (file.size() < 13) {
        info.status = "Файл поврежден (слишком короткий GIF)";
        return info;
    }

    char magic[6];
    if (file.read(magic, 6) != 6 || qstrncmp(magic, "GIF", 3) != 0) {
        info.status = "Файл поврежден (неверная сигнатура GIF)";
        return info;
    }

    info.width = readInt16LE(file);
    info.height = readInt16LE(file);

    char packed = 0;
    if (file.read(&packed, 1) != 1) {
        info.status = "Файл поврежден (некорректный заголовок GIF)";
        return info;
    }
    info.depth = ((packed & 0x70) >> 4) + 1;
    info.compression = "LZW";

    return info;
}

ImageInfo TiffParser::parse(const QString& filePath) {
    ImageInfo info;
    info.fileName = QFileInfo(filePath).fileName();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        info.status = "Ошибка доступа";
        return info;
    }

    if (file.size() < 8) {
        info.status = "Файл поврежден (слишком короткий TIFF)";
        return info;
    }

    unsigned char sig[4];
    if (file.read(reinterpret_cast<char*>(sig), 4) != 4) {
        info.status = "Файл поврежден (не читается сигнатура TIFF)";
        return info;
    }

    bool littleEndian = false;
    if (sig[0] == 'I' && sig[1] == 'I' && sig[2] == 0x2A && sig[3] == 0x00) {
        littleEndian = true;
    } else if (sig[0] == 'M' && sig[1] == 'M' && sig[2] == 0x00 && sig[3] == 0x2A) {
        littleEndian = false;
    } else {
        info.status = "Файл поврежден (неверная сигнатура TIFF)";
        return info;
    }

    auto readU32 = [&](QFile& f) -> quint32 {
        unsigned char b[4];
        if (f.read(reinterpret_cast<char*>(b), 4) != 4) return 0;
        if (littleEndian)
            return quint32(b[0]) | (quint32(b[1]) << 8) | (quint32(b[2]) << 16) | (quint32(b[3]) << 24);
        else
            return (quint32(b[0]) << 24) | (quint32(b[1]) << 16) | (quint32(b[2]) << 8) | quint32(b[3]);
    };
    auto readU16 = [&](QFile& f) -> quint16 {
        unsigned char b[2];
        if (f.read(reinterpret_cast<char*>(b), 2) != 2) return 0;
        if (littleEndian)
            return quint16(b[0]) | (quint16(b[1]) << 8);
        else
            return (quint16(b[0]) << 8) | quint16(b[1]);
    };

    quint32 ifdOffset = readU32(file);
    if (ifdOffset == 0 || ifdOffset >= quint32(file.size())) {
        info.status = "Файл поврежден (некорректный IFD offset)";
        return info;
    }

    if (!file.seek(ifdOffset)) {
        info.status = "Файл поврежден (не удаётся перейти к IFD)";
        return info;
    }

    quint16 numEntries = readU16(file);
    if (numEntries == 0 || numEntries > 1000) {
        info.status = "Файл поврежден (некорректное число тегов)";
        return info;
    }

    int compressionCode = 1;

    for (int i = 0; i < numEntries; ++i) {
        quint16 tag = readU16(file);
        quint16 type = readU16(file);
        quint32 count = readU32(file);
        quint32 value = readU32(file);

        Q_UNUSED(type);
        Q_UNUSED(count);

        if (tag == 256) info.width = int(value);
        else if (tag == 257) info.height = int(value);
        else if (tag == 258) info.depth = int(value);
        else if (tag == 259) compressionCode = int(value);
        else if (tag == 282 || tag == 283) {
            if (value != 0 && value < quint32(file.size()) - 8) {
                qint64 savedPos = file.pos();
                if (file.seek(value)) {
                    quint32 num = readU32(file);
                    quint32 den = readU32(file);
                    if (den != 0 && num != 0) {
                        int dpiVal = int(num / den);
                        if (dpiVal > 0 && dpiVal < 100000) info.dpi = dpiVal;
                    }
                    file.seek(savedPos);
                }
            }
        }
    }

    switch (compressionCode) {
    case 1: info.compression = "None"; break;
    case 2: info.compression = "CCITT RLE"; break;
    case 3: info.compression = "CCITT G3"; break;
    case 4: info.compression = "CCITT G4"; break;
    case 5: info.compression = "LZW"; break;
    case 6: info.compression = "JPEG (old)"; break;
    case 7: info.compression = "JPEG"; break;
    case 8: info.compression = "Deflate"; break;
    case 32773: info.compression = "PackBits"; break;
    default: info.compression = "Неизвестно"; break;
    }

    return info;
}

ImageInfo PcxParser::parse(const QString& filePath) {
    ImageInfo info;
    info.fileName = QFileInfo(filePath).fileName();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        info.status = "Ошибка доступа";
        return info;
    }

    if (file.size() < 128) {
        info.status = "Файл поврежден (слишком короткий PCX)";
        return info;
    }

    unsigned char header[128];
    if (file.read(reinterpret_cast<char*>(header), 128) != 128) {
        info.status = "Файл поврежден (не читается заголовок PCX)";
        return info;
    }

    if (header[0] != 0x0A) {
        info.status = "Файл поврежден (неверная сигнатура PCX)";
        return info;
    }

    unsigned char version = header[1];
    unsigned char encoding = header[2];
    unsigned char bitsPerPixel = header[3];

    int xmin = header[4] | (header[5] << 8);
    int ymin = header[6] | (header[7] << 8);
    int xmax = header[8] | (header[9] << 8);
    int ymax = header[10] | (header[11] << 8);

    int width = xmax - xmin + 1;
    int height = ymax - ymin + 1;

    if (width <= 0 || height <= 0) {
        info.status = "Файл поврежден (некорректные размеры PCX)";
        return info;
    }

    int planes = header[65];
    if (planes <= 0) planes = 1;

    info.width = width;
    info.height = height;
    info.depth = bitsPerPixel * planes;

    if (encoding == 1)
        info.compression = "RLE";
    else
        info.compression = "None";

    switch (version) {
    case 0: info.compression += " (v2.5)"; break;
    case 2: info.compression += " (v2.8)"; break;
    case 3: info.compression += " (v2.8 palette)"; break;
    case 4: info.compression += " (v3.0)"; break;
    case 5: info.compression += " (v3.0)"; break;
    default: break;
    }

    return info;
}

ImageParser* ParserFactory::getParser(const QString& extension) {
    QString ext = extension.toLower();
    if (ext == "bmp") return new BmpParser();
    if (ext == "png") return new PngParser();
    if (ext == "jpg" || ext == "jpeg") return new JpgParser();
    if (ext == "gif") return new GifParser();
    if (ext == "tif" || ext == "tiff") return new TiffParser();
    if (ext == "pcx") return new PcxParser();
    return nullptr;
}
