//
// Created by Andrea on 22/02/2022.
//

#ifndef VIDEO_APP_VIDEODEMUXER_H
#define VIDEO_APP_VIDEODEMUXER_H

#include "Demuxer.h"

class VideoDemuxer: public Demuxer {

private:
    uint16_t fps;
    SRResolution resolution;
    void setOptions() override;

public:

    VideoDemuxer(char *url, char *src, uint16_t fps, SRResolution resolution);
    AVFormatContext* open() override;

};


#endif //VIDEO_APP_VIDEODEMUXER_H
