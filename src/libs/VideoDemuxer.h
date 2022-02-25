#ifndef VIDEO_APP_VIDEODEMUXER_H
#define VIDEO_APP_VIDEODEMUXER_H

#include "Demuxer.h"

class VideoDemuxer: public Demuxer {

private:


    uint16_t fps;
    SRResolution resolution;
    SROffset offset;
    void setOptions() override;

public:

    VideoDemuxer(const char *src, char *url, uint16_t fps, SRResolution resolution, SROffset offset);
    AVFormatContext* open() override;


};


#endif //VIDEO_APP_VIDEODEMUXER_H
