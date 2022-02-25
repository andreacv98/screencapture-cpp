#ifndef VIDEO_APP_CONTROLLER_H
#define VIDEO_APP_CONTROLLER_H


#include <mutex>
#include <condition_variable>
#include <thread>
#include "AudioDemuxer.h"
#include "VideoDemuxer.h"

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <cstring>
#include <math.h>
#include <string.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

#ifdef WIN32
#include <windows.h>
#endif

#ifdef __unix__
#include <X11/Xlib.h>
#endif

#include "utils.h"
#include "Decoder.h"
#include "Encoder.h"
#include "Muxer.h"

#ifdef __APPLE__
#define VIDEO_SOURCE ("avfoundation")
#define VIDEO_URL ("1:none")
#define AUDIO_SOURCE ("avfoundation")
#define AUDIO_URL ("none:0")
#endif

#ifdef __unix__
#define VIDEO_SOURCE ("x11grab")
#define VIDEO_URL (":0.0+0,0")
#define AUDIO_SOURCE ("pulse")
#define AUDIO_URL ("alsa_input.pci-0000_00_05.0.analog-stereo")
#endif

#ifdef _WIN32
#define VIDEO_SOURCE ("gdigrab")
#define VIDEO_URL ("desktop")
#define AUDIO_SOURCE ("dshow")
#define AUDIO_URL ("audio=Microfono (Logitech G533 Gaming Headset)")
#endif

class Controller {

private:
    //synchro stuff
    std::mutex r_mutex;
    std::condition_variable r_cv;
    std::mutex w_lock;

    //threads
    std::thread videoThread;
    std::thread audioThread;

    SRPacketBuffer inVideoBuffer;
    SRPacketBuffer inAudioBuffer;

    std::unique_ptr<AudioDemuxer> inAudio;
    std::unique_ptr<VideoDemuxer> inVideo;

    std::unique_ptr<Muxer> output;

    SRSettings settings;

    std::unique_ptr<Decoder> decoderAudio;
    std::unique_ptr<Decoder> decoderVideo;
    std::unique_ptr<Encoder> encoderAudio;
    std::unique_ptr<Encoder> encoderVideo;

    int inVideoStreamIndex;
    int inAudioStreamIndex;

    bool captureSwitch;
    bool captureStarted;
    bool killSwitch;

    AVAudioFifo *fifo;

    void captureVideo();
    void captureAudio();
    static int initConvertedSamples(uint8_t ***converted_input_samples,
                                    const AVCodecContext *output_codec_context,
                                    int frame_size);

    int init_fifo();

    int add_samples_to_fifo(uint8_t **converted_input_samples, const int frame_size);

public:
    Controller(char * audioUrl, char * videoUrl, SRSettings settings);
    ~Controller();

#ifdef __unix__
    Display *dpy;       //display from X11
#endif

    void startCapture();
    void pauseCapture();
    void resumeCapture();
    void endCapture();
    void initThreads();
    void infoDisplays();

private:
    void set();

};


#endif //VIDEO_APP_CONTROLLER_H
