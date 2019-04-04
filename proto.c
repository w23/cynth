#include "library.h"
#include "audio.h"
#include <math.h>

const int samplerate = 44100;

const int notes[] = { 0, 15, 17, 22 };

typedef struct {
	float phase;
	const int *notes;
	int notes_count;
	int notelen;
	int countoff;
	float a;
	float (*env)(float phase);
	float (*fmod)(float phase, float f);
	float (*wave)(float phase);
} voice_t;

const int bass[] = { 3, 5, 0, 0 };
float hash(float p) { return fmodf(sinf(p) * 43758.5453, 1.f); }
float release(float p) { return 1.f - p; }
float release2(float p) { return release(p) * release(p); }
float release4(float p) { return release2(p) * release2(p); }
float ar1(float p) {
	//return .5f;
	const float a = .003f;
	const float r = .06f;
	if (p < a) return p/a;
	p -= a;
	if (p < r) return 1.f - p/r;
	return 0.f;
}

float one(float p) { return 1.f; }
float fmodNone(float phase, float f) { return f; }
float fmodDrum(float phase, float f) { return f + 80.f * release4(phase); }

const int kick[] = { -12 };
const int peeper[] = {
	34, 34, 34, 34,
	34, 34, 34, 34,
	34, 34, 34, 34,
	34, 34, 34, 34,
	29, 29, 29, 29,
	29, 29, 29, 29,
	27, 27, 27, 27,
	27, 27, 27, 27,
	34, 34, 34, 34,
	34, 34, 34, 34,
	34, 34, 34, 34,
	34, 34, 34, 34,
	29, 29, 29, 29,
	29, 29, 29, 29,
	34, 34, 34, 34,
	34, 34, 34, 34,
};

voice_t voices[] = {
	{ 0, notes, COUNTOF(notes), 8129 * 4, 0, .9f, ar1, fmodNone, tri },
	//{ 0, bass, COUNTOF(bass), 8192 * 8, 0, .2f, one, fmodNone, sine },
	//{ 0, kick, COUNTOF(kick), 8192 * 4, 8192 * 2, .6f, release2, fmodDrum, sine },
	//{ 0, kick, COUNTOF(kick), 8192 / 2, 0, .1f, release2, fmodNone, hash},
	//{ 0, peeper, COUNTOF(peeper), 8192 / 2, 0, .15f, release2, fmodNone, tri},
};

float doVoice(unsigned int count, voice_t *v) {
	count += v->countoff;
	const int envn = count % v->notelen;
	const float notephase = envn / (float)(v->notelen);
	//const float env = v->env(notephase);
	const float env = v->env(envn / (float)samplerate);

	const int notenum = count / v->notelen;
	const int note = v->notes[notenum % v->notes_count];

	const float notefreq = 80.f * powf(2.f, note / 12.f);
	const float dp = v->fmod(notephase, notefreq) / samplerate;

	v->phase = fmodf(v->phase + dp, 1.f);

	return v->a * env * v->wave(v->phase);
}

const int primes[] = {
2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
283, 293, 307, 311, 313, 317, 331, 337, 347, 349,
353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
547, 557, 563, 569, 571, 577, 587, 593, 599, 601,
607, 613, 617, 619, 631, 641, 643, 647, 653, 659,
661, 673, 677, 683, 691, 701, 709, 719, 727, 733,
739, 743, 751, 757, 761, 769, 773, 787, 797, 809,
811, 821, 823, 827, 829, 839, 853, 857, 859, 863,
877, 881, 883, 887, 907, 911, 919, 929, 937, 941,
947, 953, 967, 971, 977, 983, 991, 997, 1009, 1013,
1019, 1021, 1031, 1033, 1039, 1049, 1051, 1061, 1063, 1069,
1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151,
1153, 1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223,
};

#if 0
#define LP_F mctl[2]
#define RC_C mctl[3]
#define RC_Q mctl[4]
#define REV_SIZE mctl[5]
#define REV_DRY mctl[6]
#else
#define LP_F mctl[21]
#define RC_C mctl[22]
#define RC_Q mctl[23]
#define REV_SIZE mctl[24]
#define REV_FB mctl[25]
#define REV_DRY mctl[26]
#define REV_SPRD mctl[27]
#endif


typedef struct { float xm2, xm1, s; } FilterAP;
#define FILTER_AP(name, in, za, zr) \
	static FilterAP name = {0}; \
	name.s = name.xm2 - 2.f * zr * cosf(za) * name.xm1 + zr * zr * in; \
	name.xm2 = name.xm1; name.xm1 = in

float sample(int t) {
	(void)t;
	static unsigned int count = 0;
	++count;

	float s = 0;
	for (int i = 0; i < COUNTOF(voices); ++i)
		s += doVoice(count, voices + i);

	FILTER_AP(apf, s, RC_C * M_PI * 2.f / 127.f, RC_Q / 127.f);
	s = apf.s;

	FILTER_RC(rcf, s, RC_C, RC_Q);
	//s = rcf.s;

	FILTER_LP(lpf, s, LP_F / 127.f);
	s = lpf.s;

	DELAY(delay);
	float rsig = 0;
	const float kr = REV_FB / 128.f;
	const float size = .001f + .2f * REV_SIZE / 127.f;
	const float spread = .001f + 2.f * REV_SPRD / 127.f;
	//const float spread = size * 16 * REV_SPRD / 127.f;
	const float min_size = size;
	const int N = 32;
	const int max_prime = primes[N];
	static int dbg = 1;
	const float r = .1f / 300.f;
	float A = 0.f;
	for (int i = 0; i < N; ++i) {
		const float l = min_size + (primes[i] * spread / max_prime);
		//const float k = r * r / (l*l);
		const float k = r / l;
		A += k;
		//const float k = r / l / N;
		//const float k = 1.f / N;
		//const float k = .9f * min_size / l / N;
		const unsigned offset = l * samplerate;
		//if (dbg) printf("%d %f %f %u\n", i, l, k, offset);
		rsig += k * (i%2 ? -1 : 1) * DELAY_READ(delay, offset);
	}
	rsig *= kr / A;
	FILTER_LP(lpdf, rsig, mctl[28] / 127.f);
	rsig = lpdf.s;

	dbg = 0;
	//s += .6f * DELAY_READ(delay, 8192 * 3 / 2);
	DELAY_WRITE(delay, - s + rsig);
	s = mix(s, rsig, REV_DRY / 127.f);

	return s;
}
