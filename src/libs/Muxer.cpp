#include "Muxer.h"
#include <stdexcept>
#include <iostream>

Muxer::Muxer(SRSettings outputSettings) {
    this->outputSettings = outputSettings;
    outAVFormatContext = nullptr;
    outVCodecContext = nullptr;
    outACodecContext = nullptr;
    outAVOutputFormat = nullptr;
    outVideoStreamIndex = -1;
    outAudioStreamIndex = -1;
}

Muxer::~Muxer() {
    if(outAVFormatContext){
        if( av_write_trailer(outAVFormatContext) < 0) {
            std::cerr << "Muxer: error in writing av trailer" << std::endl;
            exit(1);
        } else {
            std::cout << "\n[Muxer] av trailer closed";
        }

        avformat_close_input(&outAVFormatContext);
        if (outAVFormatContext){
            std::cerr << "Muxer: unable to free audio avformat context" << std::endl;
            exit(1);
        } else {
            std::cout << "\n[Muxer] av audio avformat closed";
        }
    }
}

int Muxer::initOutputFile() {

    outAVFormatContext = nullptr;
    int value = 0;

    /*get the filetype from filename extension*/
    outAVOutputFormat = av_guess_format(nullptr,outputSettings.filename, nullptr);
    if(!outAVOutputFormat) throw std::runtime_error("Muxer: cannot guess the video format");

    /*allocate the format context*/
    avformat_alloc_output_context2(&outAVFormatContext, outAVOutputFormat, outAVOutputFormat->name, outputSettings.filename);
    if (!outAVFormatContext) throw std::runtime_error("Muxer: cannot allocate the output context");

    if(outputSettings._recvideo) generateVideoOutputStream();
    if(outputSettings._recaudio) generateAudioOutputStream();

    /* create empty video file */
    if (!(outAVFormatContext->flags & AVFMT_NOFILE)) {
        value = avio_open2(&outAVFormatContext->pb, outputSettings.filename, AVIO_FLAG_WRITE, nullptr, nullptr);
        if (value < 0) throw std::runtime_error("Muxer: error in creating the output video file");
    }

    if (!outAVFormatContext->nb_streams) throw std::runtime_error("Muxer: output file doesn't contain any stream");


    /* imp: mp4 container or some advanced container file required header information*/
    value = avformat_write_header(outAVFormatContext, nullptr);
    if (value < 0) throw std::runtime_error("error in writing the header context");

    return 0;
}

void Muxer::generateAudioOutputStream() {
    outACodecContext = nullptr;
    AVCodec* outACodec = nullptr;
    int i;

    AVStream *audio_st = avformat_new_stream(outAVFormatContext, nullptr);
    if (!audio_st) throw std::runtime_error("Muxer: cannot create audio stream");
    outACodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!outACodec) throw std::runtime_error("Muxer: cannot find requested encoder");
    outACodecContext = avcodec_alloc_context3(outACodec);
    if (!outACodecContext) throw std::runtime_error("Muxer: cannot create related VideoCodecContext");


    /* set properties for the video stream encoding*/
    if ((outACodec)->supported_samplerates) {
        outACodecContext->sample_rate = (outACodec)->supported_samplerates[0];
        for (i = 0; (outACodec)->supported_samplerates[i]; i++) {
            if ((outACodec)->supported_samplerates[i] == outputSettings.audio_sample_rate)
                outACodecContext->sample_rate = outputSettings.audio_sample_rate;
        }
    }
    outACodecContext->codec_id = AV_CODEC_ID_AAC;
    outACodecContext->sample_fmt  = (outACodec)->sample_fmts ? (outACodec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    outACodecContext->channels  = outputSettings.audio_channels;
    outACodecContext->channel_layout = av_get_default_channel_layout(outACodecContext->channels);
    outACodecContext->bit_rate = 96000;
    outACodecContext->time_base = { 1, outputSettings.audio_sample_rate };

    outACodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    if ((outAVFormatContext)->oformat->flags & AVFMT_GLOBALHEADER) {
        outACodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(outACodecContext, outACodec, nullptr)< 0) throw std::runtime_error("Muxer: error in opening the avcodec");

    //find a free stream index
    outAudioStreamIndex = -1;
    for(i=0; i < outAVFormatContext->nb_streams; i++)
        if(outAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_UNKNOWN)
            outAudioStreamIndex = i;

    if(outAudioStreamIndex < 0) throw std::runtime_error("Muxer: cannot find a free stream for audio on the output");

    avcodec_parameters_from_context(outAVFormatContext->streams[outAudioStreamIndex]->codecpar, outACodecContext);

}

void Muxer::generateVideoOutputStream() {
    int i;
    AVStream *video_st = avformat_new_stream(outAVFormatContext, nullptr);

    if (!video_st) throw std::runtime_error("Muxer: failed to create a new stream");
    AVCodec* outVCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!outVCodec) throw std::runtime_error("Muxer: cannot find requested encoder");
    outVCodecContext = avcodec_alloc_context3(outVCodec);
    if (!outVCodecContext) throw std::runtime_error("Muxer: cannot create VideoCodecContext");

    /* set properties for the video stream encoding */
    outVCodecContext->codec_id = AV_CODEC_ID_H264;// AV_CODEC_ID_MPEG4; // AV_CODEC_ID_H264 // AV_CODEC_ID_MPEG1VIDEO
    outVCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    outVCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    outVCodecContext->bit_rate = 2500000; // 400000
    outVCodecContext->width = outputSettings._outscreenres.width;
    outVCodecContext->height = outputSettings._outscreenres.height;
    outVCodecContext->gop_size = 3;
    outVCodecContext->max_b_frames = 2;
    outVCodecContext->time_base.num = 1;
    outVCodecContext->time_base.den = outputSettings._fps; // 15fps
    outVCodecContext->compression_level = 1;
    /* reduce preset to slow if H264 to avoid resources leak */
    if(outVCodecContext->codec_id == AV_CODEC_ID_H264)
        av_opt_set(outVCodecContext->priv_data, "preset", "slow", 0);

    /*setting global headers because some formats require them*/
    if (outAVFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        outVCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(outVCodecContext, outVCodec, nullptr)< 0) throw std::runtime_error("Muxer: error in opening the avcodec");

    //find a free stream index
    outVideoStreamIndex = -1;
    for(i=0; i < outAVFormatContext->nb_streams; i++)
        if(outAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_UNKNOWN)
            outVideoStreamIndex = i;

    if(outVideoStreamIndex < 0) throw std::runtime_error("Muxer: cannot find a free stream for video on the output");

    avcodec_parameters_from_context(outAVFormatContext->streams[outVideoStreamIndex]->codecpar, outVCodecContext);
}

AVCodecContext *Muxer::getVCodecContext() const {
    return outVCodecContext;
}

AVCodecContext *Muxer::getACodecContext() const {
    return outACodecContext;
}

AVFormatContext *Muxer::getOutAVFormatContext() const {
    return outAVFormatContext;
}

