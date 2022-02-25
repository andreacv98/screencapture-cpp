//
// Created by Andrea on 22/02/2022.
//

#include <iostream>
#include "VideoDemuxer.h"

VideoDemuxer::VideoDemuxer(const char *src, char *url, uint16_t fps, SRResolution resolution) : Demuxer(src, url), fps(fps),
                                                                                                resolution(resolution) {
}

/**
 * The method set the options for the video device.
 *
 * @return
 * @throw invalid_argument
 */
void VideoDemuxer::setOptions() {
    std::cout << "\nVideo Input Options setup started" << std::endl;

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



    char framerate[30];
    sprintf(framerate, "%d", fps);

    value = av_dict_set(&options, "framerate", framerate, 0);
    if (value < 0) {
        throw std::invalid_argument("Error in setting framerate");
    }

    char s[30];
    sprintf(s,"%dx%d", resolution.width, resolution.height);

    value = av_dict_set(&options, "video_size", s, 0);

    if (value < 0) {
        throw std::invalid_argument("Error in setting video size");
    }

#ifdef _WIN32
    char off_x[30];
    sprintf(off_x,"%d", settings._screenoffset.x);
    char off_y[30];
    sprintf(off_y,"%d", settings._screenoffset.y);


    value = av_dict_set(&inVOptions, "offset_x", off_x, 0);
    if (value < 0) {
        cout << "\nerror in setting dictionary value off_x";
        exit(1);
    }

    value = av_dict_set(&inVOptions, "offset_y", off_y, 0);
    if (value < 0) {
        cout << "\nerror in setting dictionary value off_y";
        exit(1);
    }
#endif

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

    inFormatContext = avformat_alloc_context();
    setOptions();
    inFormat = av_find_input_format(src);
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

    if (inCodec == nullptr) {
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
    }



    return inFormatContext;
}
