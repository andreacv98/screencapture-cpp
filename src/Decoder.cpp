#include "Decoder.h"

bool Decoder::sendPacket(const AVPacket *packet) {

}

int Decoder::getDecodedOutput(AVFrame* rawFrame) {
    int ret = avcodec_receive_frame(this->codecContext, rawFrame);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        return ret;
    else if (ret < 0) {
        fprintf(stderr, "Error during decoding\n");
        exit(1);
    }
    return ret;
}

const AVCodecContext *Decoder::getCodecContext() const {
    return codecContext
}

void Decoder::setCodecContext(AVCodecContext *context){
    this->codecContext = context;
}

