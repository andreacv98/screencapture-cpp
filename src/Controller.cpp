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
                                                                               inVideo(nullptr),
                                                                               inAudio(nullptr),
                                                                               output(nullptr)
                                                                               {
    inVideoBuffer.np = 0;
    inAudioBuffer.np = 0;

    avdevice_register_all();

    inVideo = make_unique<VideoDemuxer>(VIDEO_SOURCE, videoUrl, settings._fps, settings._inscreenres);
    inAudio = make_unique<AudioDemuxer>(AUDIO_SOURCE, audioUrl);
    output = make_unique<Muxer>(settings);

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

    cout << "\nScreen Recorder ready to start\n";
}

Controller::~Controller() {

    if(settings._recvideo && videoThread.joinable()) videoThread.join();
    if(settings._recaudio && audioThread.joinable()) audioThread.join();

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
    int nbytes = av_image_get_buffer_size(output->getVCodecContext()->pix_fmt,output->getVCodecContext()->width,output->getVCodecContext()->height,32);
    uint8_t *video_outbuf = (uint8_t*)av_malloc(nbytes*sizeof (uint8_t));
    if( video_outbuf == nullptr )
    {
        cout<<"\nunable to allocate memory";
        exit(1);
    }

    // Setup the data pointers and linesizes based on the specified image parameters and the provided array.
    ret = av_image_fill_arrays( scaledFrame->data, scaledFrame->linesize, video_outbuf , AV_PIX_FMT_YUV420P, output->getVCodecContext()->width,output->getVCodecContext()->height,1 ); // returns : the size in bytes required for src
    if(ret < 0)
    {
        cout<<"\nerror in filling image array";
    }


    SwsContext* swsCtx_ ;

    // Allocate and return swsContext.
    // a pointer to an allocated context, or NULL in case of error
    swsCtx_ = sws_getContext(decoderVideo->getCodecContext()->width,
                             decoderVideo->getCodecContext()->height,
                             decoderVideo->getCodecContext()->pix_fmt,
                             output->getVCodecContext()->width,
                             output->getVCodecContext()->height,
                             output->getVCodecContext()->pix_fmt,
                             SWS_BICUBIC, NULL, NULL, NULL);



    // Unique lock with defer lock to not lock automatically at the construction of the unique lock
    std::unique_lock<std::mutex> r_lock(r_mutex, std::defer_lock);


    cout<<"\n\n[VideoThread] thread started!";
    bool paused = false;
    while(true) {
        r_lock.lock();
        paused = !captureSwitch && captureStarted;
        if (paused) inVideo->closeInput();

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

        if (paused) inVideo->open();
        r_lock.unlock();

        if(av_read_frame(inVideo->getInFormatContext(), inPacket) >= 0 && inPacket->stream_index == inVideoStreamIndex) {
            //decode video routine

            av_packet_rescale_ts(inPacket,  inVideo->getInFormatContext()->streams[inVideoStreamIndex]->time_base,decoderVideo->getCodecContext()->time_base);
            ret = decoderVideo->sendPacket(inPacket);
            while (decoderVideo->getDecodedOutput(rawFrame) >= 0) {
                //raw frame ready
                if(output->getOutAVFormatContext()->streams[output->outVideoStreamIndex]->start_time <= 0) {
                    output->getOutAVFormatContext()->streams[output->outVideoStreamIndex]->start_time = rawFrame->pts;
                }

                av_init_packet(outPacket);
                outPacket->data =  nullptr;    // packet data will be allocated by the encoder
                outPacket->size = 0;

                /*initializing scaleFrame */
                scaledFrame->width = output->getVCodecContext()->width;
                scaledFrame->height = output->getVCodecContext()->height;
                scaledFrame->format = output->getVCodecContext()->pix_fmt;
                scaledFrame->pts = rawFrame->pts;
                scaledFrame->pkt_dts=rawFrame->pkt_dts;
                scaledFrame->best_effort_timestamp = rawFrame->best_effort_timestamp;
                //av_frame_get_buffer(scaledFrame, 0);

                sws_scale(swsCtx_, rawFrame->data, rawFrame->linesize,0, decoderVideo->getCodecContext()->height, scaledFrame->data, scaledFrame->linesize);

                encoderVideo->sendFrame(scaledFrame);
                while(encoderVideo->getPacket(outPacket)>=0){
                    //outPacket ready
                    if(outPacket->pts != AV_NOPTS_VALUE)
                        outPacket->pts = av_rescale_q(outPacket->pts, encoderVideo->getCodecContext()->time_base,  output->getOutAVFormatContext()->streams[output->outVideoStreamIndex]->time_base);
                    if(outPacket->dts != AV_NOPTS_VALUE)
                        outPacket->dts = av_rescale_q(outPacket->dts, encoderVideo->getCodecContext()->time_base, output->getOutAVFormatContext()->streams[output->outVideoStreamIndex]->time_base);

                    outPacket->stream_index = output->outVideoStreamIndex;
                    w_lock.lock();
                    if(av_interleaved_write_frame(output->getOutAVFormatContext() , outPacket) != 0)
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
                                         av_get_default_channel_layout(encoderAudio->getCodecContext()->channels),
                                         encoderAudio->getCodecContext()->sample_fmt,
                                         encoderAudio->getCodecContext()->sample_rate,
                                         av_get_default_channel_layout(decoderAudio->getCodecContext()->channels),
                                         decoderAudio->getCodecContext()->sample_fmt,
                                         decoderAudio->getCodecContext()->sample_rate,
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
        if (paused) inAudio->closeInput();

        r_cv.wait(r_lock, [&](){return (captureSwitch || killSwitch);});

        if(killSwitch) {
            cout << "\n[AudioThread] thread stopped!";
            return;
        }

        if (paused) inAudio->open();
        r_lock.unlock();

        if(av_read_frame(inAudio->getInFormatContext(), inPacket) >= 0 && inPacket->stream_index == inAudioStreamIndex) {
            //decode video routing
            av_packet_rescale_ts(outPacket,  inAudio->getInFormatContext()->streams[inAudioStreamIndex]->time_base, decoderAudio->getCodecContext()->time_base);
            ret = decoderAudio->sendPacket(inPacket);
            while (decoderAudio->getDecodedOutput(rawFrame) >= 0) {
                if(output->getOutAVFormatContext()->streams[output->outAudioStreamIndex]->start_time <= 0) {
                    output->getOutAVFormatContext()->streams[output->outAudioStreamIndex]->start_time = rawFrame->pts;
                }
                initConvertedSamples(&resampledData, encoderAudio->getCodecContext(), rawFrame->nb_samples);

                swr_convert(resampleContext,
                            resampledData, rawFrame->nb_samples,
                            (const uint8_t **)rawFrame->extended_data, rawFrame->nb_samples);

                add_samples_to_fifo(resampledData,rawFrame->nb_samples);

                //raw frame ready
                av_init_packet(outPacket);
                outPacket->data = nullptr;    // packet data will be allocated by the encoder
                outPacket->size = 0;

                const int frame_size = FFMAX(av_audio_fifo_size(fifo), encoderAudio->getCodecContext()->frame_size);

                scaledFrame = av_frame_alloc();
                if(!scaledFrame) {
                    cout << "\nCannot allocate an AVPacket for encoded video";
                    exit(1);
                }

                scaledFrame->nb_samples     = encoderAudio->getCodecContext()->frame_size;
                scaledFrame->channel_layout = encoderAudio->getCodecContext()->channel_layout;
                scaledFrame->format         = encoderAudio->getCodecContext()->sample_fmt;
                scaledFrame->sample_rate    = encoderAudio->getCodecContext()->sample_rate;
                // scaledFrame->best_effort_timestamp = rawFrame->best_effort_timestamp;
                // scaledFrame->pts = rawFrame->pts;
                av_frame_get_buffer(scaledFrame,0);

                while (av_audio_fifo_size(fifo) >= encoderAudio->getCodecContext()->frame_size){
                    ret = av_audio_fifo_read(fifo, (void **)(scaledFrame->data), encoderAudio->getCodecContext()->frame_size);
                    scaledFrame->pts = pts;
                    pts += scaledFrame->nb_samples;
                    encoderAudio->sendFrame(scaledFrame);
                    while(encoderAudio->getPacket(outPacket)>=0){
                        //outPacket ready
                        av_packet_rescale_ts(outPacket, encoderAudio->getCodecContext()->time_base,  output->getOutAVFormatContext()->streams[output->outAudioStreamIndex]->time_base);


                        outPacket->stream_index = output->outAudioStreamIndex;

                        w_lock.lock();
                        if(av_interleaved_write_frame(output->getOutAVFormatContext() , outPacket) != 0)
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
    if (captureStarted) return;

    set();

    initThreads();
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
    std::lock_guard<std::mutex> r_lock(r_mutex);
    cout<<"\n[MainThread] Capture paused\n";
    captureSwitch = false;
    r_cv.notify_all();
}
/**
 * resumeCapture() resumes Audio and Video capturing threads
 * @Note is callable only after thread initialization by mean of initThreads();
 */
void Controller::resumeCapture() {
    if (captureSwitch || killSwitch) return;
    std::lock_guard<std::mutex> r_lock(r_mutex);
    captureSwitch = true;
    cout<<"\n[MainThread] Capture resumed\n";
    r_cv.notify_all();
}
/**
 * endCapture() ends Audio and Video capturing threads
 * @Note is callable only after thread initialization by mean of initThreads();
 */
void Controller::endCapture() {
    if (killSwitch) return;
    std::lock_guard<std::mutex> r_lock(r_mutex);
    killSwitch = true;
    cout<<"\n[MainThread] Capture ended\n";
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
    if (!(fifo = av_audio_fifo_alloc(encoderAudio->getCodecContext()->sample_fmt,
                                     encoderAudio->getCodecContext()->channels, 1))) {
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

int Controller::initConvertedSamples(uint8_t ***converted_input_samples, const AVCodecContext *output_codec_context, int frame_size){
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

void Controller::set() {
    // INPUT
    if (settings._recvideo){
        try {
            decoderVideo = make_unique<Decoder>();
            inVideo->open();
            decoderVideo->setCodecContext(inVideo->getInCodecContext());
        } catch (const std::runtime_error& e) {
            cerr << "Error opening video input: " << e.what() << endl;
            throw;
        }
    }

    if (settings._recaudio){
        try {
            decoderAudio = make_unique<Decoder>();
            inAudio->open();
            decoderAudio->setCodecContext(inAudio->getInCodecContext());
        } catch (const std::runtime_error& e) {
            cerr << "Error opening audio input: " << e.what() << endl;
            throw;
        }
    }
    // OUTPUT
    if (settings._recvideo || settings._recaudio){
        output->initOutputFile(decoderAudio->getCodecContext());
    }

    if (settings._recvideo){
        try {
            encoderVideo = make_unique<Encoder>();
            encoderVideo->setCodecContext(output->getVCodecContext());
        } catch (const std::runtime_error& e) {
            cerr << "Error opening video output: " << e.what() << endl;
            throw;
        }
    }

    if (settings._recaudio){
        try {
            encoderAudio = make_unique<Encoder>();
            encoderAudio->setCodecContext(output->getACodecContext());
        } catch (const std::runtime_error& e) {
            cerr << "Error opening audio output: " << e.what() << endl;
            throw;
        }
    }
}
