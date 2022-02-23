//
// Created by Andrea on 22/02/2022.
//

#ifndef VIDEO_APP_AUDIODEMUXER_H
#define VIDEO_APP_AUDIODEMUXER_H


#include "Demuxer.h"

class AudioDemuxer: public Demuxer {


private:
    int fps;
    int width;
    int height;
    void setOptions() override;

public:

    AudioDemuxer(char *src, char *url);
    AVFormatContext* open() override;

};


#endif //VIDEO_APP_AUDIODEMUXER_H
