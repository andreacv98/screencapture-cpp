//
// Created by pushz on 22/02/22.
//

#ifndef VIDEO_APP_ENCODER_H
#define VIDEO_APP_ENCODER_H

#include "utils.h"

class Encoder {
    AVCodecContext *outCodecContext;
public:
    Encoder():outCodecContext(nullptr){};
    ~Encoder();

    /**
     * Send a frame to the encoder
     * @param frame the frame to send to the encoder
     * @return 0 if the decoder correctly received the frame
     */
    int sendFrame(AVFrame* frame);

    /**
     * Read encoded data from the encoder
     * @return 0 if success
     */
    int getPacket(AVPacket* packet);

    void setCodecContext(AVCodecContext* context);

    const AVCodecContext* getCodecContext() const;

};

#endif //VIDEO_APP_ENCODER_H
