#pragma once
#include <QMainWindow>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include "ColorModel.h"

class ColorController;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setController(ColorController* controller);

    void updateUi(const RGBColor& rgb, const CMYKColor& cmyk, const HSVColor& hsv, bool outOfBounds);

private:
    ColorController* m_controller = nullptr;
    bool m_blockSignals = false;

    QDoubleSpinBox *spinR, *spinG, *spinB;
    QSlider *slideR, *slideG, *slideB;

    QDoubleSpinBox *spinC, *spinM, *spinY, *spinK;
    QSlider *slideC, *slideM, *slideY, *slideK;

    QDoubleSpinBox *spinH, *spinS, *spinV;
    QSlider *slideH, *slideS, *slideV;

    QFrame *colorPreview;
    QLabel *warningLabel;
    QPushButton *btnPalette;
    QComboBox *comboCmykMode;
    QComboBox *comboStrategy;

    void setupUi();
    void updateSliderStyles(const RGBColor& rgb, const CMYKColor& cmyk, const HSVColor& hsv);
    void connectSignals();
};
