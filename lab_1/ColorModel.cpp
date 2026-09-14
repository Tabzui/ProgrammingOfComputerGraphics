#include "ColorModel.h"

void ColorModel::setRgb(double r, double g, double b) {
    applyRgbBounds(r, g, b);
    m_rgb = {r, g, b};
    updateFromRgb();
}

void ColorModel::setCmyk(double c, double m, double y, double k) {
    m_cmyk = {c, m, y, k};
    updateFromCmyk();
}

void ColorModel::setHsv(double h, double s, double v) {
    m_hsv = {h, s, v};
    updateFromHsv();
}

void ColorModel::updateFromRgb() {

    double rN = m_rgb.r / 255.0;
    double gN = m_rgb.g / 255.0;
    double bN = m_rgb.b / 255.0;

    double k = std::min({1.0 - rN, 1.0 - gN, 1.0 - bN});

    // UCR / GCR
    if (m_cmykMode == CmykMode::UCR && k < 0.4) {
        k = 0.0;
    }

    if (k >= 1.0) {
        m_cmyk = {0, 0, 0, 100.0};
    } else {
        m_cmyk.c = (1.0 - rN - k) / (1.0 - k) * 100.0;
        m_cmyk.m = (1.0 - gN - k) / (1.0 - k) * 100.0;
        m_cmyk.y = (1.0 - bN - k) / (1.0 - k) * 100.0;
        m_cmyk.k = k * 100.0;
    }

    double cmax = std::max({rN, gN, bN});
    double cmin = std::min({rN, gN, bN});
    double delta = cmax - cmin;

    m_hsv.v = cmax * 100.0;
    m_hsv.s = (cmax == 0) ? 0 : (delta / cmax) * 100.0;

    if (delta == 0) m_hsv.h = 0;
    else if (cmax == rN) m_hsv.h = 60.0 * std::fmod(((gN - bN) / delta), 6.0);
    else if (cmax == gN) m_hsv.h = 60.0 * (((bN - rN) / delta) + 2.0);
    else m_hsv.h = 60.0 * (((rN - gN) / delta) + 4.0);

    if (m_hsv.h < 0) m_hsv.h += 360.0;
}

void ColorModel::updateFromCmyk() {
    double c = m_cmyk.c / 100.0;
    double m = m_cmyk.m / 100.0;
    double y = m_cmyk.y / 100.0;
    double k = m_cmyk.k / 100.0;

    double r = 255.0 * (1.0 - c) * (1.0 - k);
    double g = 255.0 * (1.0 - m) * (1.0 - k);
    double b = 255.0 * (1.0 - y) * (1.0 - k);

    applyRgbBounds(r, g, b);
    m_rgb = {r, g, b};
    updateFromRgb();
}

void ColorModel::updateFromHsv() {
    double h = m_hsv.h;
    double s = m_hsv.s / 100.0;
    double v = m_hsv.v / 100.0;
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

    r *= 255.0; g *= 255.0; b *= 255.0;
    applyRgbBounds(r, g, b);
    m_rgb = {r, g, b};
    updateFromRgb();
}

void ColorModel::applyRgbBounds(double& r, double& g, double& b) {
    m_outOfBounds = false;
    if (r > 255.0 || g > 255.0 || b > 255.0 || r < 0.0 || g < 0.0 || b < 0.0) {
        m_outOfBounds = true;
        if (m_strategy == OutOfBoundsStrategy::CLIPPING) {
            r = std::clamp(r, 0.0, 255.0);
            g = std::clamp(g, 0.0, 255.0);
            b = std::clamp(b, 0.0, 255.0);
        } else {
            double maxVal = std::max({r, g, b});
            if (maxVal > 255.0) {
                double scale = 255.0 / maxVal;
                r *= scale; g *= scale; b *= scale;
            }
            r = std::max(0.0, r); g = std::max(0.0, g); b = std::max(0.0, b);
        }
    }
}
