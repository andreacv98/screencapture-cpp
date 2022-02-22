#ifndef VIDEO_APP_DECODER_H
#define VIDEO_APP_DECODER_H

#include "utils.h"

class Decoder {
    AVCodecContext *codecContext;
public:

    Decoder():codecContext(nullptr){};

    /**
     * Send a packet to the decoder
     * @param packet the packet to send
     * @return true if the decoder correctly received the packet, false otherwise
     */
    bool sendPacket(const AVPacket* packet);

    /**
     * Return decoded output data from a decoder
     * @return 0 if success
     */
    int getDecodedOutput(AVFrame* rawFrame);

    /**
     *
     * @return AVCodecContext
     */
    const AVCodecContext* getCodecContext() const;

    void setCodecContext(AVCodecContext* context);
};

#endif //VIDEO_APP_DECODER_H
