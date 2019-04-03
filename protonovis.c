#include "cynth.c"

int main(int argc, char *argv[]) {
	const char *midi_device = getenv("MIDI");
	if (!cynthInit(midi_device))
		return 1;

	for (;;) sleep(1);

	return 0;
}
