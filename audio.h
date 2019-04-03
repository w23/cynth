#pragma once

/* Midi control state */
extern int mctl[128];

/* Midi notes state */
typedef struct {
	int gate;
	int samples_t;
	int key;
} MidiNoteState;

extern MidiNoteState mnote;

float sample(int t);
