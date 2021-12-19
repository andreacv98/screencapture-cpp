#include <iostream>
#include "ScreenRecorder.h"

int main() {
    ScreenRecorder sc;
    if(sc.openVideoSource()==0) {
        std::cout<<"\nVideo Input Initialized";
    }
    if(sc.openAudioSource()==0) {
        std::cout<<"\nAudio Input Initialized";
    }
    sc.initOutputFile("output.mp4", true);
    return 0;
}