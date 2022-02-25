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

    void generateVideoOutputStream();
    void generateAudioOutputStream(const AVCodecContext* inACodecContext);

public:
    int outVideoStreamIndex;
    int outAudioStreamIndex;

    /**
     * Create a new muxer
     * @param outputSettings
     */
    Muxer(SRSettings outputSettings);
    ~Muxer();

    /**
     * Open the output file and write the header
     */
    int initOutputFile(const AVCodecContext* inACodecContext);

    AVCodecContext* getACodecContext() const;
    AVCodecContext* getVCodecContext() const;
    AVFormatContext* getOutAVFormatContext() const;
};

#endif //VIDEO_APP_MUXER_H
