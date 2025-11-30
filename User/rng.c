#include "rng.h"

void rng_init() {
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_RNG, ENABLE);
    RNG_Cmd(ENABLE);

    // TODO: Seed?
}

// Get len 32-bit random words to dst
void rng_get(uint32_t *dst, uint32_t len) {    
    uint32_t i;

    for (i = 0; i < len; i++) {
        // Wait for new random word
        while (RNG_GetFlagStatus(RNG_FLAG_DRDY) == RESET);
        dst[i] = RNG_GetRandomNumber();
    }
}
