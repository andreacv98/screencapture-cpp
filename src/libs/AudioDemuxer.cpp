#include <iostream>
#include "AudioDemuxer.h"

/**
 * The method opens the device and defines the needed data structure.
 *
 * @return the device context
 * @throw runtime_error
 */
AVFormatContext *AudioDemuxer::open() {

    inFormatContext = avformat_alloc_context();
    setOptions();
    inFormat = av_find_input_format(src);
    value = avformat_open_input(&inFormatContext, url, inFormat, &options);
    if (value != 0) {
        throw std::runtime_error("Cannot open selected device");
    }


    value = avformat_find_stream_info(inFormatContext, nullptr);
    if (value < 0) {
        throw std::runtime_error("Cannot find the stream information");
    }

    //find the first audio stream with a given code
    streamIndex = -1;
    for (int i = 0; i < inFormatContext->nb_streams; i++){
        if (inFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            streamIndex = i;
            break;
        }
    }

    if (streamIndex == -1) {
        throw std::runtime_error("Cannot find the audio stream index. (-1)");
    }

    if (inCodec == nullptr) {
        AVCodecParameters *params = inFormatContext->streams[streamIndex]->codecpar;
        inCodec = avcodec_find_decoder(params->codec_id);
        if (inCodec == nullptr) {
            throw std::runtime_error("Cannot find the audio decoder");
        }
        std::cout << "Input audio codec:" << inCodec->name;

        inCodecContext = avcodec_alloc_context3(inCodec);

        if(avcodec_parameters_to_context(inCodecContext, params)<0)
            throw std::runtime_error("Cannot create codec context for audio input");


        value = avcodec_open2(inCodecContext, inCodec, nullptr);
        if (value < 0) {
            throw std::runtime_error("Cannot open the input audio codec");
        }
    }

    return inFormatContext;
}

AudioDemuxer::AudioDemuxer(const char *src, char *url) : Demuxer(src, url) {

}

void AudioDemuxer::setOptions() {

}
