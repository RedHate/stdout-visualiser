// start the program like visualizer plug:hw:1 plug:hw:1 


#include <alloca.h>S
#include <time.h>
#include <pthread.h>
#include <alsa/asoundlib.h>

// Alsa related definitions
#define SAMPLE_RATE 4000
#define CHANNELS    1
#define FRAME_SIZE  4 //I asked ai and it says this is the best trade off between latency and bandwidth when using 8bit 4000 "32kbps" 20ms (62kb on the line)
#define FORMAT      SND_PCM_FORMAT_S16_LE

int init_alsa(snd_pcm_t **handle, const char *device, snd_pcm_stream_t stream, uint32_t channels) {
    
    // Local vars
    int err;
    snd_pcm_hw_params_t *params;
    
    // Init a handle for the PCM steam from a chosen device
    if ((err = snd_pcm_open(handle, device, stream, 0)) < 0) {
        fprintf(stderr, "ALSA open error: %s\n", snd_strerror(err));
        return err;
    }
	
	// I assume this allocates memory for params, return doesn't allow error checking
	snd_pcm_hw_params_alloca(&params);
	
	// Init all channel params and error check
    if ((err = snd_pcm_hw_params_any(*handle, params)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_any error: %s\n", snd_strerror(err));
		return err;
	}
	
	// (ignore that gcc throws warning on this or fix it lol)
	// set resample since we want to adjust the rate 
	uint32_t resample = 1;
    if ((err = snd_pcm_hw_params_set_rate_resample(*handle, params, *(unsigned int*)&resample)) < 0) {
        //fprintf(stderr, "ALSA snd_pcm_hw_params_set_rate_resample error: %s\n", snd_strerror(err));
		return err;
	}
	// Set access
    if ((err = snd_pcm_hw_params_set_access(*handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_set_access error: %s\n", snd_strerror(err));
		return err;
	}
	// Set format
    if ((err = snd_pcm_hw_params_set_format(*handle, params, FORMAT)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_set_format error: %s\n", snd_strerror(err));
		return err;
	}
	
	// Set channels
    snd_pcm_hw_params_set_channels(*handle, params, channels);
    
	// Set rate
	uint32_t sample_rate = SAMPLE_RATE;
    if ((err = snd_pcm_hw_params_set_rate_near(*handle, params, &sample_rate, 0)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params_set_rate error: %s\n", snd_strerror(err));
		return err;
	}
	// Set hw params
    if ((err = snd_pcm_hw_params(*handle, params)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_hw_params error: %s\n", snd_strerror(err));
		return err;
	}
	// Prepare for pcm
    if ((err = snd_pcm_prepare(*handle)) < 0) {
        fprintf(stderr, "ALSA snd_pcm_prepare error: %s\n", snd_strerror(err));
		return err;
	}

    printf("ALSA initialized at %u Hz for %d\n", sample_rate, stream);

    return 0;
    
}

void audio_visualiser(short *buffer, size_t size, int color_mode) {
	
	/*
	 * This is honestly one of the coolest things I have ever made
	 * such a simple thing and looks so neat
	 */
	
	// Visualiser array
	char visualiser_array[16][16]={
		"+",
		"++",
		"+++",
		"++++",
		"+++++",
		"++++++",
		"+++++++",
		"++++++++",
		"+++++++++",
		"++++++++++",
		"+++++++++++",
		"++++++++++++",
		"+++++++++++++",
		"++++++++++++++",
		"+++++++++++++++",
		"++++++++++++++++",
	};
	
	// Terminal color strings
	char color_strings[8][10]={
		"\033[1;37m",//WHITE
		"\033[1;34m",//BLUE
		"\033[1;36m",//CYAN
		"\033[1;32m",//GREEN
		"\033[1;33m",//YELLOW
		"\033[1;31m",//RED
		"\033[1;35m",//MAGENTA
		"\033[1;37m",//WHITE1
		//"\033[0m", //NOCOLOR
	};
	
	// Main visualiser loop
	int c;
	for(c=0;c<sizeof(buffer);c++) {
		// Type cast and reduce to 8 bit
		char byte = (char)(buffer[c]/255);
		// Check upper and lower signal boundaries
		if((byte >= 0) && (byte <= 127)) {
			// Are we in bounds of the color array?
			if((byte/32 >= 0) && (byte/32 <= 8))
				// Print the pretty stuff
				printf("[%3d]%s                 | %s\033[0m\n", *(char*)&byte, (color_mode)? color_strings[6] : color_strings[((int)(byte/8))/2],   visualiser_array[(int)(byte/8)]);
		}
		// Check upper and lower signal boundaries
		if((byte <= 0) && (byte >= -127)) {
			// Are we in bounds of the color array?
			if((~byte/32 >= 0) && (~byte/32 <= 8))
				// Print the pretty stuff
				printf("[%3d]%s%16s |\033[0m\n", *(char*)&byte, (color_mode)? color_strings[2] : color_strings[((int)(~byte/6))/2], visualiser_array[(int)(~byte/8)]);
		}
	}
			
}

int main(int argc, char *argv[]) {
	
	// Set up capture and plauback handles
	snd_pcm_t *capture_handle, *playback_handle;
	
	// Init capture and playback devices
	init_alsa(&capture_handle, argv[1], SND_PCM_STREAM_CAPTURE, CHANNELS);
	init_alsa(&playback_handle, argv[2], SND_PCM_STREAM_PLAYBACK, CHANNELS);
	
	// Set up a buffer
	short buffer[FRAME_SIZE*CHANNELS];
	
	// Get loopy!
	while(1) {
		// Read some samples
		ssize_t r = snd_pcm_readi(capture_handle, buffer, sizeof(buffer));
		if (r > 0) {
			// Write some samples
			snd_pcm_writei(playback_handle, buffer, r);
			// 1 or 0 for rainbow vs pink and blue
			int color_mode = 1;
			// Draw the visualiser
			audio_visualiser(buffer, r, color_mode);
		}
	}
	
	return 0;
}
