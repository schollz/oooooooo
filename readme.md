# oooooooo

[![build workflow](https://github.com/schollz/oooooooo/actions/workflows/ci.yml/badge.svg)](https://github.com/schollz/oooooooo/actions/workflows/ci.yml) [![GitHub Release](https://img.shields.io/github/v/release/schollz/oooooooo)](https://github.com/schollz/oooooooo/releases/latest)

![output](https://github.com/user-attachments/assets/1ea7dadb-491c-4786-85e0-29a07b8a003e)


### introduction


i call this script "oooooooo" because it is composed of eight loops. they are like digital tape loops - each loop can be can leveled, paned, slowed, sped up, shortened, lengthened, overdubed, destroyed, warbled. 

this is a fork of [softcut JACK client](https://github.com/monome/softcut-lib/) from [monome](https://monome.org), originally for use with the [norns](https://monome.org/norns/) sound computer. I've forked it to port a norns script I wrote, [oooooo](https://github.com/schollz/oooooo), to be available as a standalone cross-platform application. 

## Quickstart

This program requires JACK to be running. Make sure to first [download and install JACK](https://jackaudio.org/downloads/). Then you can use the `qjackctl` GUI to start JACK, or you can start it from the command line.

### Windows

Download [oooooooo_v1.0.0.zip](https://github.com/schollz/oooooooo/releases/download/v1.0.0/oooooooo_v1.0.0.zip). Unzip it and then run the `oooooooo.exe` program in the resulting directory.

### Linux

Download [oooooooo_v1.0.0.AppImage](https://github.com/schollz/oooooooo/releases/download/v1.0.0/oooooooo_v1.0.0.AppImage) and then run the following commands:

```bash
chmod +x oooooooo_v1.0.0.AppImage
./oooooooo_v1.0.0.AppImage
```

### macOS

Use brew to install:

```bash
brew tap schollz/tap
brew install oooooooo
```

Then run the following command:

```bash
oooooooo
```

## Help

### Basics

The following keys are available to control the program:

- `h` toggle help menu
- `tab` toggles parameter menu
- `1-8` selects loop
- `up/down` selects parameter
- `left/right` adjusts parameter


### Playing/Recording

- `p` toggles play
- `r` toggles record
- `ctrl + r` toggles record once
- `shift + r` toggles priming of recording

### LFOs

- `l` toggles lfo
- `+/-` adjusts lfo peiod
- `ctl+left/right` adjusts *min* lfo
- `alt+left/right` adjusts *max* lfo


### Saving/Loading

- drag and drop audio files to load
- `s` saves parameters
- `ctrl+s` saves audio
- `o` loads parameters
- `ctrl+o` loads audio
- `f` will save audio output to a file (not implemented)

## Development

This project is dynamically linked against several other libraries. You will need to install the depenndencies before you can build this project.

### Pre-requisites

#### Linux

First install [Jack](https://jackaudio.org/downloads/#linux).

Then install the dependencies:

```bash
sudo apt-get install  libjack-jackd2-0 liblo7 libsdl2-2.0-0 libsdl2-ttf-2.0-0 librtmidi6 libsndfile1 flac
```

#### macOS

First install [Jack](https://jackaudio.org/downloads/#macos).

Then install the dependencies:

```bash
brew install jack liblo sdl2 sdl2_ttf rtmidi libsndfile flac
```

#### Windows

First install [Jack](https://jackaudio.org/downloads/#windows).

Then install the dependencies by first downloading and installing [MSYS2](https://www.msys2.org/).

```powershell
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-pkg-config mingw-w64-x86_64-python mingw-w64-x86_64-jack2 mingw-w64-x86_64-liblo mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf mingw-w64-x86_64-rtmidi mingw-w64-x86_64-libsndfile
```

Then, before building, you need to set the environment variables for the MSYS2 shell. You can do this by running the following command in the MSYS2 shell:

```powershell
$env:PATH += ";C:\msys64\mingw64\bin"
```

or if your in MSYS2:

```bash
export PATH=/mingw64/bin:$PATH
export PKG_CONFIG_PATH="/mingw64/lib/pkgconfig:$PKG_CONFIG_PATH"
export MINGW_PREFIX="/mingw64"
rm -rf build && mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=/mingw64 -G "Unix Makefiles" .. && cmake --build . --config Release -- -j$(nproc)
```


### Build

Once the dependencies are installed, you can build the project.

```
mkdir build 
cd build 
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release -- -j$(nproc)
```

# License

[softcut](https://github.com/monome/softcut-lib/) is licensed under the GPLv3 license, Copyright (c) monome.

Compressor code is licensed under the LGPL V2.1, Copyright (c) 2023 Electrosmith, Corp, GRAME, Centre National de Creation Musicale.

Reverb code is licensed under BSD-2-Clause, Copyright (c) Jean Pierre Cimalando.

Monaspace Argon font is licensed under the SIL Open Font License, Version 1.1, Copyright (c) 2023, GitHub.
