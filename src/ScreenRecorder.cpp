//
// Created by andrea on 19/12/21.
//

#include "ScreenRecorder.h"

#include "ScreenRecorder.h"

using namespace std;




ScreenRecorder::ScreenRecorder() {
    avdevice_register_all();
    cout << "\nScreen Recorder initialized correctly";
}
ScreenRecorder::~ScreenRecorder() {
    avformat_close_input(&inVFormatContext);
    if (!inVFormatContext) {
        cout << "\nfile closed sucessfully";
    } else {
        cout << "\nunable to close the file";
        exit(1);
    }

    avformat_free_context(inVFormatContext);
    if (!inVFormatContext) {
        cout << "\navformat free successfully";
    } else {
        cout << "\nunable to free avformat context";
        exit(1);
    }}


int ScreenRecorder::openVideoSource() {
    int value = 0;
    inVOptions = nullptr;
    inVFormatContext = avformat_alloc_context();

    /*Defining options for the device initialization*/
    char s[20];
    sprintf(s, "%dx%d", 1920, 1080);
    value = av_dict_set(&inVOptions, "video_size", s, 0);
    if (value < 0) {
        cout << "\nerror in setting dictionary value";
        exit(1);
    }
    value = av_dict_set(&inVOptions, "framerate", "30", 0);
    if (value < 0) {
        cout << "\nerror in setting dictionary value";
        exit(1);
    }
    value = av_dict_set(&inVOptions, "preset", "medium", 0);
    if (value < 0) {
        cout << "\nerror in setting preset values";
        exit(1);
    }

    value = av_dict_set(&inVOptions, "probesize", "17M", 0);
    if (value < 0) {
        cout << "\nerror in setting preset values";
        exit(1);
    }

    //get input format
    inVInputFormat = av_find_input_format("x11grab");
    sprintf(s, ":0.0+%d,%d", 0,0);
    value = avformat_open_input(&inVFormatContext, s, inVInputFormat, &inVOptions);
    if (value != 0) {
        cout << "\nCannot open selected device";
        exit(1);
    }



    //get video stream infos from context
    value = avformat_find_stream_info(inVFormatContext, nullptr);
    if (value < 0) {
        cout << "\nCannot find the stream information";
        exit(1);
    }

    //find the first video stream with a given code
    VideoStreamIndex = -1;
    for (int i = 0; i < inVFormatContext->nb_streams; i++){
        if (inVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            VideoStreamIndex = i;
            break;
        }
    }

    if (VideoStreamIndex == -1) {
        cout << "\nCannot find the video stream index. (-1)";
        exit(1);
    }

    AVCodecParameters *params = inVFormatContext->streams[VideoStreamIndex]->codecpar;
    inVCodec = avcodec_find_decoder(params->codec_id);
    if (inVCodec == nullptr) {
        cout << "\nCannot find the decoder";
        exit(1);
    }

    inVCodecContext = avcodec_alloc_context3(inVCodec);
    avcodec_parameters_to_context(inVCodecContext, params);

    value = avcodec_open2(inVCodecContext, inVCodec, nullptr);
    if (value < 0) {
        cout << "\nCannot open the av codec";
        exit(1);
    }

    return 0;
}
int ScreenRecorder::openAudioSource() {
    int value = 0;
    inAOptions = nullptr;
    inAFormatContext = avformat_alloc_context();

    value = av_dict_set(&inAOptions, "sample_rate", "44100", 0);
    if(value < 0) {
        cerr << "Cannot set sample rate option on input audio stream";
        exit(1);
    }


    inAInputFormat = av_find_input_format("pulse");
    value = avformat_open_input(&inAFormatContext, "default", inAInputFormat, &inAOptions);
    if (value != 0) {
        cout << "\nCannot open selected device";
        exit(1);
    }

    value = avformat_find_stream_info(inAFormatContext, nullptr);
    if (value < 0) {
        cout << "\nCannot find the audio stream information";
        exit(1);
    }

    //find the first video stream with a given code
    AudioStreamIndex = -1;
    for (int i = 0; i < inAFormatContext->nb_streams; i++){
        if (inAFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            AudioStreamIndex = i;
            break;
        }
    }

    if (AudioStreamIndex == -1) {
        cout << "\nCannot find the audio stream index. (-1)";
        exit(1);
    }

    AVCodecParameters *params = inAFormatContext->streams[AudioStreamIndex]->codecpar;
    inACodec = avcodec_find_decoder(params->codec_id);
    if (inACodec == nullptr) {
        cout << "\nCannot find the audio decoder";
        exit(1);
    }
    cout << "Input audio codec:" << inACodec->name;

    inACodecContext = avcodec_alloc_context3(inACodec);
    if(avcodec_parameters_to_context(inACodecContext, params)<0)
        cout<<"Cannot create codec context for audio input";



    value = avcodec_open2(inACodecContext, inACodec, nullptr);
    if (value < 0) {
        cout << "\nCannot open the input audio codec";
        exit(1);
    }
    return 0;
}

int ScreenRecorder::initOutputFile(char *filename, bool audio_recorded){
    outAVFormatContext = nullptr;
    int value = 0;

    /*get the filetype from filename extension*/
    outAVOutputFormat = av_guess_format(nullptr,filename, nullptr);
    if(!outAVOutputFormat) {
        cout << "\nCannot get the video format. try with correct format";
        exit(1);
    }

    /*allocate the format context*/
    avformat_alloc_output_context2(&outAVFormatContext, outAVOutputFormat, outAVOutputFormat->name, filename);
    if (!outAVFormatContext) {
        cout << "\nCannot allocate the output context";
        exit(1);
    }

    generateVideoOutputStream(outAVFormatContext);
    if(audio_recorded) generateAudioOutputStream(outAVFormatContext);

    /* create empty video file */
    if (!(outAVFormatContext->flags & AVFMT_NOFILE)) {
        value = avio_open2(&outAVFormatContext->pb, filename, AVIO_FLAG_WRITE, nullptr, nullptr);
        if (value < 0) {
            cout << "\nerror in creating the video file";
            exit(1);
        }
    }

    if (!outAVFormatContext->nb_streams) {
        cout << "\noutput file dose not contain any stream";
        exit(1);
    }
    cout << endl << outAVFormatContext->nb_streams << " stream(s) correctly initialized";

    /* imp: mp4 container or some advanced container file required header information*/
    value = avformat_write_header(outAVFormatContext, &inVOptions);
    if (value < 0) {
        cout << "\nerror in writing the header context";
        exit(1);
    }


    return 0;
}

void ScreenRecorder::generateVideoOutputStream(AVFormatContext *formatContext){
    AVStream *video_st = avformat_new_stream(formatContext, nullptr);
    if (!video_st) {
        cout << "\nCannot create video stream";
        exit(1);
    }
    outVCodec = avcodec_find_encoder(AV_CODEC_ID_MPEG4);
    if (!outVCodec) {
        cout << "\nCannot find requested encoder";
        exit(1);
    }
    outVCodecContext = avcodec_alloc_context3(outVCodec);
    if (!outVCodecContext) {
        cout << "\nCannot create related VideoCodecContext";
        exit(1);
    }

    /* set properties for the video stream encoding */
    outVCodecContext->codec_id = AV_CODEC_ID_MPEG4;// AV_CODEC_ID_MPEG4; // AV_CODEC_ID_H264 // AV_CODEC_ID_MPEG1VIDEO
    outVCodecContext->codec_type = AVMEDIA_TYPE_VIDEO;
    outVCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;
    outVCodecContext->bit_rate = 400000; // 2500000
    outVCodecContext->width = 1920;
    outVCodecContext->height = 1080;
    outVCodecContext->gop_size = 3;
    outVCodecContext->max_b_frames = 2;
    outVCodecContext->time_base.num = 1;
    outVCodecContext->time_base.den = 30; // 15fps

    /* reduce preset to slow if H264 to avoid resources leak */
    if(outVCodecContext->codec_id == AV_CODEC_ID_H264)
        av_opt_set(outVCodecContext->priv_data, "preset", "slow", 0);

    /*setting global headers because some formats require them*/
    if (formatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        outVCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(outVCodecContext, outVCodec, nullptr)< 0) {
        cout << "\nerror in opening the avcodec";
        exit(1);
    }

    avcodec_parameters_from_context(formatContext->streams[video_st->id]->codecpar, outVCodecContext);
}

void ScreenRecorder::generateAudioOutputStream(AVFormatContext *formatContext){
    outACodecContext = nullptr;
    outACodec = nullptr;
    int i;

    AVStream *audio_st = avformat_new_stream(formatContext, nullptr);
    if (!audio_st) {
        cout << "\nCannot create audio stream";
        exit(1);
    }
    outACodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if (!outACodec) {
        cout << "\nCannot find requested encoder";
        exit(1);
    }
    outACodecContext = avcodec_alloc_context3(outACodec);
    if (!outACodecContext) {
        cout << "\nCannot create related VideoCodecContext";
        exit(1);
    }


    /* set properties for the video stream encoding*/
    outACodecContext->sample_fmt  = (outACodec)->sample_fmts ? (outACodec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    outACodecContext->bit_rate    = 64000;
    if ((outACodec)->supported_samplerates) {
        outACodecContext->sample_rate = (outACodec)->supported_samplerates[0];
        for (i = 0; (outACodec)->supported_samplerates[i]; i++) {
            if ((outACodec)->supported_samplerates[i] == 44100)
                outACodecContext->sample_rate = 44100;
        }
    }
    outACodecContext->channels = av_get_channel_layout_nb_channels(outACodecContext->channel_layout);
    outACodecContext->channel_layout = AV_CH_LAYOUT_STEREO;
    if ((outACodec)->channel_layouts) {
        outACodecContext->channel_layout = (outACodec)->channel_layouts[0];
        for (i = 0; (outACodec)->channel_layouts[i]; i++) {
            if ((outACodec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                outACodecContext->channel_layout = AV_CH_LAYOUT_STEREO;
        }
    }
    outACodecContext->channels        = av_get_channel_layout_nb_channels(outACodecContext->channel_layout);
    outACodecContext->time_base = (AVRational){ 1, outACodecContext->sample_rate };

    if (outAVFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        outACodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    int err;
    if ((err = avcodec_open2(outACodecContext, outACodec, nullptr))< 0) {
        cout << "\nerror in opening the avcodec with error: "<< err;
        exit(1);
    }

    // avcodec_parameters_from_context(formatContext->streams[audio_st->id]->codecpar, outACodecContext);
}