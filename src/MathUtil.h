#pragma once
#include <stdint.h>

#define mabs(x)				((x) < 0 ? -(x) : (x))
#define msign(x)			(x == 0 ? 0 : (x < 0 ? -1 : 1))
#define mmin(x, y)			((x) < (y) ? (x) : (y))
#define mmax(x, y)			((x) > (y) ? (x) : (y))
#define upperRound(x, inc)	((((x) + inc - 1) / inc) * inc)
#define lowerRound(x, inc)	(((x) / inc) * inc)

void GetSine(uint8_t angle, int16_t *sin, int16_t *cos);
uint8_t GetAtan(int16_t x, int16_t y);
uint32_t RandomNumber();
