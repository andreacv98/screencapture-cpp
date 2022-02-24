//
// Created by pushz on 22/02/22.
//

#ifndef VIDEO_APP_UTILS_H
#define VIDEO_APP_UTILS_H

//FFMPEG LIBRARIES
#include <queue>

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

#endif //VIDEO_APP_UTILS_H
