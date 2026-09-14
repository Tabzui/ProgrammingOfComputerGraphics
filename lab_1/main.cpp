#include <QApplication>
#include "ColorModel.h"
#include "MainWindow.h"
#include "ColorController.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    ColorModel model;
    MainWindow view;
    ColorController controller(&model, &view);

    view.setController(&controller);
    model.setRgb(255, 0, 0);
    controller.onRgbChanged(255, 0, 0);

    view.show();
    return a.exec();
}
