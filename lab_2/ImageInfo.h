#pragma once
#include <QString>

struct ImageInfo {
    QString fileName;
    int width = 0;
    int height = 0;
    int dpi = 0;
    int depth = 0;
    QString compression = "N/A";
    QString status = "ОК";
};
