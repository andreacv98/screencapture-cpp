#include <iostream>
#include <thread>
#include "ScreenRecorder.h"
#include "Controller.h"

#ifdef __APPLE__
#include <unistd.h>
#endif

#ifdef __unix__
#include <unistd.h>
#endif

#ifdef _WIN32
void sleep(unsigned milliseconds)
 {
    std::this_thread::sleep_for(std::chrono::seconds(milliseconds));
 }
#endif

int main() {
    int screen_number;
    bool audio;
    int audio_number;
    char output_name[100];
    int resolution_out;
    int x_start;
    int x_toadd;
    int y_start;
    int y_toadd;
    char video_url [50];

    bool test = true;


    SRSettings settings;

    if (test){
       settings._recvideo = true;
       settings._recaudio = true;
       x_start = 0;
       y_start = 0;
       x_toadd = 1920;
       y_toadd = 1080;
       settings._screenoffset = {x_start, y_start};
       settings._inscreenres= {x_toadd, y_toadd};
       settings.filename = "test.mp4";
       settings._outscreenres = {1920,1080};
       settings._fps = 15;

       //Controller c("alsa_input.pci-0000_00_05.0.analog-stereo", ":0.0+0,0", settings);
        Controller c("audio=Microfono (Logitech G533 Gaming Headset)", "desktop", settings);


        bool capturing = true;
        int command = 0;

       /* sample capture routine*/
       while(capturing){
           std::cout << "\n--- 0 -> start | 1 -> pause | 2 -> resume | 3 -> end ---\n";
           std::cin>>command;
           switch (command) {
               case 0:
                   c.startCapture();
                   break;
               case 1:
                   c.pauseCapture();
                   break;
               case 2:
                   c.resumeCapture();
                   break;
               case 3:
                   c.endCapture();
                   capturing = false;
               default:
                   break;
           }
       }
       return 0;
    }

    std::cout << "\n\nStarted!\n\n" << std::endl;

    /*settings*/
    settings._recvideo = true;

    #ifdef __unix__
    std::cout << "---Dispays info---" << std::endl;
    sc.infoDisplays();
    #endif


    std::cout << "---Enter n pixel to start area capture x (x origin area) ---" << std::endl;
    std::cin >>x_start;
    std::cout << "---Enter n pixel to start area capture y (y origin area) ---" << std::endl;
    std::cin >>y_start;

#ifdef __unix__
    std::cout << "---Enter n pixel to asix x to record (x pixels) max: ---" << XDisplayWidth(sc.dpy,0) - x_start <<std::endl;
    std::cin >>x_toadd;
    std::cout << "---Enter n pixel to asix y to record (y pixels) max: ---" << XDisplayHeight(sc.dpy,0) - y_start <<std::endl;
    std::cin >>y_toadd;
#endif

#ifdef _WIN32
    std::cout << "---Enter n pixel to asix x to record (x pixels) ---" << std::endl;
    std::cin >>x_toadd;
    std::cout << "---Enter n pixel to asix y to record (y pixels) ---" << std::endl;
    std::cin >>y_toadd;
#endif

    settings._screenoffset = {x_start, y_start};
    settings._inscreenres= {x_toadd, y_toadd};


    std::cout << "---Do you want audio record? 1 for yes, 0 for not ---" << std::endl;
    std::cin >>audio;

    if (audio){
        settings._recaudio = true;

        /*
        std::cout << "---Select audio # to record---" << std::endl;
        std::cin >>audio_number;
         */
    }else settings._recaudio = false;

    std::cout << "---Insert name for output file and extensions (max 100chr) eg. 'output.mp4'---" << std::endl;
    std::cin >>output_name;

    settings.filename = output_name;

    std::cout << "---Choose output resolution #" << std::endl;
    std::cout << "resolution #0: \t1280x720 pixels"
                 "\nresolution #1: \t1920x1080 pixels"<<
                 "\nresolution #2: \t2560x1440 pixels"<< std::endl;
    std::cin >> resolution_out;

    switch (resolution_out) {
        case 0:
            settings._outscreenres = {1280,720};
            break;
        case 1:
            settings._outscreenres = {1920,1080};
            break;
        case 2:
            settings._outscreenres = {2560,1440};
            break;
        default:
            break;
    }

    settings._fps = 15;


#ifdef _WIN32
    Controller c("audio=Microfono (Logitech G533 Gaming Headset)", "desktop", settings);
#endif

#ifdef __unix__
    sprintf(video_url,":0.0+%d,%d", settings._screenoffset.x, settings._screenoffset.y);
    //printf("VideoUrl:%s", video_url);
    Controller c("alsa_input.pci-0000_00_05.0.analog-stereo", video_url, settings);
#endif

    bool capturing = true;
    int command = 0;

    /* sample capture routine*/
    while(capturing){
        std::cout << "\n--- 0 -> start | 1 -> pause | 2 -> resume | 3 -> end ---\n";
        std::cin>>command;
        switch (command) {
            case 0:
                c.startCapture();
                break;
            case 1:
                c.pauseCapture();
                break;
            case 2:
                c.resumeCapture();
                break;
            case 3:
                c.endCapture();
                capturing = false;
            default:
                break;
        }
    }

    return 0;
}