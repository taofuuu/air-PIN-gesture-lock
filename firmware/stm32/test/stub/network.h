/* Stands in for the generated network, whose runtime only exists for ARM. test_gesture.c implements these. */
#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>

typedef uint8_t ai_u8;
typedef void *ai_handle;
typedef struct { ai_handle data; } ai_buffer;
typedef struct { int type, code; } ai_error;

#define AI_ALIGNED(x) _Alignas(x)
#define AI_HANDLE_NULL NULL
#define AI_ERROR_NONE 0

ai_error ai_network_create_and_init(ai_handle *network, const ai_handle activations[], const ai_handle weights[]);
ai_buffer *ai_network_inputs_get(ai_handle network, uint16_t *n_buffer);
ai_buffer *ai_network_outputs_get(ai_handle network, uint16_t *n_buffer);
int32_t ai_network_run(ai_handle network, const ai_buffer *input, ai_buffer *output);

#endif
