% WRK2MID(1) wrk2mid 0.0.0 | Cakewalk to Standard MIDI File Translator
% Pedro López-Cabanillas <plcl@users.sf.net>

# NAME

**wrk2mid** — Cakewalk to Standard MIDI File Translator

# SYNOPSIS

| **wrk2mid** \[**-o**|**--output** _output_file_] [**-f**|**--format** _format_] \[_input_file_]
| **wrk2mid** \[**-h**|**--help**|**--help-all**|**-v**|**--version**]

# DESCRIPTION

This program is a command line multiplatform converter utility from Cakewalk files to SMF.
It reads .WRK (Cakewalk files), and outputs .MID (Standard MIDI files).

## Options

-h, --help

:   Prints brief usage information.

-v, --version

:   Prints the current version number.

-f, --format _format_

:   Output SMF format (0/1).
  
-o, --output _output_file_

:   Output file name. By default is the same name as the input file, replacing .WRK with a .MID suffix.

## Arguments

_input_file_

:   Input WRK (Cakewalk) file name.

# BUGS

See Tickets at Sourceforge <https://sourceforge.net/p/wrk2mid/tickets/> and GitHub <https://github.com/pedrolcl/wrk2mid/issues/>

# SEE ALSO

**qt5options (7)**
