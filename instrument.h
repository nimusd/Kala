#ifndef INSTRUMENT_H
#define INSTRUMENT_H

// Minimal instrument.h stub — provides the LookupTable class needed by
// Faust-generated piano calibration data (piano.h).

class LookupTable
{
public:
    LookupTable() : m_points(nullptr), m_count(0) {}

    LookupTable(const double* points, int count)
        : m_points(points), m_count(count) {}

    float getValue(float index) const
    {
        if (!m_points || m_count == 0)
            return 0.0f;

        // Clamp index to point range
        if (index <= static_cast<float>(m_points[0]))
            return static_cast<float>(m_points[1]);
        if (index >= static_cast<float>(m_points[(m_count - 1) * 2]))
            return static_cast<float>(m_points[(m_count - 1) * 2 + 1]);

        // Linear interpolation between the two nearest x,y pairs
        int lo = 0;
        int hi = m_count - 1;
        while (hi - lo > 1) {
            int mid = (lo + hi) / 2;
            if (static_cast<float>(m_points[mid * 2]) <= index)
                lo = mid;
            else
                hi = mid;
        }

        float x0 = static_cast<float>(m_points[lo * 2]);
        float y0 = static_cast<float>(m_points[lo * 2 + 1]);
        float x1 = static_cast<float>(m_points[hi * 2]);
        float y1 = static_cast<float>(m_points[hi * 2 + 1]);

        float t = (index - x0) / (x1 - x0);
        return y0 + t * (y1 - y0);
    }

private:
    const double* m_points;
    int m_count;
};

#endif // INSTRUMENT_H
