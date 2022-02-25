#include "Encoder.h"
#include <stdexcept>
#include <iostream>

Encoder::~Encoder() {
    /*avcodec_free_context(&outCodecContext);
    if (outCodecContext) {
        std::cerr << "Encoder: unable to free codec context" << std::endl;
        exit(1);
    }else {
        std::cout << "\n[Encoder] codec context freed";
    }*/
}

int Encoder::sendFrame(AVFrame *frame) {
    int ret = avcodec_send_frame(outCodecContext, frame);
    if (ret == AVERROR(EAGAIN)) return ret;
    if (ret == AVERROR_EOF) throw std::logic_error("Encoder: has already been flushed");
    if (ret < 0) throw std::runtime_error("Encoder: failed to send frame to encoder");
    return ret;
}

int Encoder::getPacket(AVPacket *packet) {
    if (!packet) throw std::runtime_error("Encoder: failed to allocate packet");

    int ret = avcodec_receive_packet(outCodecContext, packet);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) return ret;
    if (ret < 0) throw std::runtime_error("Encoder: failed to receive frame from decoder");

    return ret;
}

const AVCodecContext* Encoder::getCodecContext() const {
    return outCodecContext;
}

void Encoder::setCodecContext(AVCodecContext *context){
    this->outCodecContext = context;
}

