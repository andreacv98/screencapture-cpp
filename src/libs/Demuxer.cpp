//
// Created by Andrea on 22/02/2022.
//

#include <iostream>
#include "Demuxer.h"

Demuxer::Demuxer(const char *src, char *url) :
                                        src(const_cast<char *>(src)),
                                        url(url),
                                        value(0),
                                        streamIndex(-1),
                                        options(nullptr),
                                        inFormatContext(nullptr),
                                        inCodecContext(nullptr),
                                        inCodec(nullptr)
{

}

AVCodecContext *Demuxer::getInCodecContext() const {
    return inCodecContext;
}

Demuxer::~Demuxer() {
    avformat_close_input(&inFormatContext);
    if (inFormatContext) {
        std::cerr << "\n Unable to close device";
        exit(1);
    } else {
        std::cout << "\n ["<< src << "]Device closed";
    }
    if(inFormatContext) {
        avformat_free_context(inFormatContext);
        if (inFormatContext) {
            std::cerr << "\n Unable to free avformat context";
            exit(1);
        } else {
            std::cout << "\n ["<< src << "]AvformatContext closed";
        }
    }
}

/**
 * The method close the format context.
 *
 * @return
 * @throw runtime_error
 */
void Demuxer::closeInput() {
    avformat_close_input(&inFormatContext);
    if (inFormatContext) {
        throw std::runtime_error("Unable to close the file");
    }
}

AVFormatContext *Demuxer::getInFormatContext() const {
    return inFormatContext;
}
