#pragma once
#include <algorithm>
#include <cmath>

struct RGBColor { double r = 0, g = 0, b = 0; };
struct CMYKColor { double c = 0, m = 0, y = 0, k = 100; };
struct HSVColor { double h = 0, s = 0, v = 0; };

class ColorModel {
public:
    enum class CmykMode { GCR, UCR };
    enum class OutOfBoundsStrategy { CLIPPING, SCALING };

    void setRgb(double r, double g, double b);
    void setCmyk(double c, double m, double y, double k);
    void setHsv(double h, double s, double v);

    RGBColor getRgb() const { return m_rgb; }
    CMYKColor getCmyk() const { return m_cmyk; }
    HSVColor getHsv() const { return m_hsv; }

    void setCmykMode(CmykMode mode) { m_cmykMode = mode; updateFromRgb(); }
    void setStrategy(OutOfBoundsStrategy strat) { m_strategy = strat; }

    bool wasOutOfBounds() const { return m_outOfBounds; }

private:
    RGBColor m_rgb;
    CMYKColor m_cmyk;
    HSVColor m_hsv;

    CmykMode m_cmykMode = CmykMode::GCR;
    OutOfBoundsStrategy m_strategy = OutOfBoundsStrategy::CLIPPING;
    bool m_outOfBounds = false;

    void updateFromRgb();
    void updateFromCmyk();
    void updateFromHsv();
    void applyRgbBounds(double& r, double& g, double& b);
};
