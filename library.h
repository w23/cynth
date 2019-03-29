static unsigned lcg() {
	static unsigned x = 0;
	const unsigned a = 1664525, c = 1013904223;
	x = a * x + c;
	return x;
}

static float noise() {
	return 2.f * (lcg() / (float)0xffffffffu) - 1.f;
}

typedef struct { float s, x; } SimpleHPF;
#define FILTER_SIMPLE_HP(name, in, alpha) \
	static SimpleHPF name = {0}; \
	{ \
		name.s = alpha * (name.s + in - name.x); \
		name.x = in; \
	}

typedef struct { float s; } SimpleLPF;
#define FILTER_SIMPLE_LP(name, in, alpha) \
	static SimpleHPF name = {0}; \
	name.s = alpha * in + (1.f - alpha) * name.s

typedef struct { float buf[65536]; } DelayLine;
#define DELAY(name) static DelayLine name = {0}
#define DELAY_WRITE(name, in) name.buf[count%COUNTOF(name.buf)] = (in)
#define DELAY_READ(name, delay) (name.buf[(count-delay)%COUNTOF(name.buf)])

