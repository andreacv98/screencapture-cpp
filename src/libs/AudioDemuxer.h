#ifndef VIDEO_APP_AUDIODEMUXER_H
#define VIDEO_APP_AUDIODEMUXER_H


#include "Demuxer.h"

class AudioDemuxer: public Demuxer {


private:
    void setOptions() override;

public:

    AudioDemuxer(const char *src, char *url);
    AVFormatContext* open() override;

};


#endif //VIDEO_APP_AUDIODEMUXER_H
