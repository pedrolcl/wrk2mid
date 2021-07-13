#[[
    Cakewalk to Standard MIDI Files Command Line Utility Translator
    Copyright (C) 2021, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
#]]

cmake_minimum_required(VERSION 3.14)

project(wrk2mid
  LANGUAGES CXX
  VERSION 1.0.0
  DESCRIPTION "Command line utility for translating WRK (Cakewalk) files into MID (standard MIDI files)"
  HOMEPAGE_URL "https://sourceforge.net/p/wrk2mid/"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
find_package(Drumstick 2.3 COMPONENTS File REQUIRED)

message (STATUS "Cakewalk to Standard MIDI File Translator v${PROJECT_VERSION}
     install prefix: ${CMAKE_INSTALL_PREFIX}
     Build configuration: ${CMAKE_BUILD_TYPE}
     Operating System: ${CMAKE_SYSTEM_NAME}
     Processor: ${CMAKE_SYSTEM_PROCESSOR}
     Qt Version: ${QT_VERSION}
     Drumstick Version: ${Drumstick_VERSION}"
)

add_executable(${PROJECT_NAME}
  events.cpp
  events.h
  main.cpp
  sequence.cpp
  sequence.h
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    VERSION=${PROJECT_VERSION}
)

target_link_libraries(${PROJECT_NAME}
  Qt${QT_VERSION_MAJOR}::Core
  Drumstick::File
)

include(GNUInstallDirs)
add_subdirectory(docs)
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
