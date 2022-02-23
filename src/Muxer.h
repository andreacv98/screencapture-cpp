//
// Created by pushz on 23/02/22.
//

#ifndef VIDEO_APP_MUXER_H
#define VIDEO_APP_MUXER_H
#include "utils.h"
#include <string>

class Muxer {

    AVFormatContext *outAVFormatContext;
    AVCodecContext *outVCodecContext;
    AVCodecContext *outACodecContext;
    AVOutputFormat *outAVOutputFormat;

    int outVideoStreamIndex;
    int outAudioStreamIndex;

    SRSettings outputSettings;
    std::string outputFilename;

public:
    Muxer(SRSettings outputSettings, std::string outputFilename);
    ~Muxer();

    /**
     * Add a video stream to the muxer
     */
    void generateVideoOutputStream();
    void generateAudioOutputStream(AVCodecContext* inACodecContext);
    int initOutputFile(AVCodecContext* inACodecContext);
};

#endif //VIDEO_APP_MUXER_H
