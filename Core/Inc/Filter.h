/*
 * filter.h
 * Author: Gökçe
 */

#ifndef FILTER_H_
#define FILTER_H_

#include <stdint.h>

/*
 * Fonksiyon: filter_apply_volume
 * Amaç: Ses sinyalinin genliğini (ses seviyesini) değiştirir.
 * Parametreler:
 * - volume: 0.0 (Sessiz) ile 1.0 (Maksimum) arası katsayı.
 */
void filter_apply_volume(int16_t *input_buf, uint32_t length, float volume);

/*
 * Fonksiyon: filter_apply_ring
 * Amaç: Robotik ses efekti (Ring Modulator).
 */
void filter_apply_ring(int16_t *input_buf, uint32_t length, float *carrier_phase, float carrier_freq, uint32_t sampling_freq);

#endif /* FILTER_H_ */
