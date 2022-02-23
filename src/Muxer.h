//
// Created by pushz on 23/02/22.
//

#ifndef VIDEO_APP_MUXER_H
#define VIDEO_APP_MUXER_H
#include "utils.h"
#include <string>

class Muxer {

private:
    AVFormatContext *outAVFormatContext;

    AVCodecContext *outVCodecContext;
    AVCodecContext *outACodecContext;

    AVOutputFormat *outAVOutputFormat;

    SRSettings outputSettings;
    std::string outputFilename;

    void generateVideoOutputStream();
    void generateAudioOutputStream(const AVCodecContext* inACodecContext);

public:
    int outVideoStreamIndex;
    int outAudioStreamIndex;

    Muxer(SRSettings outputSettings, std::string outputFilename);
    ~Muxer();

    /**
     * Add a video stream to the muxer
     */

    int initOutputFile(const AVCodecContext* inACodecContext);

    AVCodecContext* getACodecContext() const;
    AVCodecContext* getVCodecContext() const;
};

#endif //VIDEO_APP_MUXER_H
