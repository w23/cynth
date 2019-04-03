#define ATTO_GL_H_IMPLEMENT
#include "atto/app.h"
#include "atto/gl.h"

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "cynth.c"

static void paint(ATimeUs ts, float dt) {
	(void)ts; (void)dt;

	glViewport(0, 0, a_app_state->width, a_app_state->height);
	glClear(GL_COLOR_BUFFER_BIT);
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
	if (!cynthInit(midi_device))
		aAppTerminate(2);
}
