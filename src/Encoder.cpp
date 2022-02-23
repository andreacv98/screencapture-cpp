#include "Encoder.h"
#include <stdexcept>

int Encoder::sendFrame(AVFrame *frame) {
    int ret = avcodec_send_frame(codecContext, frame);
    if (ret == AVERROR(EAGAIN)) return ret;
    if (ret == AVERROR_EOF) throw std::logic_error("Encoder: has already been flushed");
    if (ret < 0) throw std::runtime_error("Encoder: failed to send frame to encoder");
    return ret;
}

int Encoder::getPacket(AVPacket *packet) {
    if (!packet) throw std::runtime_error("Encoder: failed to allocate packet");

    int ret = avcodec_receive_packet(codecContext, packet);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) return ret;
    if (ret < 0) throw std::runtime_error("Encoder: failed to receive frame from decoder");

    return ret;
}

const AVCodecContext* Encoder::getCodecContext() const {
    return codecContext;
}

void Encoder::setCodecContext(AVCodecContext *context){
    this->codecContext = context;
}

