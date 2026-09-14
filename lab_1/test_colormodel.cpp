#include <QTest>
#include <cstdlib>
#include "ColorModel.h"

class TestColorModel : public QObject {
    Q_OBJECT

private slots:
    void testRgbToCmykGcr();
    void testRgbToCmykUcr();
    void testRgbToHsv();
    void testClippingStrategy();
    void testScalingStrategy();
};

void TestColorModel::testRgbToCmykGcr() {
    ColorModel model;
    model.setCmykMode(static_cast<ColorModel::CmykMode>(0));
    model.setRgb(255.0, 0.0, 0.0);
    CMYKColor cmyk = model.getCmyk();

    QCOMPARE(cmyk.c, 0.0);
    QCOMPARE(cmyk.m, 100.0);
    QCOMPARE(cmyk.y, 100.0);
    QCOMPARE(cmyk.k, 0.0);
}

void TestColorModel::testRgbToCmykUcr() {
    ColorModel model;
    model.setCmykMode(static_cast<ColorModel::CmykMode>(1));

    model.setRgb(200.0, 200.0, 200.0);
    CMYKColor cmyk = model.getCmyk();

    QVERIFY(cmyk.k == 0.0 || cmyk.k < 40.0);
}

void TestColorModel::testRgbToHsv() {
    ColorModel model;

    model.setRgb(0.0, 255.0, 0.0);
    HSVColor hsv = model.getHsv();

    QCOMPARE(hsv.h, 120.0);
    QCOMPARE(hsv.s, 100.0);
    QCOMPARE(hsv.v, 100.0);
}

void TestColorModel::testClippingStrategy() {
    ColorModel model;
    model.setStrategy(static_cast<ColorModel::OutOfBoundsStrategy>(0));

    model.setCmyk(0.0, 0.0, 0.0, 0.0);
    RGBColor rgb = model.getRgb();

    QVERIFY(rgb.r >= 0.0 && rgb.r <= 255.0);
    QVERIFY(rgb.g >= 0.0 && rgb.g <= 255.0);
    QVERIFY(rgb.b >= 0.0 && rgb.b <= 255.0);
}

void TestColorModel::testScalingStrategy() {
    ColorModel model;
    model.setStrategy(static_cast<ColorModel::OutOfBoundsStrategy>(1));

    model.setCmyk(10.0, 20.0, 30.0, 5.0);
    RGBColor rgb = model.getRgb();

    QVERIFY(rgb.r >= 0.0 && rgb.r <= 255.0);
    QVERIFY(rgb.g >= 0.0 && rgb.g <= 255.0);
    QVERIFY(rgb.b >= 0.0 && rgb.b <= 255.0);
}

int main(int argc, char *argv[]) {
    TestColorModel testObject;
    int result = QTest::qExec(&testObject, argc, argv);

    system("pause");
    return result;
}

#include "test_colormodel.moc"
