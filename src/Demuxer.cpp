//
// Created by Andrea on 22/02/2022.
//

#include <iostream>
#include "Demuxer.h"

Demuxer::Demuxer(char *url, char *src) : value(0), streamIndex(0), options(nullptr)  {
    inFormatContext = avformat_alloc_context();
    inFormat = av_find_input_format(src);
}

AVCodecContext *Demuxer::getInCodecContext() const {
    return inCodecContext;
}

Demuxer::~Demuxer() {
    if (inFormatContext) {
        avformat_close_input(&inFormatContext);
        if (inFormatContext) {
            std::cerr << "\n Unable to close device";
            exit(1);
        }
    }
    if (inCodecContext) {
        avcodec_free_context(&inCodecContext);
        if (inCodecContext) {
            std::cerr << "\nUnable to close device";
            exit(1);
        }
    }
}


int Demuxer::readPacket(AVPacket* read_packet, long long int pts_offset) {
    //TODO
}
