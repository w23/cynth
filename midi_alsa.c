#include "midi.h"
#include <alsa/asoundlib.h>
#include <stdio.h>

static snd_seq_t *seq;
static int port;

#define ALSA_CHECK(f) \
	if ((f) < 0) { \
		fprintf(stderr, "ERROR: Failed to " #f "\n"); \
		return 0; \
	}

int midi_open(const char *device) {
	ALSA_CHECK(snd_seq_open(&seq, device, SND_SEQ_OPEN_INPUT, SND_SEQ_NONBLOCK));
	ALSA_CHECK(snd_seq_set_client_name(seq, "Cynth"));
	ALSA_CHECK(port = snd_seq_create_simple_port(seq, "midi:in",
		SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_APPLICATION));
	return 1;
}

int midi_poll(MidiEvent *e) {
	snd_seq_event_t *ev = NULL;
	snd_seq_event_input(seq, &ev);
	if (!ev)
		return 0;

	int noteon = 0;
	switch(ev->type) {
		case SND_SEQ_EVENT_NOTEON:
			noteon = 1;
		case SND_SEQ_EVENT_NOTEOFF:
			e->type = noteon ? MidiNoteOn : MidiNoteOff;
			e->channel = ev->data.note.channel;
			e->note.velocity = ev->data.note.velocity;
			e->note.note = ev->data.note.note;
			return 1;

		case SND_SEQ_EVENT_CONTROLLER:
			e->type = MidiControl;
			e->channel = ev->data.control.channel;
			e->control.param = ev->data.control.param;
			e->control.value = ev->data.control.value;
			return 1;

		default:
			return 0;
	}
}

void midi_close() {}
