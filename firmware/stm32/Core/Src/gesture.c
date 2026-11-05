/* Runs the trained model on one recording.
 *
 * Core/AI is generated from ml/model/. preprocess() and one_movement() repeat the functions of the
 * same name in ml/train.py and must keep matching them, or the model sees numbers it wasn't trained on. */
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "gesture.h"
#include "network.h"
#include "network_data.h"

#define CHANNELS 6                 // accel x, y, z, then gyro x, y, z
#define SENSOR_FULL_SCALE 32768.0f // counts that the model sees as 1.0
#define GYRO_FULL_SCALE 500.0f     // deg/s at SENSOR_FULL_SCALE

typedef float input_t[CHANNELS][GESTURE_MODEL_SAMPLES];

AI_ALIGNED(4) static ai_u8 activations[AI_NETWORK_DATA_ACTIVATIONS_SIZE];
static ai_handle network = AI_HANDLE_NULL;

static int network_ready(void)
{
  if (network != AI_HANDLE_NULL)
    return 1;
  const ai_handle buffers[] = {activations};
  ai_error err = ai_network_create_and_init(&network, buffers, NULL);
  if (err.type == AI_ERROR_NONE)
    return 1;
  printf("gesture: network init failed, type %d code %d\r\n", (int)err.type, (int)err.code);
  network = AI_HANDLE_NULL;
  return 0;
}

static void preprocess(const imu_sample_t *samples, size_t n, input_t x)
{
  // Gravity correction: turn around the wand's length (x) until gravity is z
  size_t still = n < GESTURE_MODEL_GRAVITY_SAMPLES ? n : GESTURE_MODEL_GRAVITY_SAMPLES;
  int32_t ay = 0, az = 0;
  for (size_t t = 0; t < still; t++)
  {
    ay += samples[t].ay;
    az += samples[t].az;
  }
  float roll = atan2f(ay, az), c = cosf(roll) / SENSOR_FULL_SCALE, s = sinf(roll) / SENSOR_FULL_SCALE;

  for (size_t t = 0; t < n; t++)
  {
    const imu_sample_t *r = &samples[t];
    x[0][t] = r->ax / SENSOR_FULL_SCALE;
    x[1][t] = c * r->ay - s * r->az;
    x[2][t] = s * r->ay + c * r->az;
    x[3][t] = r->gx / SENSOR_FULL_SCALE;
    x[4][t] = c * r->gy - s * r->gz;
    x[5][t] = s * r->gy + c * r->gz;
  }
  for (int ch = 0; ch < CHANNELS; ch++)
    memset(&x[ch][n], 0, (GESTURE_MODEL_SAMPLES - n) * sizeof(float));
}

/* Whether the wand turned far enough, mostly one way. Catches what the model can't: it never sees
 * a wiggle or an accidental press in training. */
static int one_movement(input_t x)
{
  float gx = 0, gy = 0, gz = 0, path = 0;
  for (int t = 0; t < GESTURE_MODEL_SAMPLES; t++)
  {
    gx += x[3][t];
    gy += x[4][t];
    gz += x[5][t];
    path += sqrtf(x[3][t] * x[3][t] + x[4][t] * x[4][t] + x[5][t] * x[5][t]);
  }
  float to_degrees = GYRO_FULL_SCALE / GESTURE_MODEL_RATE_HZ;
  float turned = sqrtf(gx * gx + gy * gy + gz * gz) * to_degrees;  // from start to end
  path *= to_degrees;                                              // on the way
  return turned >= GESTURE_MODEL_MIN_TURN && turned >= GESTURE_MODEL_MIN_STRAIGHTNESS * path;
}

int gesture_classify(const imu_sample_t *samples, size_t n)
{
  if (n == 0 || !network_ready())
    return -1;
  if (n > GESTURE_MODEL_SAMPLES)
    n = GESTURE_MODEL_SAMPLES;

  // The input and output buffers live inside activations, so inference overwrites the input
  ai_buffer *in = ai_network_inputs_get(network, NULL), *out = ai_network_outputs_get(network, NULL);
  preprocess(samples, n, in[0].data);
  if (!one_movement(in[0].data) || ai_network_run(network, in, out) != 1)
    return -1;

  const float *confidence = out[0].data;
  int best = 0;
  for (int i = 1; i < GESTURE_COUNT; i++)
    if (confidence[i] > confidence[best])
      best = i;
  return confidence[best] < GESTURE_MODEL_THRESHOLD ? -1 : best;
}
