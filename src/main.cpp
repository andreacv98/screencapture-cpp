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

    /*settings*/
    sc.settings.filename = "output.mp4";
    sc.settings._recaudio = true;
    sc.settings._recvideo = true;

    sc.settings._inscreenres = {1920,1080};
    sc.settings._outscreenres = {1920,1080};
    sc.settings._fps = 14;
    sc.settings._screenoffset = {0,0};

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