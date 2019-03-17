#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "atto/app.h"
#define AUDIO_IMPLEMENT
#include "aud_io.h"
#define ATTO_GL_H_IMPLEMENT
#include "atto/gl.h"

#define COUNTOF(a) (sizeof(a) / sizeof(*(a)))

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
	{ 0, notes, COUNTOF(notes), 8192, 0, .3f, release, fmodNone, tri },
	//{ 0, bass, COUNTOF(bass), 8192 * 8, 0, .2f, one, fmodNone, sine },
	//{ 0, kick, COUNTOF(kick), 8192 * 4, 8192 * 2, .6f, release2, fmodDrum, sine },
	//{ 0, kick, COUNTOF(kick), 8192 / 2, 0, .1f, release2, fmodNone, hash},
	//{ 0, peeper, COUNTOF(peeper), 8192 / 2, 0, .15f, release2, fmodNone, tri},
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
	const float dp = v->fmod(notephase, notefreq) / 44100.f;

	v->phase = fmodf(v->phase + dp, 1.f);

	return v->a * env * v->wave(v->phase);
}

float sample() {
	static unsigned int count = 0;
	++count;

	float s = 0;
	for (int i = 0; i < COUNTOF(voices); ++i)
		s += doVoice(count, voices + i);

	const int buf_write = count % COUNTOF(delay_buffer);
	const int buf_read = (buf_write + COUNTOF(delay_buffer) - 8192 * 3 / 2) % COUNTOF(delay_buffer);

	return delay_buffer[buf_write] = s + .6f * delay_buffer[buf_read];
}

#define VIS_BUF_SIZE 8192
static float vis_buf[VIS_BUF_SIZE];
static int vis_buf_pos = 0;

static void audioCb(void *userdata, float *samples, int nsamples) {
	for (int i = 0; i < nsamples; ++i) {
		samples[i] = sample();
		vis_buf[vis_buf_pos] = samples[i];
		vis_buf_pos = (vis_buf_pos + 1) % VIS_BUF_SIZE;
	}
}

GLint vis_buf_pos_loc;

static void paint(ATimeUs ts, float dt) {
	(void)ts; (void)dt;

	glViewport(0, 0, a_app_state->width, a_app_state->height);
	glClear(GL_COLOR_BUFFER_BIT);
	//glUniform1i(vis_buf_pos_loc, vis_buf_pos);
	glDrawArrays(GL_LINE_STRIP, 0, VIS_BUF_SIZE);
}

static const char *vertex[] = {
"#version 130\n\n"
"attribute float sample;\n"
"uniform float buf_len;\n"
"uniform int buf_pos;\n"
"void main() {\n"
	"gl_Position = vec4(\n"
	"float(gl_VertexID+buf_pos)/buf_len*2.-1., sample, 0., 1.);\n"
"}\n",
NULL};

static const char *fragment[] = {
"void main() { gl_FragColor = vec4(1.); }\n", NULL};

void attoAppInit(struct AAppProctable *proctable) {
	(void)proctable;

	const AGLProgram p = aGLProgramCreate(vertex, fragment);
	if (p < 0) {
		printf("Shader error:\n%s\n", a_gl_error);
	}

	glUseProgram(p);
	const GLint sample_index = glGetAttribLocation(p, "sample");
	glUniform1f(glGetUniformLocation(p, "buf_len"), VIS_BUF_SIZE);
	vis_buf_pos_loc = glGetUniformLocation(p, "buf_pos");
	glVertexAttribPointer(sample_index, 1, GL_FLOAT, GL_FALSE, 0, vis_buf);
	glEnableVertexAttribArray(sample_index);

	proctable->paint = paint;

	audioOpen(44100, 1, 0, audioCb, NULL, NULL);
	//audioClose();
}
