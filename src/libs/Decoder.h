#ifndef VIDEO_APP_DECODER_H
#define VIDEO_APP_DECODER_H

#include "utils.h"

class Decoder {
    AVCodecContext *inCodecContext;
public:

    Decoder():inCodecContext(nullptr){};

    /**
     * Send a packet to the decoder
     * @param packet the packet to send
     * @return 0 if the decoder correctly received the packet
     * @throw runtime_error
     */
    int sendPacket(const AVPacket* packet);

    /**
     * Return decoded output data from a decoder
     * @return 0 if success
     * @throw runtime_error
     */
    int getDecodedOutput(AVFrame* rawFrame);

    const AVCodecContext* getCodecContext() const;

    void setCodecContext(AVCodecContext* context);
};

#endif //VIDEO_APP_DECODER_H
