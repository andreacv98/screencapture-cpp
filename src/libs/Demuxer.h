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

    Demuxer(char * src, char * url);
    ~Demuxer();
    virtual AVFormatContext* open() = 0;
    AVCodecContext *getInCodecContext() const;
    AVFormatContext *getInFormatContext() const;
    void closeInput();

private:

    virtual void setOptions() = 0;

};


#endif //VIDEO_APP_DEMUXER_H
