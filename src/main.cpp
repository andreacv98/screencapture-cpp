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
    int x_start;
    int x_toadd;
    int y_start;
    int y_toadd;


    std::cout << "\n\nStarted!\n\n" << std::endl;

    /*settings*/
    sc.settings._recvideo = true;


#ifdef __unix__
    std::cout << "---Select screen # to record---" << std::endl;
    sc.infoDisplays();
    std::cin >>screen_number;
#endif


    std::cout << "---Enter n pixel to start area capture x (x offset) ---" << std::endl;
    std::cin >>x_start;
    std::cout << "---Enter n pixel to start area capture y (y offset) ---" << std::endl;
    std::cin >>y_start;
    std::cout << "---Enter n pixel to add to start area capture x (x added) ---" << std::endl;
    std::cin >>x_toadd;
    std::cout << "---Enter n pixel to add to start area capture y (y added) ---" << std::endl;
    std::cin >>y_toadd;


#ifdef _WIN32
    sc.settings._screenoffset = {x_start, y_start};
    sc.settings._inscreenres= {x_toadd, y_toadd};
#endif

#ifdef __unix__
    sc.settings._screenoffset = {x_start, y_start};
    sc.settings._inscreenres= {1280, 720};
#endif

/*
#ifdef __unix__
    sc.settings._inscreenres = {XDisplayWidth(sc.dpy,screen_number),  XDisplayHeight(sc.dpy,screen_number)};
#endif
#ifdef _WIN32
    sc.settings._inscreenres = {2560,1440};
#endif
    */

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

    std::cout << "---Choose output resolution #" << std::endl;
    std::cout << "resolution #0: \t1280x720 pixels"
                 "\nresolution #1: \t1920x1080 pixels"<<
                 "\nresolution #2: \t2560x1440 pixels"<< std::endl;
    std::cin >> resolution_out;

    switch (resolution_out) {
        case 0:
            sc.settings._outscreenres = {1280,720};
            break;
        case 1:
            sc.settings._outscreenres = {1920,1080};
            break;
        case 2:
            sc.settings._outscreenres = {2560,1440};
            break;
        default:
            break;
    }

    sc.settings._fps = 15;

    /*open input devices*/
    sc.openVideoSource();
    sc.openAudioSource();

    /*init data structures and threads based on settings*/
    sc.initOutputFile();
    sc.initThreads();



    /* sample capture routine*/
    sc.startCapture();
    sleep(10);
    sc.endCapture();

    return 0;
}