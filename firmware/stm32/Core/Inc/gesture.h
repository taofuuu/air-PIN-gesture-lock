/* Turns one recording into a gesture, using the model trained in ml/. */
#ifndef GESTURE_H
#define GESTURE_H

#include <stddef.h>

#include "gesture_model.h"
#include "imu.h"

typedef enum { GESTURE_UP, GESTURE_DOWN, GESTURE_LEFT, GESTURE_RIGHT, GESTURE_COUNT } gesture_t;

/* samples is one button hold, press to release; only the first GESTURE_MODEL_SAMPLES are used.
 * Returns a gesture_t, or -1 if unsure. */
int gesture_classify(const imu_sample_t *samples, size_t n);

#endif /* GESTURE_H */
