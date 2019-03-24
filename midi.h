#pragma once

typedef struct {
	enum { MidiControl, MidiNoteOn, MidiNoteOff } type;
	int channel;
	union {
		struct { int param, value; } control;
		struct { int velocity, note; } note;
	};
} MidiEvent;

int midi_open(const char *device);
int midi_poll(MidiEvent *event);
void midi_close();
