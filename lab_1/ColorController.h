#pragma once
#include <QObject>
#include <algorithm>
#include "ColorModel.h"

class MainWindow;

class ColorController : public QObject {
    Q_OBJECT
public:
    explicit ColorController(ColorModel* model, MainWindow* view, QObject *parent = nullptr);

    void onRgbChanged(double r, double g, double b);
    void onCmykChanged(double c, double m, double y, double k);
    void onHsvChanged(double h, double s, double v);

    void setCmykMode(int index);
    void setStrategy(int index);

private:
    ColorModel* m_model;
    MainWindow* m_view;
    bool m_updating = false;

    void updateView();
};
