#ifndef VIDEO_APP_SCREENRECORDER_H
#define VIDEO_APP_SCREENRECORDER_H

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
//#include <semaphore.h>

#ifdef WIN32
#include <windows.h>
#endif

#ifdef __unix__
#include <X11/Xlib.h>
#endif


//FFMPEG LIBRARIES
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavcodec/avfft.h"
#include "libavdevice/avdevice.h"
#include "libavfilter/avfilter.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"

#include "libavutil/opt.h"
#include "libavutil/common.h"
#include "libavutil/channel_layout.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"
#include "libavutil/time.h"
#include "libavutil/opt.h"
#include "libavutil/pixdesc.h"
#include "libavutil/file.h"
#include "libavutil/audio_fifo.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"

}
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

#define CAPTURE_BUFFER 10

typedef struct S{
    int width;
    int height;
}SRResolution;

typedef struct T{
    int x;
    int y;
}SROffset;

typedef struct A{
    bool _recaudio;
    bool _recvideo;
    SRResolution  _inscreenres;
    SRResolution  _outscreenres;
    SROffset _screenoffset;
    uint16_t  _fps;
    char* filename;
}SRSettings;

typedef struct B{
    int np;
    std::queue<AVPacket*> buf;
}SRPacketBuffer;

class ScreenRecorder {

private:


    //synchro stuff
    std::mutex r_mutex;
    std::condition_variable r_cv;
    std::mutex w_lock;

    //threads
    std::thread videoThread;
    std::thread audioThread;
    std::thread producerThread;

    SRPacketBuffer inVideoBuffer;
    SRPacketBuffer inAudioBuffer;

    //video
    AVInputFormat *inVInputFormat;
    AVFormatContext *inVFormatContext;
    AVDictionary *inVOptions;
    AVCodecContext *inVCodecContext;
    AVCodec *inVCodec;

    AVFormatContext *outAVFormatContext;
    AVDictionary *outVOptions;
    AVCodecContext *outVCodecContext;
    AVCodec *outVCodec;

    //audio
    AVDictionary *inAOptions;
    AVFormatContext *inAFormatContext;
    AVInputFormat *inAInputFormat;
    AVCodecContext *inACodecContext;

    AVCodecContext *outACodecContext;
    AVCodec *outACodec;
    AVCodec *inACodec;


    AVFrame *rawVideoFrame;
    AVFrame *rawAudioFrame;

    //output
    AVOutputFormat *outAVOutputFormat;

    int inVideoStreamIndex;
    int inAudioStreamIndex;
    int outVideoStreamIndex;
    int outAudioStreamIndex;

    bool captureSwitch;
    bool killSwitch;

    AVAudioFifo *fifo;

    void generateVideoOutputStream();
    void generateAudioOutputStream();
    void captureVideo();
    void captureAudio();
    void produce();
    void initOptions();
    void initBuffers();
    static int initConvertedSamples(uint8_t ***converted_input_samples,
                                    AVCodecContext *output_codec_context,
                                    int frame_size);
public:
    SRSettings settings;
    #ifdef __unix__
    Display *dpy;       //display from X11
    #endif


    ScreenRecorder();
    ~ScreenRecorder();

    int openVideoSource();
    int openAudioSource();
    int initOutputFile();

    void startCapture();
    void pauseCapture();
    void endCapture();

    void initThreads();

    int init_fifo();

    int add_samples_to_fifo(uint8_t **converted_input_samples, const int frame_size);

    void infoDisplays();
    void listDevices();
};



#endif //VIDEO_APP_SCREENRECORDER_H
