// Standalone regression check for the D8 "never coarsen, but still refine"
// rule (Claude-curve-lanes.md, R4). Not part of the Kala build - compile and
// run manually:
//
//   g++ -std=c++17 -I. -I$QT/include -I$QT/include/QtCore \
//       test_curve_dotdrag.cpp curve.cpp -L$QT/lib -lQt6Core -o test_dd.exe
//
// Exercises curveAfterDotDrag() from curve.cpp directly - the same function
// the canvas calls, so this cannot drift from production behaviour.

#include "curve.h"
#include <QtGlobal>
#include <cstdio>
#include <cmath>

static int failures = 0;
static void check(bool ok, const char *what)
{
    std::printf("%s  %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++failures;
}

int main()
{
    // ---- 1. Never coarsen -------------------------------------------------
    // A 30-point curve drawn in the dialog, dragged on a NARROW note (4 dots).
    // The old code rebuilt at numDots+2 = 6 points, destroying 24 of them.
    Curve dialogCurve;
    for (int i = 0; i < 30; ++i) {
        double t = i / 29.0;
        dialogCurve.addPoint(t, 0.5 + 0.4 * std::sin(t * 3.14159 * 3.0));
    }
    check(dialogCurve.getPointCount() == 30, "setup: dialog curve has 30 points");

    Curve dragged = curveAfterDotDrag(dialogCurve, 4, 0, 0.9);
    std::printf("     narrow-note drag: %d points survived (was 30, old code gave 6)\n",
                dragged.getPointCount());
    check(dragged.getPointCount() == 30, "narrow drag preserves all 30 points");

    // ---- 2. Still refine on widen ----------------------------------------
    // A flat 2-point curve on a WIDE note (20 dots) must gain resolution, or
    // stretch-the-note-wide stops buying precision.
    Curve flat(0.5);
    check(flat.getPointCount() == 2, "setup: flat curve has 2 points");

    Curve widened = curveAfterDotDrag(flat, 20, 5, 0.8);
    std::printf("     wide-note drag: %d points (numDots+2 = 22)\n",
                widened.getPointCount());
    check(widened.getPointCount() == 22, "wide drag refines a flat curve to 22 points");

    // ---- 3. The dragged dot actually lands on the target value -----------
    {
        const int numDots = 8, dotIndex = 3;
        const double td = double(dotIndex + 1) / (numDots + 1);
        Curve c(0.5);
        Curve r = curveAfterDotDrag(c, numDots, dotIndex, 0.85);
        std::printf("     value at dragged dot t=%.3f -> %.4f (want 0.85)\n",
                    td, r.valueAt(td));
        check(std::fabs(r.valueAt(td) - 0.85) < 1e-6, "dragged dot reaches its target value");
    }

    // ---- 4. Influence is local: endpoints and far points untouched -------
    {
        const int numDots = 8, dotIndex = 3;
        Curve r = curveAfterDotDrag(dialogCurve, numDots, dotIndex, 0.95);
        const double tPrev = double(dotIndex) / (numDots + 1);
        const double tNext = double(dotIndex + 2) / (numDots + 1);

        bool farUnchanged = true;
        for (const Curve::Point &p : r.getPoints()) {
            if (p.time <= tPrev || p.time >= tNext) {
                if (std::fabs(p.value - dialogCurve.valueAt(p.time)) > 1e-9) {
                    std::printf("     point t=%.4f changed by %.6f outside the dot's span\n",
                                p.time, p.value - dialogCurve.valueAt(p.time));
                    farUnchanged = false;
                }
            }
        }
        check(farUnchanged, "detail outside the dragged dot's two neighbours is untouched");
        check(std::fabs(r.valueAt(0.0) - dialogCurve.valueAt(0.0)) < 1e-9 &&
              std::fabs(r.valueAt(1.0) - dialogCurve.valueAt(1.0)) < 1e-9,
              "endpoints stay pinned");
    }

    // ---- 5. Repeated drags must not erode the curve ----------------------
    {
        Curve c = dialogCurve;
        for (int i = 0; i < 25; ++i)
            c = curveAfterDotDrag(c, 4, i % 4, 0.3 + 0.01 * i);
        std::printf("     after 25 narrow-note drags: %d points\n", c.getPointCount());
        check(c.getPointCount() == 30, "25 successive narrow drags still preserve 30 points");
    }

    std::printf("\n%s (%d failure%s)\n", failures ? "FAILED" : "ALL PASSED",
                failures, failures == 1 ? "" : "s");
    return failures ? 1 : 0;
}
