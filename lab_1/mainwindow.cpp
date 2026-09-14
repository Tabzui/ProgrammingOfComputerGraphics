#include "mainwindow.h"
#include "ColorController.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QColorDialog>
#include <cmath>
#include <algorithm>

static RGBColor cmykToRgbHelper(double c, double m, double y, double k) {
    c /= 100.0; m /= 100.0; y /= 100.0; k /= 100.0;
    double r = std::clamp(255.0 * (1.0 - c) * (1.0 - k), 0.0, 255.0);
    double g = std::clamp(255.0 * (1.0 - m) * (1.0 - k), 0.0, 255.0);
    double b = std::clamp(255.0 * (1.0 - y) * (1.0 - k), 0.0, 255.0);
    return { r, g, b };
}

static RGBColor hsvToRgbHelper(double h, double s, double v) {
    s /= 100.0; v /= 100.0;
    double r = 0, g = 0, b = 0;
    if (s == 0) {
        r = g = b = v;
    } else {
        if (h >= 360.0) h = 0.0;
        h /= 60.0;
        int i = static_cast<int>(std::floor(h));
        double f = h - i;
        double p = v * (1.0 - s);
        double q = v * (1.0 - s * f);
        double t = v * (1.0 - s * (1.0 - f));
        switch (i) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
        }
    }
    return { r * 255.0, g * 255.0, b * 255.0 };
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setupUi();
    resize(950, 600);
    setWindowTitle("Лабораторная работа 1 - Вариант 7 (CMYK - RGB - HSV)");
}

void MainWindow::setController(ColorController* controller) {
    m_controller = controller;
    connectSignals();
}

void MainWindow::setupUi() {
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout *topLayout = new QHBoxLayout();
    comboCmykMode = new QComboBox();
    comboCmykMode->addItems({"GCR (Замена серого)", "UCR (Удаление подложки)"});

    comboStrategy = new QComboBox();
    comboStrategy->addItems({"Clipping (Ообрезание)", "Scaling (Масштабирование)"});

    warningLabel = new QLabel("⚠️ Выход за границы! Выполнена коррекция.");
    warningLabel->setStyleSheet("color: red; font-weight: bold;");
    warningLabel->setVisible(false);

    topLayout->addWidget(new QLabel("Цветоделение CMYK:"));
    topLayout->addWidget(comboCmykMode);
    topLayout->addWidget(new QLabel("Выход за рамки:"));
    topLayout->addWidget(comboStrategy);
    topLayout->addWidget(warningLabel);
    topLayout->addStretch();

    mainLayout->addLayout(topLayout);

    QHBoxLayout *modelsLayout = new QHBoxLayout();

    auto createRow = [](const QString& name, double min, double max, QDoubleSpinBox*& spin, QSlider*& slider) {
        QVBoxLayout* cellLayout = new QVBoxLayout();
        QHBoxLayout* controlLayout = new QHBoxLayout();

        controlLayout->addWidget(new QLabel(name));
        spin = new QDoubleSpinBox();
        spin->setKeyboardTracking(false);
        spin->setRange(min, max);
        spin->setDecimals(1);
        controlLayout->addWidget(spin);

        slider = new QSlider(Qt::Horizontal);
        slider->setRange(static_cast<int>(min), static_cast<int>(max));

        cellLayout->addLayout(controlLayout);
        cellLayout->addWidget(slider);
        return cellLayout;
    };

    QGroupBox *rgbGroup = new QGroupBox("RGB");
    QVBoxLayout *rgbLayout = new QVBoxLayout(rgbGroup);
    rgbLayout->addLayout(createRow("R:", 0, 255, spinR, slideR));
    rgbLayout->addLayout(createRow("G:", 0, 255, spinG, slideG));
    rgbLayout->addLayout(createRow("B:", 0, 255, spinB, slideB));
    modelsLayout->addWidget(rgbGroup);

    QGroupBox *cmykGroup = new QGroupBox("CMYK");
    QVBoxLayout *cmykLayout = new QVBoxLayout(cmykGroup);
    cmykLayout->addLayout(createRow("C:", 0, 100, spinC, slideC));
    cmykLayout->addLayout(createRow("M:", 0, 100, spinM, slideM));
    cmykLayout->addLayout(createRow("Y:", 0, 100, spinY, slideY));
    cmykLayout->addLayout(createRow("K:", 0, 100, spinK, slideK));
    modelsLayout->addWidget(cmykGroup);

    QGroupBox *hsvGroup = new QGroupBox("HSV");
    QVBoxLayout *hsvLayout = new QVBoxLayout(hsvGroup);
    hsvLayout->addLayout(createRow("H:", 0, 360, spinH, slideH));
    hsvLayout->addLayout(createRow("S:", 0, 100, spinS, slideS));
    hsvLayout->addLayout(createRow("V:", 0, 100, spinV, slideV));
    modelsLayout->addWidget(hsvGroup);

    mainLayout->addLayout(modelsLayout);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    colorPreview = new QFrame();
    colorPreview->setMinimumSize(150, 60);
    colorPreview->setFrameShape(QFrame::Box);

    btnPalette = new QPushButton("Выбрать цвет из палитры");

    bottomLayout->addWidget(colorPreview);
    bottomLayout->addWidget(btnPalette);
    mainLayout->addLayout(bottomLayout);

    setCentralWidget(centralWidget);
}

void MainWindow::connectSignals() {
    connect(comboCmykMode, &QComboBox::currentIndexChanged, this, [this](int idx){ if(m_controller) m_controller->setCmykMode(idx); });
    connect(comboStrategy, &QComboBox::currentIndexChanged, this, [this](int idx){ if(m_controller) m_controller->setStrategy(idx); });

    auto onRgb = [this]() {
        if (m_blockSignals || !m_controller) return;
        m_controller->onRgbChanged(spinR->value(), spinG->value(), spinB->value());
    };
    connect(spinR, &QDoubleSpinBox::valueChanged, this, onRgb);
    connect(spinG, &QDoubleSpinBox::valueChanged, this, onRgb);
    connect(spinB, &QDoubleSpinBox::valueChanged, this, onRgb);
    connect(slideR, &QSlider::valueChanged, this, [this](int v){ spinR->setValue(v); });
    connect(slideG, &QSlider::valueChanged, this, [this](int v){ spinG->setValue(v); });
    connect(slideB, &QSlider::valueChanged, this, [this](int v){ spinB->setValue(v); });

    auto onCmyk = [this]() {
        if (m_blockSignals || !m_controller) return;
        m_controller->onCmykChanged(spinC->value(), spinM->value(), spinY->value(), spinK->value());
    };
    connect(spinC, &QDoubleSpinBox::valueChanged, this, onCmyk);
    connect(spinM, &QDoubleSpinBox::valueChanged, this, onCmyk);
    connect(spinY, &QDoubleSpinBox::valueChanged, this, onCmyk);
    connect(spinK, &QDoubleSpinBox::valueChanged, this, onCmyk);
    connect(slideC, &QSlider::valueChanged, this, [this](int v){ spinC->setValue(v); });
    connect(slideM, &QSlider::valueChanged, this, [this](int v){ spinM->setValue(v); });
    connect(slideY, &QSlider::valueChanged, this, [this](int v){ spinY->setValue(v); });
    connect(slideK, &QSlider::valueChanged, this, [this](int v){ spinK->setValue(v); });

    auto onHsv = [this]() {
        if (m_blockSignals || !m_controller) return;
        m_controller->onHsvChanged(spinH->value(), spinS->value(), spinV->value());
    };
    connect(spinH, &QDoubleSpinBox::valueChanged, this, onHsv);
    connect(spinS, &QDoubleSpinBox::valueChanged, this, onHsv);
    connect(spinV, &QDoubleSpinBox::valueChanged, this, onHsv);
    connect(slideH, &QSlider::valueChanged, this, [this](int v){ spinH->setValue(v); });
    connect(slideS, &QSlider::valueChanged, this, [this](int v){ spinS->setValue(v); });
    connect(slideV, &QSlider::valueChanged, this, [this](int v){ spinV->setValue(v); });

    connect(btnPalette, &QPushButton::clicked, this, [this]() {
        QColor col = QColorDialog::getColor(QColor(spinR->value(), spinG->value(), spinB->value()), this);
        if (col.isValid() && m_controller) {
            m_controller->onRgbChanged(col.red(), col.green(), col.blue());
        }
    });
}

void MainWindow::updateUi(const RGBColor& rgb, const CMYKColor& cmyk, const HSVColor& hsv, bool outOfBounds) {
    m_blockSignals = true;

    spinR->setValue(rgb.r); slideR->setValue(static_cast<int>(rgb.r));
    spinG->setValue(rgb.g); slideG->setValue(static_cast<int>(rgb.g));
    spinB->setValue(rgb.b); slideB->setValue(static_cast<int>(rgb.b));

    spinC->setValue(cmyk.c); slideC->setValue(static_cast<int>(cmyk.c));
    spinM->setValue(cmyk.m); slideM->setValue(static_cast<int>(cmyk.m));
    spinY->setValue(cmyk.y); slideY->setValue(static_cast<int>(cmyk.y));
    spinK->setValue(cmyk.k); slideK->setValue(static_cast<int>(cmyk.k));

    spinH->setValue(hsv.h); slideH->setValue(static_cast<int>(hsv.h));
    spinS->setValue(hsv.s); slideS->setValue(static_cast<int>(hsv.s));
    spinV->setValue(hsv.v); slideV->setValue(static_cast<int>(hsv.v));

    colorPreview->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(rgb.r).arg(rgb.g).arg(rgb.b));
    warningLabel->setVisible(outOfBounds);

    updateSliderStyles(rgb, cmyk, hsv);

    m_blockSignals = false;
}

void MainWindow::updateSliderStyles(const RGBColor& rgb, const CMYKColor& cmyk, const HSVColor& hsv) {
    QString handleStyle =
        "QSlider::handle:horizontal {"
        "  background: #ffffff;"
        "  border: 1px solid #555555;"
        "  width: 14px;"
        "  margin-top: -4px;"
        "  margin-bottom: -4px;"
        "  border-radius: 7px;"
        "}";

    auto makeStyle = [&handleStyle](const QString& stops) {
        return QString(
                   "QSlider::groove:horizontal {"
                   "  height: 8px;"
                   "  border-radius: 4px;"
                   "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0, %1);"
                   "}"
                   ).arg(stops) + handleStyle;
    };


    slideR->setStyleSheet(makeStyle(QString("stop:0 rgb(0, %1, %2), stop:1 rgb(255, %1, %2)").arg(rgb.g).arg(rgb.b)));
    slideG->setStyleSheet(makeStyle(QString("stop:0 rgb(%1, 0, %2), stop:1 rgb(%1, 255, %2)").arg(rgb.r).arg(rgb.b)));
    slideB->setStyleSheet(makeStyle(QString("stop:0 rgb(%1, %2, 0), stop:1 rgb(%1, %2, 255)").arg(rgb.r).arg(rgb.g)));


    RGBColor c0 = cmykToRgbHelper(0, cmyk.m, cmyk.y, cmyk.k), c1 = cmykToRgbHelper(100, cmyk.m, cmyk.y, cmyk.k);
    slideC->setStyleSheet(makeStyle(QString("stop:0 rgb(%1,%2,%3), stop:1 rgb(%4,%5,%6)").arg(c0.r).arg(c0.g).arg(c0.b).arg(c1.r).arg(c1.g).arg(c1.b)));

    RGBColor m0 = cmykToRgbHelper(cmyk.c, 0, cmyk.y, cmyk.k), m1 = cmykToRgbHelper(cmyk.c, 100, cmyk.y, cmyk.k);
    slideM->setStyleSheet(makeStyle(QString("stop:0 rgb(%1,%2,%3), stop:1 rgb(%4,%5,%6)").arg(m0.r).arg(m0.g).arg(m0.b).arg(m1.r).arg(m1.g).arg(m1.b)));

    RGBColor y0 = cmykToRgbHelper(cmyk.c, cmyk.m, 0, cmyk.k), y1 = cmykToRgbHelper(cmyk.c, cmyk.m, 100, cmyk.k);
    slideY->setStyleSheet(makeStyle(QString("stop:0 rgb(%1,%2,%3), stop:1 rgb(%4,%5,%6)").arg(y0.r).arg(y0.g).arg(y0.b).arg(y1.r).arg(y1.g).arg(y1.b)));

    RGBColor k0 = cmykToRgbHelper(cmyk.c, cmyk.m, cmyk.y, 0), k1 = cmykToRgbHelper(cmyk.c, cmyk.m, cmyk.y, 100);
    slideK->setStyleSheet(makeStyle(QString("stop:0 rgb(%1,%2,%3), stop:1 rgb(%4,%5,%6)").arg(k0.r).arg(k0.g).arg(k0.b).arg(k1.r).arg(k1.g).arg(k1.b)));


    QString hStops;
    for (int i = 0; i <= 6; ++i) {
        double hue = i * 60.0;
        RGBColor c = hsvToRgbHelper(hue, hsv.s, hsv.v);
        hStops += QString("stop:%1 rgb(%2,%3,%4)%5")
                      .arg(i / 6.0)
                      .arg(static_cast<int>(c.r))
                      .arg(static_cast<int>(c.g))
                      .arg(static_cast<int>(c.b))
                      .arg(i < 6 ? ", " : "");
    }
    slideH->setStyleSheet(makeStyle(hStops));

    RGBColor s0 = hsvToRgbHelper(hsv.h, 0, hsv.v), s1 = hsvToRgbHelper(hsv.h, 100, hsv.v);
    slideS->setStyleSheet(makeStyle(QString("stop:0 rgb(%1,%2,%3), stop:1 rgb(%4,%5,%6)").arg(s0.r).arg(s0.g).arg(s0.b).arg(s1.r).arg(s1.g).arg(s1.b)));

    RGBColor v0 = hsvToRgbHelper(hsv.h, hsv.s, 0), v1 = hsvToRgbHelper(hsv.h, hsv.s, 100);
    slideV->setStyleSheet(makeStyle(QString("stop:0 rgb(%1,%2,%3), stop:1 rgb(%4,%5,%6)").arg(v0.r).arg(v0.g).arg(v0.b).arg(v1.r).arg(v1.g).arg(v1.b)));
}
