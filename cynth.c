#include "audio.h"
#include "midi.h"

#define AUDIO_IMPLEMENT
#include "aud_io.h"

int mctl[128];

#define VIS_BUF_SIZE 8192
static float vis_buf[VIS_BUF_SIZE];
static int vis_buf_pos = 0;
int vis_buf_pos_loc;

static void audioCb(void *userdata, float *samples, int nsamples) {
	static int counter = 0;

	for (int i = 0; i < nsamples; ++i, ++counter) {
		samples[i] = sample(counter);
		vis_buf[vis_buf_pos] = samples[i];
		vis_buf_pos = (vis_buf_pos + 1) % VIS_BUF_SIZE;
	}
}

static void midiCb(void *userdata, const unsigned char *data, int bytes) {
	(void)userdata;

	int updated = 0;
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
					updated = 1;
				}
				break;
		}
	}

	if (updated) {
		FILE *f = fopen("midi.state", "wb");
		if (f) {
			fwrite(mctl, sizeof(char), sizeof(mctl), f);
			fclose(f);
		}
	}
}

int cynthInit(const char *midi_device) {
	if (!audioOpen(44100, 1, 0, audioCb, midi_device, midiCb)) {
		fprintf(stderr, "Unable to open audio with MIDI=%s\n", midi_device);
		return 0;
	}

	return 1;
}
