#include "ColorController.h"
#include "MainWindow.h"

ColorController::ColorController(ColorModel* model, MainWindow* view, QObject *parent)
    : QObject(parent), m_model(model), m_view(view) {}

void ColorController::onRgbChanged(double r, double g, double b) {
    if (m_updating) return;
    m_model->setRgb(r, g, b);
    updateView();
}

void ColorController::onCmykChanged(double c, double m, double y, double k) {
    if (m_updating) return;
    m_model->setCmyk(c, m, y, k);
    updateView();
}

void ColorController::onHsvChanged(double h, double s, double v) {
    if (m_updating) return;
    m_model->setHsv(h, s, v);
    updateView();
}

void ColorController::setCmykMode(int index) {
    m_model->setCmykMode(index == 0 ? ColorModel::CmykMode::GCR : ColorModel::CmykMode::UCR);
    updateView();
}

void ColorController::setStrategy(int index) {
    m_model->setStrategy(index == 0 ? ColorModel::OutOfBoundsStrategy::CLIPPING : ColorModel::OutOfBoundsStrategy::SCALING);
    updateView();
}

void ColorController::updateView() {
    m_updating = true;
    m_view->updateUi(m_model->getRgb(), m_model->getCmyk(), m_model->getHsv(), m_model->wasOutOfBounds());
    m_updating = false;
}
