#pragma once
#include <math.h>

#define COUNTOF(a) (sizeof(a) / sizeof(*(a)))

static unsigned lcg() {
	static unsigned x = 0;
	const unsigned a = 1664525, c = 1013904223;
	x = a * x + c;
	return x;
}

static float noise() {
	return 2.f * (lcg() / (float)0xffffffffu) - 1.f;
}

float square(float p) { return p < .5f ? -1.f : 1.f; }
float sine(float p) { return sinf(p * 6.2832f); }
float saw(float p) { return p*2.f - 1.f; }
float tri(float p) { return 4.f * (fabs(p-.5f)-.25f); }

static float mix(float a, float b, float t) { return a + (b-a)*t; }

#define MTOF(note) (44.f * powf(2.f, note/12.f))

typedef struct { float s, x; } FilterHP;
#define FILTER_HP(name, in, alpha) \
	static FilterHP name = {0}; \
	{ \
		const float input = in; \
		name.s = (alpha) * (name.s + input - name.x); \
		name.x = input; \
	}

typedef struct { float s; } FilterLP;
#define FILTER_LP(name, in, alpha) \
	static FilterHP name = {0}; \
	name.s = alpha * in + (1.f - alpha) * name.s

typedef struct { float buf[2*65536]; } DelayLine;
#define DELAY(name) static DelayLine name = {0}
#define DELAY_WRITE(name, in) name.buf[count%COUNTOF(name.buf)] = (in)
#define DELAY_READ(name, delay) (name.buf[(count-delay)%COUNTOF(name.buf)])

typedef struct { float s, s1; } FilterRC;
#define FILTER_RC(name, in, cutoff, resonance) \
	static FilterRC name = {0}; \
	{ \
		const float c = powf(0.5f, (128 - cutoff) / 16.f); \
		const float r = powf(0.5f, (resonance + 24) / 16.f); \
		name.s = (1.f - r*c) * name.s - c * name.s1 + c * in; \
		name.s1 = (1.f - r*c) * name.s1 + c * name.s; \
	}
