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

## Changes for v1.1.0

* New build option: BUILD_DOCS.
* New Build option: USE_QT to choose between Qt5 and Qt6. Closes ticket #3.
* Convert WRK track Port parameter. Closes ticket #2.
* Convert WRK markers into SMF text markers. Closes ticket #1.
* Displayed compiled and runtime library version information.

## Building

Minimum requirements:

* C++11 compiler
* [Qt5 or Qt6](https://www.qt.io/download)
* [Drumstick 2.5](https://sourceforge.net/projects/drumstick/)
* [pandoc](https://pandoc.org/) (optional, if BUILD_DOCS)
* [CMake 3.14](https://cmake.org/)

### Build and deployment commands (for Linux)

```
$ tar -xvzf wrk2mid-x.y.z.tar.gz
$ cd wrk2mid-x.y.z
$ mkdir build
$ cd build
$ cmake .. -DCMAKE_PREFIX_PATH="$HOME/Qt5;$HOME/drumstick2"
$ make
$ make install
```

You need to replace the contents of the parameter CMAKE_PREFIX_PATH with the actual paths in your system. There are precompiled packages at Sourceforge.
See the [CMake documentation](https://cmake.org/cmake/help/latest/guide/user-interaction/index.html#introduction) for details.

You may use Qt5 or Qt6 to build this program. If you have both versions installed system wide, then you should include in the cmake command line the argument USE_QT=5 or USE_QT=6.

### Packaging notes

This program is not a GUI application, obviously. It is a command line application. The reason why there is a `wrk2mid.desktop` file is because it is required to build an AppImage. If you are building another type of distribution package, you probably should omit this file.

## Downloads

[![Download wrk2mid](https://a.fsdn.com/con/app/sf-download-button)](https://sourceforge.net/projects/wrk2mid/files/latest/download)

https://sourceforge.net/projects/wrk2mid/files/v1.1.0/

### Git repository

[Project page at SourceForge](https://sourceforge.net/projects/wrk2mid/)

[Mirror at GitHub](https://github.com/pedrolcl/wrk2mid)
