//
// Created by Andrea on 22/02/2022.
//

#include <iostream>
#include "VideoDemuxer.h"

VideoDemuxer::VideoDemuxer(char *url, char *src, int fps, int width, int height) : Demuxer(url, src), fps(fps), width(width),height(height) {
    setOptions();
}

/**
 * The method set the options for the video device.
 *
 * @return
 * @throw invalid_argument
 */
void VideoDemuxer::setOptions() {
    std::cout << "Video Input setup started" << std::endl;

#ifdef __APPLE__
    value = av_dict_set(&inVOptions, "pixel_format", "0rgb", 0);
    if (value < 0) {
        throw std::invalid_argument("Error in setting pixel format");
    }
    value = av_dict_set(&inVOptions, "video_device_index", "1", 0);

    if (value < 0) {
        throw std::invalid_argument("Error in setting video_device_index");
    }

#endif

    char s[30];
    sprintf(s,"%dx%d", width, height);

    char framerate[30];
    sprintf(s, "%d", fps);

    value = av_dict_set(&options, "framerate", framerate, 0);
    if (value < 0) {
        throw std::invalid_argument("Error in setting framerate");
        cout << "\nerror in setting dictionary value";
        exit(1);
    }

    value = av_dict_set(&options, "video_size", s, 0);

    if (value < 0) {
        throw std::invalid_argument("Error in setting video size");
    }

    value = av_dict_set(&options, "preset", "medium", 0);
    if (value < 0) {
        throw std::invalid_argument("Error in setting preset value");
    }

    value = av_dict_set(&options, "probesize", "60M", 0);
    if (value < 0) {
        throw std::invalid_argument("Error in set probesize value");
    }

    return;
}

/**
 * The method opens the device and defines the needed data structure.
 *
 * @return the device context
 * @throw runtime_error
 */
AVFormatContext *VideoDemuxer::open() {
    //if one of them != nullptr then input already initialized
    if(inFormatContext != nullptr || inCodecContext!= nullptr || streamIndex != -1)
        return inFormatContext;

    value = avformat_open_input(&inFormatContext, url, inFormat, &options);
    if (value != 0) {
        throw std::runtime_error("Cannot open selected device");
    }

    //get video stream infos from context
    value = avformat_find_stream_info(inFormatContext, nullptr);
    if (value < 0) {
        throw std::runtime_error("Cannot find the stream information");
    }

    //find the first video stream with a given code
    streamIndex = -1;
    for (int i = 0; i < inFormatContext->nb_streams; i++){
        if (inFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            streamIndex = i;
            break;
        }
    }

    if (streamIndex == -1) {
        throw std::runtime_error("Cannot find the video stream index. (-1)");
    }

    AVCodecParameters *params = inFormatContext->streams[streamIndex]->codecpar;
    inCodec = avcodec_find_decoder(params->codec_id);
    if (inCodec == nullptr) {
        throw std::runtime_error("Cannot find the decoder");
    }

    inCodecContext = avcodec_alloc_context3(inCodec);
    avcodec_parameters_to_context(inCodecContext, params);

    value = avcodec_open2(inCodecContext, inCodec, nullptr);
    if (value < 0) {
        throw std::runtime_error("Cannot open the av codec");
    }

    return inFormatContext;
}
