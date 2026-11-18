#include <ui/easing.h>

namespace Frontend {
    float easeOutCubic(float t) {
        const float inverse = 1.f - t;
        return 1.f - inverse * inverse * inverse;
    }

    float easeInOutCubic(float t) {
        if (t < 0.5f) {
            return 4.f * t * t * t;
        }
        const float inverse = -2.f * t + 2.f;
        return 1.f - inverse * inverse * inverse / 2.f;
    }

    float easeOutBack(float t, float overshoot) {
        const float u = t - 1.f;
        return 1.f + (overshoot + 1.f) * u * u * u + overshoot * u * u;
    }
}  // namespace Frontend
