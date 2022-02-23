//
// Created by Andrea on 23/02/2022.
//

#include <iostream>
#include "Controller.h"

using namespace std;

Controller::Controller(char * audioUrl, char * videoUrl, SRSettings settings): settings(settings),
                                                                                captureSwitch(false),
                                                                                killSwitch(false),
                                                                                captureStarted(false),
                                                                                rawVideoFrame(nullptr),
                                                                                rawAudioFrame(nullptr),
                                                                                inVideo(VideoDemuxer(videoUrl, VIDEO_SOURCE, settings._fps, settings._inscreenres)),
                                                                                inAudio(AudioDemuxer(audioUrl, AUDIO_SOURCE))
                                                                                {
    inVideoBuffer.np = 0;
    inAudioBuffer.np = 0;

    try {
        inVFormatContext = inVideo.open();
    } catch (const std::runtime_error& e) {
        cerr << "Error opening video input: " << e.what() << endl;
        throw;
    }

    try {
        inAFormatContext = inAudio.open();
    } catch (const std::runtime_error& e) {
        cerr << "Error opening audio input: " << e.what() << endl;
        throw;
    }


    avdevice_register_all();
    cout << "\nScreen Recorder initialized correctly\n";

#ifdef __unix__
    dpy = XOpenDisplay (NULL);          //open connection to the default X server
    if (!dpy) {
        fprintf (stderr, "unable to open display \"%s\".\n",
                 XDisplayName (NULL));
        exit (1);
    }

    printf ("\nname of display:    %s\n", DisplayString (dpy));
    printf ("default screen number:    %d\n", DefaultScreen (dpy));
    printf ("number of screens:    %d\n", ScreenCount (dpy));
#endif


}

Controller::~Controller() {

    if(settings._recvideo && videoThread.joinable()) videoThread.join();
    if(settings._recaudio && audioThread.joinable()) audioThread.join();
    //producerThread.join();

    if( av_write_trailer(outAVFormatContext) < 0)
    {
        cout<<"\nerror in writing av trailer";
        exit(1);
    }
    avformat_close_input(&inVFormatContext);
    if (!inVFormatContext) {
        cout << "\nfile closed sucessfully";
    } else {
        cout << "\nunable to close the file";
        exit(1);
    }
    avformat_close_input(&inAFormatContext);
    if (!inAFormatContext) {
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
    }
    avformat_free_context(inAFormatContext);
    if (!inAFormatContext) {
        cout << "\navformat free successfully";
    } else {
        cout << "\nunable to free avformat context";
        exit(1);
    }
    avformat_free_context(outAVFormatContext);
    if (!outAVFormatContext) {
        cout << "\navformat free successfully";
    } else {
        cout << "\nunable to free avformat context";
        exit(1);
    }
}


int Controller::initOutputFile(){
    char* filename = settings.filename;
    bool audio_recorded = settings._recaudio;

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

    if(settings._recvideo)generateVideoOutputStream();
    if(audio_recorded) generateAudioOutputStream();

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


    /* imp: mp4 container or some advanced container file required header information*/
    value = avformat_write_header(outAVFormatContext, nullptr);
    if (value < 0) {
        cout << "\nerror in writing the header context";
        exit(1);
    }


    return 0;
}
void Controller::generateVideoOutputStream(){
    int i;
    AVStream *video_st = avformat_new_stream(outAVFormatContext, nullptr);

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
    outVCodecContext->width = settings._outscreenres.width;
    outVCodecContext->height = settings._outscreenres.height;
    outVCodecContext->gop_size = 3;
    outVCodecContext->max_b_frames = 2;
    outVCodecContext->time_base.num = 1;
    outVCodecContext->time_base.den = settings._fps; // 15fps
    outVCodecContext->compression_level = 1;
    /* reduce preset to slow if H264 to avoid resources leak */
    if(outVCodecContext->codec_id == AV_CODEC_ID_H264)
        av_opt_set(outVCodecContext->priv_data, "preset", "slow", 0);

    /*setting global headers because some formats require them*/
    if (outAVFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        outVCodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(outVCodecContext, outVCodec, nullptr)< 0) {
        cout << "\nerror in opening the avcodec";
        exit(1);
    }

    //find a free stream index
    outVideoStreamIndex = -1;
    for(i=0; i < outAVFormatContext->nb_streams; i++)
        if(outAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_UNKNOWN)
            outVideoStreamIndex = i;

    if(outVideoStreamIndex < 0) {
        cout << "\nCannot find a free stream for video on the output";
        exit(1);
    }

    avcodec_parameters_from_context(outAVFormatContext->streams[outVideoStreamIndex]->codecpar, outVCodecContext);
}
void Controller::generateAudioOutputStream(){
    outACodecContext = nullptr;
    outACodec = nullptr;
    int i;

    AVStream *audio_st = avformat_new_stream(outAVFormatContext, nullptr);
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

    if ((outACodec)->supported_samplerates) {
        outACodecContext->sample_rate = (outACodec)->supported_samplerates[0];
        for (i = 0; (outACodec)->supported_samplerates[i]; i++) {
            if ((outACodec)->supported_samplerates[i] == inACodecContext->sample_rate)
                outACodecContext->sample_rate = inACodecContext->sample_rate;
        }
    }
    outACodecContext->codec_id = AV_CODEC_ID_AAC;
    outACodecContext->sample_fmt  = (outACodec)->sample_fmts ? (outACodec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
    outACodecContext->channels  = inACodecContext->channels;
    outACodecContext->channel_layout = av_get_default_channel_layout(outACodecContext->channels);
    outACodecContext->bit_rate = 96000;
    outACodecContext->time_base = { 1, inACodecContext->sample_rate };

    outACodecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    if ((outAVFormatContext)->oformat->flags & AVFMT_GLOBALHEADER) {
        outACodecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    if (avcodec_open2(outACodecContext, outACodec, nullptr)< 0) {
        cout << "\nerror in opening the avcodec with error: ";
        exit(1);
    }


    //find a free stream index
    outAudioStreamIndex = -1;
    for(i=0; i < outAVFormatContext->nb_streams; i++)
        if(outAVFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_UNKNOWN)
            outAudioStreamIndex = i;

    if(outAudioStreamIndex < 0) {
        cout << "\nCannot find a free stream for audio on the output";
        exit(1);
    }

    avcodec_parameters_from_context(outAVFormatContext->streams[outAudioStreamIndex]->codecpar, outACodecContext);

}

/**
 * captureVideo() is the "VideoThread" execution flow.
 * This execution flow get packets from video input device
 * and decode them by sending them to the decoder
 *
 * @Note captureVideo() is a thread-safe execution flow, has to be passed to a specific thread to ensure the correct execution
 */
void Controller::captureVideo(){
    int ret;
    AVPacket *inPacket, *outPacket;
    AVFrame *rawFrame, *scaledFrame;

    //allocate space for a packet
    inPacket = (AVPacket *) av_malloc(sizeof (AVPacket));
    if(!inPacket) {
        cout << "\nCannot allocate an AVPacket for encoded video";
        exit(1);
    }
    av_init_packet(inPacket);

    //allocate space for a frame
    rawFrame =av_frame_alloc();
    if(!rawFrame) {
        cout << "\nCannot allocate an AVFrame for encoded video";
        exit(1);
    }

    outPacket = (AVPacket *) av_malloc(sizeof (AVPacket));
    if(!outPacket) {
        cout << "\nCannot allocate an AVPacket for encoded video";
        exit(1);
    }

    //allocate space for a packet
    scaledFrame = av_frame_alloc();
    if(!scaledFrame) {
        cout << "\nCannot allocate an AVFrame for encoded video";
        exit(1);
    }

    int video_outbuf_size;
    int nbytes = av_image_get_buffer_size(outVCodecContext->pix_fmt,outVCodecContext->width,outVCodecContext->height,32);
    uint8_t *video_outbuf = (uint8_t*)av_malloc(nbytes*sizeof (uint8_t));
    if( video_outbuf == nullptr )
    {
        cout<<"\nunable to allocate memory";
        exit(1);
    }

    // Setup the data pointers and linesizes based on the specified image parameters and the provided array.
    ret = av_image_fill_arrays( scaledFrame->data, scaledFrame->linesize, video_outbuf , AV_PIX_FMT_YUV420P, outVCodecContext->width,outVCodecContext->height,1 ); // returns : the size in bytes required for src
    if(ret < 0)
    {
        cout<<"\nerror in filling image array";
    }


    SwsContext* swsCtx_ ;

    // Allocate and return swsContext.
    // a pointer to an allocated context, or NULL in case of error
    swsCtx_ = sws_getContext(inVCodecContext->width,
                             inVCodecContext->height,
                             inVCodecContext->pix_fmt,
                             outVCodecContext->width,
                             outVCodecContext->height,
                             outVCodecContext->pix_fmt,
                             SWS_BICUBIC, NULL, NULL, NULL);



    // Unique lock with defer lock to not lock automatically at the construction of the unique lock
    std::unique_lock<std::mutex> r_lock(r_mutex, std::defer_lock);


    cout<<"\n\n[VideoThread] thread started!";
    bool paused = false;
    while(true) {
        r_lock.lock();
        paused = !captureSwitch && captureStarted;
        if (paused) inVideo.closeInput();

        r_cv.wait(r_lock, [&](){return (captureSwitch || killSwitch);});
        if(killSwitch) {
            cout << "\n[VideoThread] thread stopped!";
            /*  ret = av_write_trailer(outAVFormatContext);
              if( ret < 0)
              {
                  cout<<"\nerror in writing av trailer";
                  exit(1);
              }*/
            return;
        }

        if (paused) inVideo.open();
        r_lock.unlock();


        if(av_read_frame(inVFormatContext, inPacket) >= 0 && inPacket->stream_index == inVideoStreamIndex) {
            //decode video routine

            av_packet_rescale_ts(inPacket,  inVFormatContext->streams[inVideoStreamIndex]->time_base,inVCodecContext->time_base);
            if((ret = avcodec_send_packet(inVCodecContext, inPacket)) < 0){
                cout << "Cannot decode current video packet " <<  ret;
                continue;
            }
            while (ret >= 0) {
                ret = avcodec_receive_frame(inVCodecContext, rawFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;
                else if (ret < 0) {
                    fprintf(stderr, "Error during decoding\n");
                    exit(1);
                }
                //raw frame ready
                if(outAVFormatContext->streams[outVideoStreamIndex]->start_time <= 0) {
                    outAVFormatContext->streams[outVideoStreamIndex]->start_time = rawFrame->pts;
                }

                av_init_packet(outPacket);
                outPacket->data =  nullptr;    // packet data will be allocated by the encoder
                outPacket->size = 0;

                /*initializing scaleFrame */
                scaledFrame->width = outVCodecContext->width;
                scaledFrame->height = outVCodecContext->height;
                scaledFrame->format = outVCodecContext->pix_fmt;
                scaledFrame->pts = rawFrame->pts;
                scaledFrame->pkt_dts=rawFrame->pkt_dts;
                scaledFrame->best_effort_timestamp = rawFrame->best_effort_timestamp;
                //av_frame_get_buffer(scaledFrame, 0);

                sws_scale(swsCtx_, rawFrame->data, rawFrame->linesize,0, inVCodecContext->height, scaledFrame->data, scaledFrame->linesize);

                if(avcodec_send_frame(outVCodecContext, scaledFrame)< 0){
                    cout << "Cannot encode current video packet " << AVERROR(EAGAIN);
                    exit(1);
                }
                while(ret>=0){
                    ret = avcodec_receive_packet(outVCodecContext, outPacket);
                    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                        break;
                    else if (ret < 0) {
                        fprintf(stderr, "Error during encoding\n");
                        exit(1);
                    }
                    //outPacket ready
                    if(outPacket->pts != AV_NOPTS_VALUE)
                        outPacket->pts = av_rescale_q(outPacket->pts, outVCodecContext->time_base,  outAVFormatContext->streams[outVideoStreamIndex]->time_base);
                    if(outPacket->dts != AV_NOPTS_VALUE)
                        outPacket->dts = av_rescale_q(outPacket->dts, outVCodecContext->time_base, outAVFormatContext->streams[outVideoStreamIndex]->time_base);

                    outPacket->stream_index = outVideoStreamIndex;
                    w_lock.lock();
                    if(av_interleaved_write_frame(outAVFormatContext , outPacket) != 0)
                    {
                        cout<<"\nerror in writing video frame";
                    }

                    w_lock.unlock();
                    av_packet_unref(outPacket);
                } // got_picture
                av_packet_unref(outPacket);
            }
        }

    }

}


/**
 * captureAudio() is the "AudioThread" execution flow.
 * This execution flow get packets from audio input device
 * and decode them by sending them to the decoder
 *
 * @Note captureAudio() is a thread-safe execution flow, has to be passed to a specific thread to ensure the correct execution
 */

static int64_t pts = 0;

void Controller::captureAudio() {
    int ret;
    AVPacket *inPacket, *outPacket;
    AVFrame *rawFrame, *scaledFrame;
    uint8_t  **resampledData;
    //allocate space for a packet
    inPacket = (AVPacket *) av_packet_alloc();
    if(!inPacket) {
        cout << "\nCannot allocate an AVPacket for encoded video";
        exit(1);
    }
    av_init_packet(inPacket);

    //allocate space for a packet
    rawFrame = av_frame_alloc();
    if(!rawFrame) {
        cout << "\nCannot allocate an AVPacket for encoded video";
        exit(1);
    }

    outPacket = (AVPacket *) av_packet_alloc();
    if(!outPacket) {
        cout << "\nCannot allocate an AVPacket for encoded video";
        exit(1);
    }



    //init the resampler
    SwrContext* resampleContext = nullptr;
    resampleContext = swr_alloc_set_opts(resampleContext,
                                         av_get_default_channel_layout(outACodecContext->channels),
                                         outACodecContext->sample_fmt,
                                         outACodecContext->sample_rate,
                                         av_get_default_channel_layout(inACodecContext->channels),
                                         inACodecContext->sample_fmt,
                                         inACodecContext->sample_rate,
                                         0, NULL);
    if(!resampleContext){
        cout << "\nCannot allocate the resample context";
        exit(1);
    }
    if ((swr_init(resampleContext)) < 0) {
        fprintf(stderr, "Could not open resample context\n");
        swr_free(&resampleContext);
        exit(1);
    }


    cout<<"\n\n[AudioThread] thread started!";
    std::unique_lock<std::mutex> r_lock(r_mutex, std::defer_lock);
    bool paused = false;
    while(true) {


        r_lock.lock();
        paused = !captureSwitch && captureStarted;
        if (paused) inAudio.closeInput();

        r_cv.wait(r_lock, [&](){return (captureSwitch || killSwitch);});

        if(killSwitch) {
            cout << "\n[AudioThread] thread stopped!";
            return;
        }

        if (paused) inAudio.open();
        r_lock.unlock();

        if(av_read_frame(inAFormatContext, inPacket) >= 0 && inPacket->stream_index == inAudioStreamIndex) {
            //decode video routing
            av_packet_rescale_ts(outPacket,  inAFormatContext->streams[inAudioStreamIndex]->time_base, inACodecContext->time_base);
            if((ret = avcodec_send_packet(inACodecContext, inPacket)) < 0){
                cout << "Cannot decode current video packet " <<  ret;
                continue;
            }
            while (ret >= 0) {
                ret = avcodec_receive_frame(inACodecContext, rawFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;
                else if (ret < 0) {
                    fprintf(stderr, "Error during decoding\n");
                    exit(1);
                }
                if(outAVFormatContext->streams[outAudioStreamIndex]->start_time <= 0) {
                    outAVFormatContext->streams[outAudioStreamIndex]->start_time = rawFrame->pts;
                }
                initConvertedSamples(&resampledData, outACodecContext, rawFrame->nb_samples);

                swr_convert(resampleContext,
                            resampledData, rawFrame->nb_samples,
                            (const uint8_t **)rawFrame->extended_data, rawFrame->nb_samples);

                add_samples_to_fifo(resampledData,rawFrame->nb_samples);

                //raw frame ready
                av_init_packet(outPacket);
                outPacket->data = nullptr;    // packet data will be allocated by the encoder
                outPacket->size = 0;

                const int frame_size = FFMAX(av_audio_fifo_size(fifo), outACodecContext->frame_size);

                scaledFrame = av_frame_alloc();
                if(!scaledFrame) {
                    cout << "\nCannot allocate an AVPacket for encoded video";
                    exit(1);
                }

                scaledFrame->nb_samples     = outACodecContext->frame_size;
                scaledFrame->channel_layout = outACodecContext->channel_layout;
                scaledFrame->format         = outACodecContext->sample_fmt;
                scaledFrame->sample_rate    = outACodecContext->sample_rate;
                // scaledFrame->best_effort_timestamp = rawFrame->best_effort_timestamp;
                // scaledFrame->pts = rawFrame->pts;
                av_frame_get_buffer(scaledFrame,0);

                while (av_audio_fifo_size(fifo) >= outACodecContext->frame_size){
                    ret = av_audio_fifo_read(fifo, (void **)(scaledFrame->data), outACodecContext->frame_size);
                    scaledFrame->pts = pts;
                    pts += scaledFrame->nb_samples;
                    if(avcodec_send_frame(outACodecContext, scaledFrame) < 0){
                        cout << "Cannot encode current audio packet ";
                        exit(1);
                    }
                    while(ret>=0){
                        ret = avcodec_receive_packet(outACodecContext, outPacket);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                            break;
                        else if (ret < 0) {
                            fprintf(stderr, "Error during encoding\n");
                            exit(1);
                        }
                        //outPacket ready
                        av_packet_rescale_ts(outPacket, outACodecContext->time_base,  outAVFormatContext->streams[outAudioStreamIndex]->time_base);


                        outPacket->stream_index = outAudioStreamIndex;

                        w_lock.lock();
                        if(av_interleaved_write_frame(outAVFormatContext , outPacket) != 0)
                        {
                            cout<<"\nerror in writing audio frame";
                        }
                        w_lock.unlock();
                        av_packet_unref(outPacket);
                    }
                    ret=0;
                }// got_picture
                av_frame_free(&scaledFrame);
                av_packet_unref(outPacket);
                //av_freep(&resampledData[0]);
                // free(resampledData);
            }
        }

    }

}

/**
 * startCapture() enables Audio and Video capturing threads
 * @Note is callable only after thread initialization by mean of initThreads();
 */
void Controller::startCapture() {
    cout<<"\n[MainThread] Capture started";
    cout<<"\n[MainThread] Capturing audio: " << (settings._recaudio ? "yes" : "no") ;
    std::lock_guard<std::mutex> r_lock(r_mutex);
    if (settings._recaudio)
        init_fifo();
    captureSwitch = true;
    captureStarted = true;
    r_cv.notify_all();
}
/**
 * pauseCapture() pauses Audio and Video capturing threads
 * @Note is callable only after thread initialization by mean of initThreads();
 */
void Controller::pauseCapture() {
    if (!captureSwitch || killSwitch) return;
    std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    cout<<"\n[MainThread] Trying to pause capture... "<<std::ctime(&time);
    std::lock_guard<std::mutex> r_lock(r_mutex);
    cout<<"\n[MainThread] Capture paused";
    captureSwitch = false;
    r_cv.notify_all();
}
/**
 * resumeCapture() resumes Audio and Video capturing threads
 * @Note is callable only after thread initialization by mean of initThreads();
 */
void Controller::resumeCapture() {
    if (captureSwitch || killSwitch) return;
    cout<<"\n[MainThread] Trying to resume capture...";
    std::lock_guard<std::mutex> r_lock(r_mutex);
    captureSwitch = true;
    cout<<"\n[MainThread] Capture resumed";
    r_cv.notify_all();
}
/**
 * endCapture() ends Audio and Video capturing threads
 * @Note is callable only after thread initialization by mean of initThreads();
 */
void Controller::endCapture() {
    if (killSwitch) return;
    cout<<"\n[MainThread] Trying to end capture...";
    std::lock_guard<std::mutex> r_lock(r_mutex);
    killSwitch = true;
    cout<<"\n[MainThread] Capture ended";
    r_cv.notify_all();

}

/**
 * initThreads() generate threads and initialize them by passing the right execution flow.\n
 * Following threads are created:\n
 * - AudioThread handles the real-time audio capturing and decoding \n
 * - VideoThread handles the real-time video capturing and decoding \n
 * - ProducerThread handles encoding and multiplexing of the video and audio streams. \n
 *
 */
void Controller::initThreads() {

    if(settings._recvideo) videoThread = thread([&](){captureVideo();});
    if(settings._recaudio) audioThread = thread([&](){captureAudio();});

}

/**
 * initOptions() initializes the SROption data structure
 *
 * @note Has to be called before initOutput() and initThreads()
 */
void Controller::initOptions() {
    settings.filename = strdup("");
    settings._recaudio=false;
    settings._inscreenres={0,0};
    settings._outscreenres={0,0};
    settings._screenoffset={0,0};
}

int Controller::init_fifo()
{
    /* Create the FIFO buffer based on the specified output sample format. */
    if (!(fifo = av_audio_fifo_alloc(outACodecContext->sample_fmt,
                                     outACodecContext->channels, 1))) {
        fprintf(stderr, "Could not allocate FIFO\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}

int Controller::add_samples_to_fifo(uint8_t **converted_input_samples, const int frame_size){
    int error;
    /* Make the FIFO as large as it needs to be to hold both,
     * the old and the new samples. */
    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frame_size)) < 0) {
        fprintf(stderr, "Could not reallocate FIFO\n");
        return error;
    }
    /* Store the new samples in the FIFO buffer. */
    if (av_audio_fifo_write(fifo, (void **)converted_input_samples, frame_size) < frame_size) {
        fprintf(stderr, "Could not write data to FIFO\n");
        return AVERROR_EXIT;
    }
    return 0;
}

int Controller::initConvertedSamples(uint8_t ***converted_input_samples, AVCodecContext *output_codec_context, int frame_size){
    int error;
    /* Allocate as many pointers as there are audio channels.
     * Each pointer will later point to the audio samples of the corresponding
     * channels (although it may be NULL for interleaved formats).
     */
    if (!(*converted_input_samples = (uint8_t **)calloc(output_codec_context->channels,
                                                        sizeof(**converted_input_samples)))) {
        fprintf(stderr, "Could not allocate converted input sample pointers\n");
        return AVERROR(ENOMEM);
    }
    /* Allocate memory for the samples of all channels in one consecutive
     * block for convenience. */
    if (av_samples_alloc(*converted_input_samples, nullptr,
                         output_codec_context->channels,
                         frame_size,
                         output_codec_context->sample_fmt, 0) < 0) {

        exit(1);
    }
    return 0;
}


void Controller::infoDisplays() {

#ifdef __unix__
    for (int i = 0; i < ScreenCount (dpy); i++) {
        printf ("\n");
        printf ("screen #%d:\n", i);
        printf ("  dimensions:    %dx%d pixels\n\n",
                XDisplayWidth (dpy, i),  XDisplayHeight (dpy, i));
    }
#endif

#ifdef _WIN32

#endif

}


void Controller::listDevices() {
    int value = 0;
    AVDictionary *opts = nullptr;
    AVFormatContext  *inProbeFormatContext = avformat_alloc_context();
    AVInputFormat  *avInput = nullptr;

    value = av_dict_set(&opts, "list_devices", "true", 0);
    if (value < 0) {
        cout << "\nerror in setting dictionary value";
        exit(1);
    }

    avInput = av_find_input_format(VIDEO_SOURCE);
    value = avformat_open_input(&inProbeFormatContext,"", avInput, &opts);
    if (value != 0) {
        cout << "\nCannot open selected device";
        exit(1);
    }
    avformat_close_input(&inProbeFormatContext);
    avformat_free_context(inProbeFormatContext);
    av_freep((void *) avInput);

}
