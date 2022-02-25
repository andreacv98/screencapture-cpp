# ScreenCapture C++
ScreenCapture is a project which permits to record your screen and your voice through your microphone.
It has been developed base don the FFMpeg libraries.
## Guide to usage
### Linux
* Pre-requirements instructions:
  ```bash
  sudo apt update && sudo apt install -y build-essential libpulse-dev libxau-dev libxdmcp-dev libxcb1-dev
  ```
    * Instructions to get the audio devices:
      ```bash
      pactl list short sources
      ```
### Windows
* Pre-requirements instructions:
    * MinGW 10
    * Pacman requirements:
      ```bash
      pacman -S mingw-w64-x86_64-dlfcn mingw-w64-x86_64-lame mingw-w64-x86_64-libvorbis mingw-w64-x86_64-liboggmingw-w64-x86_64-libiconv
      ```
    * Instructions to get the audio devices:
      ```bash
      cd ./libs/Windows/bin-video/
      ffmpeg.exe -f dshow -list_devices true -i dummy
      ```
### MacOS
* Pre-requirements instructions:
  ```bash
  brew install ffmpeg
  ```

### Usage - sample application
You can easily start the compilation and the usage just by running the CMakeList file with
a proper version of CMake and then follow the usual steps.
The sample application lets you record the screen with the audio or a portion of it,
just follow the instructions.

### Usage - library
The library start with the ```Controller``` object which needs at the creation step the audio device chosen (just look the
previous instructions) and the videoUrl which is different from each operating system (on Linux system, the library can
give you the name of the display just by calling the ```DisplayInfo.showInfos()``` method). Moreover, a SRSettings parameter
is requested, which is a simple structure which needs to be filled with all the information about the recording.
After all of this you can easily start the capture with ```startCapture()``` methods and the recording process should be fine
from that moment, just be aware of the possible exception that could be thrown by the method.
From that moment you can ```pauseCapture()```, ```resumeCapture()``` and ```stopCapture()```.
You can retrieve the file from the folder where your executable was running.

More details can be retrieved from [Doxigen documentation](src/libs/docs/html/index.html).