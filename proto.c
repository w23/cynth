#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#define AUDIO_IMPLEMENT
#include "aud_io.h"

#define COUNTOF(a) (sizeof(a) / sizeof(*(a)))

#if 1 // older C prototype
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

float sine(float p) { return sinf(p * 6.2832f); }
float tri(float p) { return p; }
float hash(float p) { return fmodf(sinf(p) * 43758.5453, 1.f); }
float release(float p) { return 1.f - p; }
float release2(float p) { return release(p) * release(p); }
float release4(float p) { return release2(p) * release2(p); }
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
	{ 0, notes, COUNTOF(notes), 4096, 0, .3f, release, fmodNone, tri },
	{ 0, bass, COUNTOF(bass), 4096 * 8, 0, .2f, one, fmodNone, sine },
	{ 0, kick, COUNTOF(kick), 4096 * 4, 4096 * 2, .6f, release2, fmodDrum, sine },
	{ 0, kick, COUNTOF(kick), 4096 / 2, 0, .1f, release2, fmodNone, hash},
	{ 0, peeper, COUNTOF(peeper), 4096 / 2, 0, .15f, release2, fmodNone, tri},
};

float delay_buffer[65536 / sizeof(float)] = { 0 };

float doVoice(unsigned int count, voice_t *v) {
	count += v->countoff;
	const int envn = count & (v->notelen-1);
	const float notephase = envn / (float)(v->notelen);
	const float env = v->env(notephase);

	const int notenum = count / v->notelen;
	const int note = v->notes[notenum % v->notes_count];

	const float notefreq = 80.f * powf(2.f, note / 12.f);
	const float dp = v->fmod(notephase, notefreq) / 20000.f;

	v->phase = fmodf(v->phase + dp, 1.f);

	//const float osc0 = .6f * env * sinf(notephase * 3.1415927f * 2.f);
	return v->a * env * v->wave(v->phase);//sinf(phase * 3.1415927f * 2.f);
}

unsigned char sample() {
	static unsigned int count = 0;
	++count;


	const int notes[] = {
		0, 3, 7, 5,
		7, 5, 12, 15,
	};
	//note_hz = 440 * powf(2, note_midi / 12);

	unsigned int hz = 110 * powf(2.f, notes[(count>>12)%COUNTOF(notes)] / 12.f);
	unsigned short phase = count * (65535u * hz / 20000u);// ; 65k/20k

	//unsigned int hez = 1;
	unsigned short env = (count & ((1<<12) - 1)) << (16 - 12); // count * (65535u * hez / 20000u);// ; 65k/20k
	//float freq = 100.f;
	//float f = fmodf(freq * count / 20000.f, 1.f);
	uint16_t dx = ((int16_t)phase * (int16_t)phase) >> 16;
	unsigned char s = ((0xffffu - env) * (dx >> 8)) >> 16;

	static uint8_t dbuf[65536];
	const int buf_write = count % COUNTOF(dbuf);
	const int buf_read = (buf_write + COUNTOF(dbuf) - 4096 * 3 / 2) % COUNTOF(dbuf);

	return dbuf[buf_write] = s + (dbuf[buf_read] >> 1);
	/*

	//return (phase * (0xffffu - phase)) >> 28;

	return 0;//phase >> 12;// sinf(2.f * 3.1415926f * f);

	float s = 0;
	for (int i = 0; i < COUNTOF(voices); ++i)
		s += doVoice(count, voices + i);

	const int buf_write = count % COUNTOF(delay_buffer);
	const int buf_read = (buf_write + COUNTOF(delay_buffer) - 4096 * 3 / 2) % COUNTOF(delay_buffer);

	return 0.;//delay_buffer[buf_write] = s + .6f * delay_buffer[buf_read];
	*/
}
#endif

const int asmArpNote[] = {
	22, 17, 15, 12-12,
	24, 22, 17, 15-12,
	27, 24, 22, 17-12,
	29, 24, 17, 24-12,
};
const int asmBassNote[] = {
	15, 17, 12, 12
};
const int asmHhNote[] = {
	36, 34, 27, 29,
	27, 29, 34, 31,
};

#define DP(hz) ((hz-88)*255/(314-88))
#define DP1(hz) (DP(hz/2))
int asmArp[COUNTOF(asmArpNote)] = { DP(88), DP(209), DP(235), DP(314), };
int asmBass[COUNTOF(asmBassNote)] = { DP(105), DP(117), DP(88), DP(88), };
int asmHh[COUNTOF(asmHhNote)] = {
	DP1(627), DP1(627), DP1(470), DP1(419),
	DP1(627), DP1(627), DP1(470), DP1(527),
};

static struct {
	float kp_min, kp_max;
	float a1, m2;
	int mul, shift, add;
} stat = {
	20000.f, 0, -88.f, 255.f  / (314 - 88),
	37, 4, 231
};

uint16_t asmNote(unsigned int count, const int *buf, uint16_t cx) {
	uint16_t bp = count >> 8;
	bp += bp;
	bp &= cx;
	bp >>= cx & 0xff;
	const int note = buf[bp];
	uint16_t ax = stat.mul;
	ax *= note;
	ax >>= stat.shift;
	ax += stat.add;
	ax *= count;
	return ax;
}

uint8_t asmBuffer[65536] = { 0 };

uint8_t asmSample() {
	static unsigned int count = 0;
	++count;

	uint8_t s = 0;

	uint16_t cx = 5 + ((COUNTOF(asmArp) - 1) << 5);
	uint16_t ax = asmNote(count, asmArp, cx);
	uint16_t bp = count * -16;
	uint16_t dx = (ax * bp) >> 16;
	dx >>= 11;
	s += dx;

#if 0
	// NEW
	const float a = .95f * (.5f * (1.f + sinf(count/16384.f)));
	static float z = 0;
 	z = (s / 255.f - .5f) * (1.f - a) + z * a;
	s = (z + .5f) * 255.f;
#endif

	cx = 8 + ((COUNTOF(asmBass) - 1) << 8);
	ax = asmNote(count, asmBass, cx);
	dx = ((int16_t)ax * (int16_t)ax) >> 16;
	s += dx >> 8;

	cx = 8 + ((COUNTOF(asmHh) - 1) << 8);
	ax = asmNote(count, asmHh, cx);
	ax += ax;
	bp = count * -32;
	dx = (ax * bp) >> 16;
	dx >>= 12;
	s += dx;

	const int buf_write = count % COUNTOF(asmBuffer);
	const int buf_read = (buf_write + COUNTOF(asmBuffer) - 4096 * 3 / 2) % COUNTOF(asmBuffer);

	return asmBuffer[buf_write] = s + asmBuffer[buf_read] / 2;
}

float asmNoteFreq(int note) {
	return 22.f * powf(2.f, (note+20) / 12.f);
}

void asmNotesAnalyze(const int *notes, int count, float scale) {
	for (int i = 0; i < count; ++i) {
		const int note = notes[i];
		const float freq = asmNoteFreq(note) * scale;
		const float kphase = freq * 65536.f / 20000.f;

		stat.kp_min = (stat.kp_min < kphase) ? stat.kp_min : kphase;
		stat.kp_max = (stat.kp_max > kphase) ? stat.kp_max : kphase;
	}
}

void asmNotesToMultipliers(const int *notes, int count, int *muls, float scale) {
	for (int i = 0; i < count; ++i) {
		const int note = notes[i];
		const float freq = asmNoteFreq(note) * scale;
		const float kphase = freq * 65536.f / 20000.f;

		const float mulp = (kphase + stat.a1) * stat.m2;
		const int result = roundf(mulp);

		const int r_kphase = ((result * stat.mul) >> stat.shift) + stat.add;
		const float r_freq = r_kphase * 20000.f / 65536.f;
		const float r_note = 12.f * log2f(r_freq/(22.f*scale)) - 24;

		fprintf(stderr, "%d\t=>\t%.2f\t%.2f" // note => freq	kphase
			"\t%.2f\t%s%d%s" // scaled_kphase	(int)scaled_kphase
			"\t%d(%.2f)" // restored_kphase (delta)
			"\t%.2f(%.2f)" // restored_freq (delta)
			"\t%s%.2f(%.2f)%s" // restored_note (delta)
			"\n"
			, note, freq, kphase, mulp
			, result>255 ? "\033[31m" : "", result, "\033[0m"
			, r_kphase, r_kphase - kphase
			, r_freq, r_freq - freq
			, fabs(r_note - note) > .1f ? "\033[33m" : "", r_note, r_note - note, "\033[0m"
		);
		muls[i] = result;
	}
	fprintf(stderr, "\n");
}

void asmDumpMuls(const int *muls, int count) {
	fprintf(stderr, "\tdb");
	for (int i = 0; i < count; ++i) {
		fprintf(stderr, " %d,", muls[i]);
	}
	fprintf(stderr, "\n");
}

static void audioCb(void *userdata, float *samples, int nsamples) {
	static float s1, s2;
	const uint16_t ds = (20000 * 65536) / 44100;
	static uint16_t s = 0;
	if (s == 0) {
		s1 = (asmSample() - 127.f) / 127.f;
		s2 = (asmSample() - 127.f) / 127.f;
	}

	for (int i = 0; i < nsamples; ++i) {
		const uint16_t ps = s;
		s += ds;
		if (s < ps) {
			s1 = s2;
			s2 = (asmSample() - 127.f) / 127.f;
		}

		const float t = s / 65536.f;

		samples[i] = s1 + (s2 - s1) * t;
	}
}

#define BUFSIZE 1024
static unsigned char buffer[BUFSIZE];
int main(int argc, char *argv[]) {

	// init asm notes
	asmNotesAnalyze(asmArpNote, COUNTOF(asmArpNote), 1.f);
	asmNotesAnalyze(asmBassNote, COUNTOF(asmBassNote), .5f);
	asmNotesAnalyze(asmHhNote, COUNTOF(asmHhNote), .5f);

	const float kp_range = stat.kp_max - stat.kp_min;
	const float k_range_u8 = kp_range / 255.f;
	fprintf(stderr, "kphase min=%f max=%f range=%f ku8=%f\n", stat.kp_min, stat.kp_max, kp_range, k_range_u8);
	stat.add = stat.kp_min;
	stat.shift = 4;
	stat.mul = k_range_u8 * powf(2.f, stat.shift);
	stat.a1 = -stat.kp_min;
	stat.m2 = 1.f / k_range_u8;
	fprintf(stderr, "stat a1=%f m2=%f add=%d mul=%d shift=%d\n",
		stat.a1, stat.m2, stat.add, stat.mul, stat.shift);

	asmNotesToMultipliers(asmArpNote, COUNTOF(asmArpNote), asmArp, 1.f);
	asmNotesToMultipliers(asmBassNote, COUNTOF(asmBassNote), asmBass, .5f);
	asmNotesToMultipliers(asmHhNote, COUNTOF(asmHhNote), asmHh, .5f);

	asmDumpMuls(asmArp, COUNTOF(asmArp));
	asmDumpMuls(asmBass, COUNTOF(asmBass));
	asmDumpMuls(asmHh, COUNTOF(asmHh));

	int out_std = 0;
	for (int i = 1; i < argc; ++i) {
		const char *arg = argv[i];
		if (strcmp(arg, "-") == 0) {
			out_std = 1;
		} else if (strcmp(arg, "d") == 0) {
			for (int j = 0; j < 512; ++j) {
				for (int i = 0; i < BUFSIZE; ++i)
					buffer[i] = asmSample();
				if (BUFSIZE != write(1, buffer, BUFSIZE))
					return 1;
			}
			return 0;
		}
	}

	if (out_std) {
		for (;;) {
			for (int i = 0; i < BUFSIZE; ++i)
				buffer[i] = asmSample();//sample();// + 1.f) * 128.f;
			if (BUFSIZE != write(1, buffer, BUFSIZE))
				return 1;
		}
	} else {
		audioOpen(44100, 1, 0, audioCb, NULL, NULL);
		for (;;) sleep(1);
	}

	audioClose();
	return 0;
}
