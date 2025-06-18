#ifndef UTILS_H
#define UTILS_H

#include <cstdlib>

inline float randf() {
    return static_cast<float>(rand()) / RAND_MAX;
}

#endif // UTILS_H