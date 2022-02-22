//
// Created by Andrea on 22/02/2022.
//

#ifndef VIDEO_APP_DEMUXER_H
#define VIDEO_APP_DEMUXER_H

#include "utils.h"
#include <stdexcept>


class Demuxer {

protected:
    char * url;
    char * src;
    int value;
    int streamIndex;
    AVDictionary * options;
    AVFormatContext * inFormatContext;
    AVInputFormat * inFormat;
    AVCodecContext * inCodecContext;
    AVCodec * inCodec;

public:

    Demuxer(char * url, char * src);
    ~Demuxer();
    virtual AVFormatContext* open() = 0;
    AVCodecContext *getInCodecContext() const;
    int readPacket(AVPacket* read_packet, long long int pts_offset);

private:

    virtual void setOptions() = 0;

};


#endif //VIDEO_APP_DEMUXER_H
