#include "DisplayInfo.h"
#include <iostream>

DisplayInfo::DisplayInfo():dpy(nullptr) {
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

void DisplayInfo::showInfos() {
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

