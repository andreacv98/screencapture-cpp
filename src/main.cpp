#include <iostream>
#include <thread>
#include "ScreenRecorder.h"
#include "libs/Controller.h"

#ifdef __APPLE__
#include <unistd.h>
#endif

#ifdef __unix__
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <mmdeviceapi.h>
#include <stringapiset.h>
#include <initguid.h>  // Put this in to get rid of linker errors.
#include <Functiondiscoverykeys_devpkey.h>

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);

constexpr auto REFTIMES_PER_SEC = (10000000 * 25);
constexpr auto REFTIMES_PER_MILLISEC = 10000;

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

std::vector<std::string> ListEndpoints();

#endif

void menu(SRSettings *settings);
void registration_loop(Controller *c);

int main() {

    char audio_url[200];
    char video_url[200];

    SRSettings settings;
    std::cout << "\n\nStarted!\n\n" << std::endl;

    /*settings*/
    settings._recvideo = true;
    menu(&settings);

    //setting audio & video url
#ifdef _WIN32
    sprintf(audio_url, "audio=%s", settings.audio_url);
    sprintf(video_url,"desktop");
#endif

#ifdef __unix__
    sprintf(audio_url,"alsa_input.pci-0000_00_05.0.analog-stereo");
    sprintf(video_url,":0.0+%d,%d", settings._screenoffset.x, settings._screenoffset.y);
#endif


    //create Controller
    Controller c(audio_url, video_url, settings);

    registration_loop(&c);
    return 0;
}


void menu(SRSettings *settings){

    int fullscreen;
    bool audio;
    std::string output_name;
    int resolution_out;
    int x_start;
    int x_toadd;
    int y_start;
    int y_toadd;
    int audio_number;
    int i;

#ifdef __unix__
    Display *dpy = XOpenDisplay (NULL);
    //di.showInfos();
    resolution_width= XDisplayWidth(dpy,0);
    resolution_height = XDisplayHeight(dpy,0);
    std::cout << "---Dispays info---" << std::endl;
    std::cout <<"resolution_width:"  << resolution_width <<"\tresolution_height:"<<resolution_height<<"\n";

#endif

#ifdef _WIN32
    int resolution_width, resolution_height;

    resolution_width = GetSystemMetrics(SM_CXSCREEN);
    resolution_height = GetSystemMetrics(SM_CYSCREEN);
    std::cout << "---Dispays info---" << std::endl;
    std::cout <<"resolution_width:  "  << resolution_width <<"px\tresolution_height:  "<<resolution_height<<"px\n"<< std::endl;
#endif

    //setting resolution input
    std::cout << "---Do you want to record in fullscreen? 1 for yes, 0 for not---" << std::endl;
    std::cin >> fullscreen;
    while (fullscreen!=0 && fullscreen!=1){
        std::cout << "---ERROR : option not valid" << std::endl;
        std::cout << "---Do you want to record in fullscreen? 1 for yes, 0 for not---" << std::endl;
        std::cin >> fullscreen;
    }
    if (fullscreen){
        x_start=0;
        y_start=0;
        x_toadd= resolution_width;
        y_toadd= resolution_height;
    }else {

        std::cout << "---Enter n pixel to start area capture x (x origin area) ---" << std::endl;
        std::cin >>x_start;
        while (x_start>=resolution_width){
            std::cout << "---ERROR : origin can't be equal or greater than resolution" << std::endl;
            std::cout << "---REEnter n pixel to start area capture x (x origin area) ---" << std::endl;
            std::cin >>x_start;
        }

        std::cout << "---Enter n pixel to start area capture y (y origin area) ---" << std::endl;
        std::cin >>y_start;

        while (y_start>=resolution_height){
            std::cout << "---ERROR : origin can't be equal or greater than resolution" << std::endl;
            std::cout << "---REEnter n pixel to start area capture y (y origin area) ---" << std::endl;
            std::cin >>y_start;
        }

        std::cout << "---Enter n pixel to asix x to record (x pixels) max: ---" << resolution_width - x_start <<std::endl;
        std::cin >>x_toadd;

        while (x_toadd+x_start>resolution_width)  {
            std::cout << "---ERROR : too many x pixels" << std::endl;
            std::cout << "---ReEnter n pixel to start area capture x (x origin area) ---" << std::endl;
            std::cin >>x_start;
            while (x_start>=resolution_width){
                std::cout << "---ERROR : origin can't be equal or greater than resolution" << std::endl;
                std::cout << "---REEnter n pixel to start area capture x (x origin area) ---" << std::endl;
                std::cin >>x_start;
            }
            std::cout << "---ReEnter n pixel to asix x to record (x pixels) max: ---" << resolution_width - x_start <<std::endl;
            std::cin >>x_toadd;
        }

        std::cout << "---Enter n pixel to asix y to record (y pixels) max: ---" << resolution_height - y_start <<std::endl;
        std::cin >>y_toadd;

        while (y_toadd+y_start>resolution_height)  {
            std::cout << "---ERROR : too many y pixels" << std::endl;
            std::cout << "---Enter n pixel to start area capture y (y origin area) ---" << std::endl;
            std::cin >>y_start;

            while (y_start>=resolution_height){
                std::cout << "---ERROR : origin can't be equal or greater than resolution" << std::endl;
                std::cout << "---REEnter n pixel to start area capture y (y origin area) ---" << std::endl;
                std::cin >>y_start;
            }
            std::cout << "---Enter n pixel to asix y to record (y pixels) max: ---" << resolution_height - y_start <<std::endl;
            std::cin >>y_toadd;
        }

    }

    settings->_screenoffset = {x_start, y_start};
    settings->_inscreenres= {x_toadd, y_toadd};

    //setting resolution output
    std::cout << "---Choose output resolution #" << std::endl;
    std::cout << "resolution #0: \t1280x720 pixels"
                 "\nresolution #1: \t1920x1080 pixels"<<
              "\nresolution #2: \t2560x1440 pixels"<< std::endl;
    std::cin >> resolution_out;

    while (resolution_out != 0 && resolution_out != 1 &&  resolution_out != 2){
        std::cout << "---Choose output resolution #" << std::endl;
        std::cout << "resolution #0: \t1280x720 pixels"
                     "\nresolution #1: \t1920x1080 pixels"<<
                  "\nresolution #2: \t2560x1440 pixels"<< std::endl;
        std::cin >> resolution_out;
    }

    switch (resolution_out) {
        case 0:
            settings->_outscreenres = {1280,720};
            break;
        case 1:
            settings->_outscreenres = {1920,1080};
            break;
        case 2:
            settings->_outscreenres = {2560,1440};
            break;
        default:
            break;
    }

    //setting audio
    std::cout << "---Do you want audio record? 1 for yes, 0 for not ---" << std::endl;
    std::cin >>audio;
    while (audio!= 0 && audio!= 1){
        std::cout << "---Do you want audio record? 1 for yes, 0 for not ---" << std::endl;
        std::cin >>audio;
    }
    if (audio){
        settings->_recaudio = true;
#ifdef _WIN32
        std::cout << "---Select audio # to record---" << std::endl;
        std::vector<std::string> audioList = ListEndpoints();
        for (i=0; i<audioList.size(); i++){
            std::cout <<"#"<<i<<":\t"<<audioList[i]<<std::endl;
        }
        std::cin >>audio_number;
        while (audio_number<0 || audio_number>= i){
            std::cout << "---Select audio # to record---" << std::endl;
            std::vector<std::string> audioList = ListEndpoints();
            for (i=0; i<audioList.size(); i++){
                std::cout <<"#"<<i<<":\t"<<audioList[i]<<std::endl;
            }
            std::cin >>audio_number;
        }
        strcpy(settings->audio_url, audioList[audio_number].c_str());
#endif

    }else settings->_recaudio = false;

    std::cout << "---Insert name for output file and extensions (max 100chr) eg. 'output.mp4'---" << std::endl;
    std::cin >>settings->filename;
    settings->_fps = 15;
}

void registration_loop(Controller *c){
    bool capturing = true;
    int command ;

    std::cout << "\n--- 0 -> start ---\n";
    std::cin>>command;

    /* sample capture routine*/
    try{
        while(capturing){
            switch (command) {
                case 0:
                    c->startCapture();

                    std::cout << "\n\nRegistration started...\n";
                    std::cout << "\n--- 1 -> pause | 3 -> end ---\n";
                    std::cin>>command;
                    while (command!=1 && command!= 3){
                        std::cout << "---ERROR : option not valid" << std::endl;
                        std::cout << "\n--- 1 -> pause | 3 -> end ---\n";
                        std::cin>>command;
                    }
                    break;
                case 1:
                    c->pauseCapture();
                    std::cout << "\n--- 2 -> resume | 3 -> end ---\n";
                    std::cin>>command;
                    while (command!=2 && command!= 3){
                        std::cout << "---ERROR : option not valid" << std::endl;
                        std::cout << "\n--- 2 -> pause | 3 -> end ---\n";
                        std::cin>>command;
                    }
                    break;
                case 2:
                    c->resumeCapture();

                    std::cout << "\n--- 1 -> pause | 3 -> end ---\n";
                    std::cin>>command;
                    while (command!=1 && command!= 3){
                        std::cout << "---ERROR : option not valid" << std::endl;
                        std::cout << "\n--- 1 -> pause | 3 -> end ---\n";
                        std::cin>>command;
                    }
                    break;
                case 3:
                    c->endCapture();
                    capturing = false;
                default:
                    break;
            }
        }
    } catch (const std::runtime_error& e){
        std::cerr << "Runtime Error: " << e.what() << std::endl;
        exit(1);
    }
}

#ifdef _WIN32
//-----------------------------------------------------------
// This function enumerates all active (plugged in) audio
// rendering endpoint devices. It prints the friendly name
// and endpoint ID string of each endpoint device.
//-----------------------------------------------------------
std::vector<std::string> ListEndpoints()
{
    CoInitializeEx(NULL, COINIT_MULTITHREADED);
    HRESULT hr = S_OK;
    IMMDeviceEnumerator *pEnumerator = NULL;
    IMMDeviceCollection *pCollection = NULL;
    IMMDevice *pEndpoint = NULL;
    IPropertyStore *pProps = NULL;
    LPWSTR pwszID = NULL;
    std::vector<std::string> deviceNamesList;

    hr = CoCreateInstance(CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&pEnumerator);
    EXIT_ON_ERROR(hr);

    hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pCollection);
    EXIT_ON_ERROR(hr);

    UINT  count;
    hr = pCollection->GetCount(&count);
    EXIT_ON_ERROR(hr);

    if (count == 0)
    {
        printf("No endpoints found.\n");
    }

    // Each iteration prints the name of an endpoint device.
    PROPVARIANT varName;
    for (ULONG i = 0; i < count; i++)
    {
        // Get the pointer to endpoint number i.
        hr = pCollection->Item(i, &pEndpoint);
        EXIT_ON_ERROR(hr);

        // Get the endpoint ID string.
        hr = pEndpoint->GetId(&pwszID);
        EXIT_ON_ERROR(hr);

        hr = pEndpoint->OpenPropertyStore(
                STGM_READ, &pProps);
        EXIT_ON_ERROR(hr);

        // Initialize the container for property value.
        PropVariantInit(&varName);

        // Get the endpoint's friendly-name property.
        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
        EXIT_ON_ERROR(hr);

        //Print the endpoint friendly name and endpoint ID.
        //printf("Endpoint %d: \"%S\" (%S)\n", i, varName.pwszVal, pwszID);

        char test [200];
        sprintf(test, "%S", varName.pwszVal);

        deviceNamesList.push_back(test);

        //deviceNamesList.push_back(std::to_string(WideCharToMultiByte(CP_ACP,0,varName.pwszVal,-1,NULL,0, NULL, NULL)));

        CoTaskMemFree(pwszID);
        pwszID = NULL;
        PropVariantClear(&varName);
    }

    Exit:
    CoTaskMemFree(pwszID);
    pwszID = NULL;
    PropVariantClear(&varName);
    SAFE_RELEASE(pEnumerator);
    SAFE_RELEASE(pCollection);
    SAFE_RELEASE(pEndpoint);
    SAFE_RELEASE(pProps);

    return deviceNamesList;
}
#endif