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

    QVERIFY(info.status.contains("отсутствует EOI") || info.status.contains("неверная сигнатура"));
}

void TestParsers::testParserFactory() {
    ImageParser* parserBmp = ParserFactory::getParser("bmp");
    QVERIFY(parserBmp != nullptr);
    delete parserBmp;

    ImageParser* parserUnknown = ParserFactory::getParser("unknown_ext");
    QVERIFY(parserUnknown == nullptr);
}

QTEST_MAIN(TestParsers)
#include "tst_parsers.moc"
