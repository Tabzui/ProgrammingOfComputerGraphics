#include <QtTest>
#include <QFile>
#include <QTemporaryFile>
#include <cstring>
#include "Parsers.h"

class TestParsers : public QObject {
    Q_OBJECT

private slots:
    void testBmpHeaderParsing();
    void testCorruptedPngSignature();
    void testCorruptedJpgMissingEoi();
    void testParserFactory();
    void testTiffHeaderParsing();
    void testCorruptedTiffSignature();
    void testPcxHeaderParsing();
    void testCorruptedPcxSignature();
};

void TestParsers::testBmpHeaderParsing() {
    QTemporaryFile file;
    QVERIFY(file.open());

    QByteArray bmpData(54, 0);
    bmpData[0] = 'B';
    bmpData[1] = 'M';

    qint32 fileSize = 54;
    std::memcpy(bmpData.data() + 2, &fileSize, 4);

    qint32 width = 640;
    std::memcpy(bmpData.data() + 18, &width, 4);

    qint32 height = 480;
    std::memcpy(bmpData.data() + 22, &height, 4);

    qint16 depth = 24;
    std::memcpy(bmpData.data() + 28, &depth, 2);

    file.write(bmpData);
    file.flush();
    file.close();

    BmpParser parser;
    ImageInfo info = parser.parse(file.fileName());

    QCOMPARE(info.width, 640);
    QCOMPARE(info.height, 480);
    QCOMPARE(info.depth, 24);
    QCOMPARE(info.status, QString("ОК"));
}

void TestParsers::testCorruptedPngSignature() {
    QTemporaryFile file;
    QVERIFY(file.open());

    file.write("NOT_A_PNG_FILE_HEADER");
    file.flush();
    file.close();

    PngParser parser;
    ImageInfo info = parser.parse(file.fileName());

    QVERIFY(info.status.contains("неверная сигнатура"));
}

void TestParsers::testCorruptedJpgMissingEoi() {
    QTemporaryFile file;
    QVERIFY(file.open());

    QByteArray jpgHeader;
    jpgHeader.append("\xFF\xD8\xFF\xE0", 4);
    file.write(jpgHeader);
    file.flush();
    file.close();

    JpgParser parser;
    ImageInfo info = parser.parse(file.fileName());

    QVERIFY(info.status.contains("отсутствует EOI")
            || info.status.contains("неверная сигнатура")
            || info.status.contains("слишком короткий"));
}

void TestParsers::testParserFactory() {
    ImageParser* parserBmp = ParserFactory::getParser("bmp");
    QVERIFY(parserBmp != nullptr);
    delete parserBmp;

    ImageParser* parserTif = ParserFactory::getParser("tif");
    QVERIFY(parserTif != nullptr);
    delete parserTif;

    ImageParser* parserTiff = ParserFactory::getParser("tiff");
    QVERIFY(parserTiff != nullptr);
    delete parserTiff;

    ImageParser* parserPcx = ParserFactory::getParser("pcx");
    QVERIFY(parserPcx != nullptr);
    delete parserPcx;

    ImageParser* parserUnknown = ParserFactory::getParser("unknown_ext");
    QVERIFY(parserUnknown == nullptr);
}

void TestParsers::testTiffHeaderParsing() {
    QTemporaryFile file;
    QVERIFY(file.open());

    QByteArray tiff;
    tiff.append("II");
    tiff.append(char(0x2A));
    tiff.append(char(0x00));

    qint32 ifdOffset = 8;
    tiff.append(reinterpret_cast<const char*>(&ifdOffset), 4);

    while (tiff.size() < 8) tiff.append(char(0));

    qint16 numEntries = 4;
    tiff.append(reinterpret_cast<const char*>(&numEntries), 2);

    auto appendEntry = [&](quint16 tag, quint16 type, quint32 count, quint32 value) {
        tiff.append(reinterpret_cast<const char*>(&tag), 2);
        tiff.append(reinterpret_cast<const char*>(&type), 2);
        tiff.append(reinterpret_cast<const char*>(&count), 4);
        tiff.append(reinterpret_cast<const char*>(&value), 4);
    };

    quint32 w = 800;
    quint32 h = 600;
    quint32 depth = 8;
    quint32 compr = 1;

    appendEntry(256, 3, 1, w);
    appendEntry(257, 3, 1, h);
    appendEntry(258, 3, 1, depth);
    appendEntry(259, 3, 1, compr);

    quint32 nextIfd = 0;
    tiff.append(reinterpret_cast<const char*>(&nextIfd), 4);

    file.write(tiff);
    file.flush();
    file.close();

    TiffParser parser;
    ImageInfo info = parser.parse(file.fileName());

    QCOMPARE(info.width, 800);
    QCOMPARE(info.height, 600);
    QCOMPARE(info.depth, 8);
    QCOMPARE(info.status, QString("ОК"));
}

void TestParsers::testCorruptedTiffSignature() {
    QTemporaryFile file;
    QVERIFY(file.open());

    file.write("NOT_A_TIFF_FILE_HEADER_AT_ALL");
    file.flush();
    file.close();

    TiffParser parser;
    ImageInfo info = parser.parse(file.fileName());

    QVERIFY(info.status.contains("неверная сигнатура"));
}

void TestParsers::testPcxHeaderParsing() {
    QTemporaryFile file;
    QVERIFY(file.open());

    QByteArray pcx(128, 0);
    pcx[0] = 0x0A;
    pcx[1] = 5;
    pcx[2] = 1;
    pcx[3] = 8;

    pcx[4] = 0; pcx[5] = 0;
    pcx[6] = 0; pcx[7] = 0;
    pcx[8] = char(0xFF); pcx[9] = 0x02;
    pcx[10] = char(0x7F); pcx[11] = 0x01;

    pcx[65] = 3;

    file.write(pcx);
    file.flush();
    file.close();

    PcxParser parser;
    ImageInfo info = parser.parse(file.fileName());

    QCOMPARE(info.width, 0x02FF - 0 + 1);
    QCOMPARE(info.height, 0x017F - 0 + 1);
    QCOMPARE(info.depth, 8 * 3);
    QCOMPARE(info.status, QString("ОК"));
}

void TestParsers::testCorruptedPcxSignature() {
    QTemporaryFile file;
    QVERIFY(file.open());

    QByteArray pcx(128, 0);
    pcx[0] = 0x55;

    file.write(pcx);
    file.flush();
    file.close();

    PcxParser parser;
    ImageInfo info = parser.parse(file.fileName());

    QVERIFY(info.status.contains("неверная сигнатура"));
}

QTEST_MAIN(TestParsers)
#include "tst_parsers.moc"
