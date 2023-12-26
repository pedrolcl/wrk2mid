# wrk2mid: Cakewalk to Standard MIDI Files Translator

wrk2mid is a command line utility for translating WRK (Cakewalk) files into MID (standard MIDI files) based on [Drumstick](https://drumstick.sourceforge.io/docs/index.html) (a set of GPLv3 licensed C++/Qt libraries for MIDI applications).

License: GPLv3

```
Usage: wrk2mid [options] file
Command line utility for translating WRK (Cakewalk) files into MID (standard MIDI files)

Options:
  -h, --help             Displays help on commandline options.
  --help-all             Displays help including Qt specific options.
  -v, --version          Displays version information.
  -f, --format <format>  SMF Format (0/1)
  -o, --output <output>  Output file name
  -t, --test             Test only (no output)

Arguments:
  file                   Input WRK File Name
```

## Building

Minimum requirements:

* C++17 compiler
* [Qt6 or Qt5](https://www.qt.io/download)
* [Drumstick 2.9](https://sourceforge.net/projects/drumstick/)
* [pandoc](https://pandoc.org/) (optional, if BUILD_DOCS)
* [CMake 3.16](https://cmake.org/)

### Build and deployment commands (for Linux)

```sh
    tar xvf wrk2mid-x.y.z.tar.gz
    cd wrk2mid-x.y.z
    mkdir build
    cmake -S . -B build -DCMAKE_PREFIX_PATH="$HOME/Qt5;$HOME/drumstick2"
    cmake --build build
    cmake --install build
```

You need to replace the contents of the parameter CMAKE_PREFIX_PATH with the actual paths in your system. There are precompiled packages at Sourceforge.
See the [CMake documentation](https://cmake.org/cmake/help/latest/guide/user-interaction/index.html#introduction) for details.

You may use Qt6 or Qt5 to build this program. If you prefer Qt5, then you should include in the cmake command line the argument USE_QT5=ON

### Packaging notes

This program is not a GUI application, obviously. It is a command line application. The reason why there is a `wrk2mid.desktop` file is because it is required to build an AppImage. If you are building another type of distribution package, you probably should omit this file.

## Downloads

[![Download wrk2mid](https://a.fsdn.com/con/app/sf-download-button)](https://sourceforge.net/projects/wrk2mid/files/latest/download)

https://sourceforge.net/projects/wrk2mid/files/

### Git repository

[Project page at SourceForge](https://sourceforge.net/projects/wrk2mid/)

[Mirror at GitHub](https://github.com/pedrolcl/wrk2mid)
