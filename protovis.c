#include "audio.h"
#include "midi.h"

#define AUDIO_IMPLEMENT
#include "aud_io.h"

#define ATTO_GL_H_IMPLEMENT
#include "atto/app.h"
#include "atto/gl.h"

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define VIS_BUF_SIZE 8192
static float vis_buf[VIS_BUF_SIZE];
static int vis_buf_pos = 0;
GLint vis_buf_pos_loc;

static void paint(ATimeUs ts, float dt) {
	(void)ts; (void)dt;

	glViewport(0, 0, a_app_state->width, a_app_state->height);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_LINE_STRIP, 0, VIS_BUF_SIZE);
}

int mctl[128];

static void audioCb(void *userdata, float *samples, int nsamples) {
	static int counter = 0;

	for (int i = 0; i < nsamples; ++i, ++counter) {
		samples[i] = sample(counter);
		vis_buf[vis_buf_pos] = samples[i];
		vis_buf_pos = (vis_buf_pos + 1) % VIS_BUF_SIZE;
	}
}

void midiCb(void *userdata, const unsigned char *data, int bytes) {
	(void)userdata;

	for (int i = 0; i < bytes; ++i) {
		const unsigned char b = data[i];
		if (!(b & 0x80))
			continue; /* skip unsync data bytes */

		if (bytes - i < 3)
			break; /* expect at least 2 args */

		int noteon = 0;
		const int channel = b & 0x0f;
		switch (b & 0xf0) {
			case 0x90: /* note on */
				noteon = 1;
			case 0x80: /* note off */
				{
					const int key = data[i + 1];
					const int vel = data[i + 2];
					fprintf(stderr, "MidiNoteO%s ch=%d n=%d v=%d\n", noteon ? "n" : "ff",
						channel, key, vel);
					i += 2;
				}
				break;

			case 0xb0: /* control change */
				{
					const int controller = data[i + 1];
					const int value = data[i + 2];
					fprintf(stderr, "MidiControl ch=%d p=%d v=%d\n", channel, controller, value);
					mctl[controller] = value;
					i += 2;
				}
				break;
		}
	}
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
		fprintf(stderr, "Shader error:\n%s\n", a_gl_error);
		aAppTerminate(1);
	}

	glUseProgram(p);
	const GLint sample_index = glGetAttribLocation(p, "sample");
	glUniform1f(glGetUniformLocation(p, "buf_len"), VIS_BUF_SIZE);
	vis_buf_pos_loc = glGetUniformLocation(p, "buf_pos");
	glVertexAttribPointer(sample_index, 1, GL_FLOAT, GL_FALSE, 0, vis_buf);
	glEnableVertexAttribArray(sample_index);

	proctable->paint = paint;

	const char *midi_device = getenv("MIDI");
	if (!audioOpen(44100, 1, 0, audioCb, midi_device, midiCb)) {
		fprintf(stderr, "Unable to open audio with MIDI=%s\n", midi_device);
		aAppTerminate(2);
	}
}
