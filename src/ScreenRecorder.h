//
// Created by andrea on 19/12/21.
//

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

//FFMPEG LIBRARIES

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavcodec/avfft.h>
#include <libavdevice/avdevice.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavformat/avformat.h>
#include <libavformat/avio.h>

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
#include "libswscale/swscale.h"

}



class ScreenRecorder {

private:
    //video
    AVInputFormat *inVInputFormat;
    AVFormatContext *inVFormatContext;
    AVDictionary *inVOptions;
    AVCodecContext *inVCodecContext;
    const AVCodec *inVCodec;

    AVFormatContext *outAVFormatContext;
    AVDictionary *outVOptions;
    AVCodecContext *outVCodecContext;
    const AVCodec *outVCodec;

    //audio
    AVDictionary *inAOptions;
    AVFormatContext *inAFormatContext;
    AVInputFormat *inAInputFormat;
    AVCodecContext *inACodecContext;

    AVCodecContext *outACodecContext;
    const AVCodec *outACodec;
    const AVCodec *inACodec;

    //output
    AVOutputFormat *outAVOutputFormat;

    int VideoStreamIndex;
    int AudioStreamIndex;
    void generateVideoOutputStream(AVFormatContext *formatContext);
    void generateAudioOutputStream(AVFormatContext *formatContext);

public:
    ScreenRecorder();
    ~ScreenRecorder();

    int openVideoSource();
    int openAudioSource();
    int initOutputFile(char *filename, bool audio_recorded);
};



#endif //VIDEO_APP_SCREENRECORDER_H
