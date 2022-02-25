#ifndef VIDEO_APP_DISPLAYINFO_H
#define VIDEO_APP_DISPLAYINFO_H

#ifdef __unix__
#include <X11/Xlib.h>
#endif

class DisplayInfo{

public:
    DisplayInfo();
#ifdef __unix__
    Display *dpy;       //display from X11
#endif

void showInfos();

};


#endif //VIDEO_APP_DISPLAYINFO_H
