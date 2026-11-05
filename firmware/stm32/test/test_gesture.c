/* What gesture.c does around the network, on a PC with a fake network. From firmware/stm32:
 *
 *   gcc -Wall -Itest/stub -ICore/Inc -ICore/AI Core/Src/gesture.c test/test_gesture.c -lm -o test_gesture.exe
 *   ./test_gesture.exe
 */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gesture.h"
#include "network.h"

static float input[6][GESTURE_MODEL_SAMPLES];
static float confidence[GESTURE_COUNT];
static int runs;

ai_error ai_network_create_and_init(ai_handle *network, const ai_handle activations[], const ai_handle weights[])
{
  *network = input;
  return (ai_error){AI_ERROR_NONE, 0};
}

ai_buffer *ai_network_inputs_get(ai_handle network, uint16_t *n_buffer)
{
  static ai_buffer in = {input};
  return &in;
}

ai_buffer *ai_network_outputs_get(ai_handle network, uint16_t *n_buffer)
{
  static ai_buffer out = {confidence};
  return &out;
}

int32_t ai_network_run(ai_handle network, const ai_buffer *in, ai_buffer *out)
{
  runs++;
  return 1;
}

/* Gravity along z, then rotation about z at deg_per_s for each entry's 0.1 s. */
static size_t stroke(imu_sample_t *samples, const int *deg_per_s, size_t stretches)
{
  size_t n = 0;
  for (int t = 0; t < GESTURE_MODEL_GRAVITY_SAMPLES; t++)
    samples[n++] = (imu_sample_t){.az = 8192};
  for (size_t i = 0; i < stretches; i++)
    for (int t = 0; t < 10; t++)
      samples[n++] = (imu_sample_t){.az = 8192, .gz = (int16_t)(deg_per_s[i] * 65.536f)};
  return n;
}

static void confident_in(int gesture, float value)
{
  for (int i = 0; i < GESTURE_COUNT; i++)
    confidence[i] = (1 - value) / (GESTURE_COUNT - 1);
  confidence[gesture] = value;
}

static void test_a_confident_stroke_gets_its_gesture(void)
{
  imu_sample_t samples[GESTURE_MODEL_SAMPLES];
  size_t n = stroke(samples, (int[]){300, 100}, 2);
  confident_in(GESTURE_LEFT, 0.999f);
  assert(gesture_classify(samples, n) == GESTURE_LEFT);
}

static void test_low_confidence_is_unsure(void)
{
  imu_sample_t samples[GESTURE_MODEL_SAMPLES];
  size_t n = stroke(samples, (int[]){300, 100}, 2);
  confident_in(GESTURE_LEFT, GESTURE_MODEL_THRESHOLD - 0.01f);
  assert(gesture_classify(samples, n) == -1);
}

static void test_a_wiggle_is_unsure_without_running_the_model(void)
{
  imu_sample_t samples[GESTURE_MODEL_SAMPLES];
  size_t n = stroke(samples, (int[]){300, -100, 300, -200}, 4);  // travels 90 degrees, ends 30 from the start
  confident_in(GESTURE_UP, 1);
  runs = 0;
  assert(gesture_classify(samples, n) == -1 && runs == 0);
}

static void test_a_still_press_is_unsure(void)
{
  imu_sample_t samples[GESTURE_MODEL_SAMPLES];
  size_t n = stroke(samples, (int[]){20, 20, 20}, 3);  // sensor drift: straight, but only 6 degrees
  confident_in(GESTURE_UP, 1);
  assert(gesture_classify(samples, n) == -1);
}

static void test_scales_and_pads(void)
{
  imu_sample_t samples[GESTURE_MODEL_SAMPLES];
  size_t n = stroke(samples, (int[]){300, 100}, 2);
  memset(input, 0x7f, sizeof input);
  gesture_classify(samples, n);
  assert(fabsf(input[2][0] - 0.25f) < 1e-6f);  // 8192 counts = 1 g = a quarter of full scale
  assert(fabsf(input[5][n - 1] - 100 / 500.0f) < 1e-3f);
  for (int ch = 0; ch < 6; ch++)
    assert(input[ch][n] == 0 && input[ch][GESTURE_MODEL_SAMPLES - 1] == 0);
}

static void test_undoes_twist_in_either_direction(void)
{
  imu_sample_t straight[GESTURE_MODEL_SAMPLES], twisted[GESTURE_MODEL_SAMPLES];
  size_t n = 60;
  srand(0);
  for (size_t t = 0; t < n; t++)
    straight[t] = (imu_sample_t){rand() % 18000 - 9000, rand() % 18000 - 9000, rand() % 18000 - 9000,
                                 rand() % 18000 - 9000, rand() % 18000 - 9000, rand() % 18000 - 9000};
  for (int t = 0; t < GESTURE_MODEL_GRAVITY_SAMPLES; t++)
    straight[t] = (imu_sample_t){.ax = 300, .az = 8192};
  gesture_classify(straight, n);
  float expected[6][GESTURE_MODEL_SAMPLES];
  memcpy(expected, input, sizeof input);

  int degrees[] = {-70, 45, 90};
  for (int i = 0; i < 3; i++)
  {
    float c = cosf(degrees[i] * (float)M_PI / 180), s = sinf(degrees[i] * (float)M_PI / 180);
    for (size_t t = 0; t < n; t++)
    {
      imu_sample_t r = straight[t];
      twisted[t] = (imu_sample_t){r.ax, lroundf(c * r.ay + s * r.az), lroundf(c * r.az - s * r.ay),
                                  r.gx, lroundf(c * r.gy + s * r.gz), lroundf(c * r.gz - s * r.gy)};
    }
    gesture_classify(twisted, n);
    for (int ch = 0; ch < 6; ch++)
      for (size_t t = 0; t < n; t++)
        assert(fabsf(input[ch][t] - expected[ch][t]) < 1e-3f);
  }
}

static void test_keeps_only_the_samples_the_model_takes(void)
{
  static imu_sample_t samples[GESTURE_MODEL_SAMPLES + 50];
  size_t n = 0;
  while (n < GESTURE_MODEL_SAMPLES + 50)
    samples[n++] = (imu_sample_t){.az = 8192, .gz = 3000};
  confident_in(GESTURE_DOWN, 1);
  assert(gesture_classify(samples, n) == GESTURE_DOWN);
}

static void test_nothing_recorded_is_unsure(void)
{
  assert(gesture_classify(NULL, 0) == -1);
}

int main(void)
{
  test_a_confident_stroke_gets_its_gesture();
  test_low_confidence_is_unsure();
  test_a_wiggle_is_unsure_without_running_the_model();
  test_a_still_press_is_unsure();
  test_scales_and_pads();
  test_undoes_twist_in_either_direction();
  test_keeps_only_the_samples_the_model_takes();
  test_nothing_recorded_is_unsure();
  puts("ok");
  return 0;
}
