#include <iostream>
#include <thread>
#include "ScreenRecorder.h"

#ifdef __APPLE__
#include <unistd.h>
#endif

#ifdef __unix__
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
void sleep(unsigned milliseconds)
 {
    std::this_thread::sleep_for(std::chrono::seconds(milliseconds));
 }
#endif

int main() {
    ScreenRecorder sc;

    int screen_number;
    bool audio;
    int audio_number;
    char output_name[100];
    int resolution_out;


    std::cout << "\n\nStarted!\n\n" << std::endl;

    /*settings*/
    sc.settings._recvideo = true;

    std::cout << "---Select screen # to record---" << std::endl;
    sc.infoDisplays();
    std::cin >>screen_number;

    std::cout << "---Do you want audio record? 1 for yes, 0 for not ---" << std::endl;
    std::cin >>audio;

    if (audio){
        sc.settings._recaudio = true;

        /*
        std::cout << "---Select audio # to record---" << std::endl;
        std::cin >>audio_number;
         */
    }else sc.settings._recaudio = false;

    std::cout << "---Insert name for output file and extensions (max 100chr) eg. 'output.mp4'---" << std::endl;
    std::cin >>output_name;

    sc.settings.filename = output_name;

    sc.settings._inscreenres = {XDisplayWidth(sc.dpy,screen_number),  XDisplayHeight(sc.dpy,screen_number)};

    std::cout << "---Choose output resolution #" << std::endl;
    std::cout << "resolution #0: \t1280x720 pixels"
                 "\nresolution #1: \t1920x1080 pixels"<< std::endl;
    std::cin >> resolution_out;

    switch (resolution_out) {
        case 0:
            sc.settings._outscreenres = {1280,720};
            break;
        case 1:
            sc.settings._outscreenres = {1920,1080};
            break;
        default:
            break;
    }

    sc.settings._fps = 12;

    sc.settings._screenoffset = {0,0};

    /*open input devices*/
    sc.openVideoSource();
    sc.openAudioSource();

    /*init data structures and threads based on settings*/
    sc.initOutputFile();
    sc.initThreads();


    /* sample capture routine*/
    sleep(2);
    sc.startCapture();

    sleep(5);
    sc.pauseCapture();

    sleep(5);
    sc.resumeCapture();

    sleep(5);
    sc.endCapture();

    return 0;
}