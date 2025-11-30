#ifndef __RNG_H__
#define __RNG_H__

#include "debug.h"

void rng_init();
void rng_get(uint32_t *dst, uint32_t len);

#endif
