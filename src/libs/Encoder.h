#ifndef VIDEO_APP_ENCODER_H
#define VIDEO_APP_ENCODER_H

#include "utils.h"

class Encoder {
    AVCodecContext *outCodecContext;
public:

    Encoder():outCodecContext(nullptr){};

    /**
     * Send a frame to the encoder
     * @param frame the frame to send to the encoder
     * @return 0 if the decoder correctly received the frame
     * @throw runtime_error
     */
    int sendFrame(AVFrame* frame);

    /**
     * Read encoded data from the encoder
     * @return 0 if success
     * @throw runtime_error
     */
    int getPacket(AVPacket* packet);

    void setCodecContext(AVCodecContext* context);

    const AVCodecContext* getCodecContext() const;

};

#endif //VIDEO_APP_ENCODER_H
