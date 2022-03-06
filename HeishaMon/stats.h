#ifndef STATS_H
#define STATS_H

struct Stats {
    float min = 66666.0F;
    float max = 0.0F;
    float sum = 0.0F;

    float avg(uint32_t count) const {
        return (count == 0) ? 0.0F : (sum / count);
    }

    void update(float value) {
        min = std::min(min, value);
        max = std::max(max, value);
        sum += value;
    }

    void reset() {
        min = 66666.0F;
        max = 0.0F;
        sum = 0.0F;
    }
};

#endif