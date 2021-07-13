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

Arguments:
  file                   Input WRK File Name
```

## Building

Minimum requirements:

* C++11 compiler
* [Qt5 or Qt6](https://www.qt.io/download)
* [Drumstick 2.3](https://sourceforge.net/projects/drumstick/)
* [pandoc](https://pandoc.org/)
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
You need to replace the contents of the parameter CMAKE_PREFIX_PATH with the actual paths in your system. There are x86_64 precompiled packages for Linux, Windows and macOS at Sourceforge.

## Downloads

[![Download wrk2mid](https://a.fsdn.com/con/app/sf-download-button)](https://sourceforge.net/projects/wrk2mid/files/latest/download)

### Git repository

[Project page at SourceForge](https://sourceforge.net/projects/wrk2mid/)

[Mirror at GitHub](https://github.com/pedrolcl/wrk2mid)
