#[[
    Cakewalk to Standard MIDI Files Command Line Utility Translator
    Copyright (C) 2021-2026, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

cmake_minimum_required(VERSION 3.16)

project(wrk2mid
  LANGUAGES CXX
  VERSION 1.2.2
  DESCRIPTION "Command line utility for translating WRK (Cakewalk) files into MID (standard MIDI files)"
  HOMEPAGE_URL "https://wrk2mid.sourceforge.io"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

#set(PROJECT_RELEASE_DATE "May 10, 2026")
option(BUILD_DOCS "Process Markdown sources of man pages and help files" ON)
option(USE_QT5 "Prefer building with Qt5 instead of Qt6" OFF)

if (USE_QT5)
    find_package(QT NAMES Qt5 REQUIRED)
    find_package(Qt5 5.15 COMPONENTS Core REQUIRED)
    unset(CORE5COMPAT_LIB)
else()
    find_package(QT NAMES Qt6 REQUIRED)
    find_package(Qt6 6.2 COMPONENTS Core Core5Compat  REQUIRED)
    get_target_property(CORE5COMPAT_LIB Qt6::Core5Compat IMPORTED_LOCATION)
    get_target_property(CORE_LIB Qt6::Core IMPORTED_LOCATION)
endif()

find_package(Drumstick 2.11 COMPONENTS File REQUIRED)

message (STATUS "Cakewalk to Standard MIDI File Translator v${PROJECT_VERSION}
     Install prefix: ${CMAKE_INSTALL_PREFIX}
     Build configuration: ${CMAKE_BUILD_TYPE}
     Operating System: ${CMAKE_SYSTEM_NAME}
     Processor: ${CMAKE_SYSTEM_PROCESSOR}
     Qt Version: ${QT_VERSION}
     Drumstick Version: ${Drumstick_VERSION}
     CMake Version: ${CMAKE_VERSION}
     Build docs: ${BUILD_DOCS}"
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
    Drumstick_VERSION=${Drumstick_VERSION}
)

if (NOT MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wno-deprecated-declarations
    )
endif()

target_link_libraries(${PROJECT_NAME}
  Qt${QT_VERSION_MAJOR}::Core
  Drumstick::File
)

if (QT_VERSION VERSION_GREATER_EQUAL 6.0)
    target_link_libraries(${PROJECT_NAME}
        Qt6::Core5Compat
    )
endif()

if (UNIX)
    include(GNUInstallDirs)
    install(TARGETS ${PROJECT_NAME}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
else()
    set(CMAKE_INSTALL_BINDIR ".")
    install(TARGETS ${PROJECT_NAME}
            RUNTIME DESTINATION ".")
endif()

if (UNIX)
    add_subdirectory(docs)

    install(FILES ${PROJECT_NAME}.png
            DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps/)

    install(FILES ${PROJECT_NAME}.desktop
            DESTINATION ${CMAKE_INSTALL_DATADIR}/applications/)

    install(FILES ${PROJECT_NAME}.metainfo.xml
            DESTINATION ${CMAKE_INSTALL_DATADIR}/metainfo/)

    configure_file(wrktest.in wrktest.sh IMMEDIATE @ONLY)
endif()

# uninstall target
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)
  add_custom_target ( uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

find_package(Git)
if ((NOT WIN32) AND Git_FOUND)
    find_program(GZIP gzip gzip.exe)
    find_program(BZIP2 bzip2 bzip2.exe)
    if ((EXISTS ${GZIP}) AND (EXISTS ${BZIP2}))
        set(ARCHNAME ${PROJECT_NAME}-${PROJECT_VERSION})
        add_custom_target ( tarballs
            COMMAND ${GIT_EXECUTABLE} archive --format=zip --prefix=${ARCHNAME}/ --output ${CMAKE_BINARY_DIR}/${ARCHNAME}.zip master
            COMMAND ${GIT_EXECUTABLE} archive --format=tar --prefix=${ARCHNAME}/ master | ${GZIP} > ${CMAKE_BINARY_DIR}/${ARCHNAME}.tar.gz
            COMMAND ${GIT_EXECUTABLE} archive --format=tar --prefix=${ARCHNAME}/ master | ${BZIP2} > ${CMAKE_BINARY_DIR}/${ARCHNAME}.tar.bz2
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Creating tarballs..."
            VERBATIM )
    else()
        message(WARNING "Missing utilities for tarball creation")
    endif()
endif()

if (QT_VERSION VERSION_GREATER_EQUAL 6.0)
    if (WIN32)
        set ( CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION "." )
        get_target_property(TARGET_CONFIG Drumstick::File IMPORTED_CONFIGURATIONS)
        get_target_property(DRUMSTICKFILE_LIB Drumstick::File IMPORTED_LOCATION_${TARGET_CONFIG})
        # message(STATUS "DRUMSTICKFILE_LIB: ${DRUMSTICKFILE_LIB}")
        # file( COPY ${DRUMSTICKFILE_DLL}
        #     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        set ( CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS
            ${DRUMSTICKFILE_LIB}
            ${CORE_LIB}
            ${CORE5COMPAT_LIB}
        )
        include(DeploymentUtils)
    elseif (UNIX AND NOT APPLE)
        qt_generate_deploy_app_script(
            TARGET ${PROJECT_NAME}
            OUTPUT_SCRIPT deploy_script
            NO_UNSUPPORTED_PLATFORM_ERROR
            NO_TRANSLATIONS
            NO_COMPILER_RUNTIME
            NO_PLUGINS
        )
        install(SCRIPT ${deploy_script})
        # AppImage
        install( CODE
          "include(${CMAKE_SOURCE_DIR}/cmake/CreateAppImage.cmake)
           create_appimage(
                EXE \"${PROJECT_NAME}\"
                ICON \"${PROJECT_NAME}.png\"
                DESKTOP \"${PROJECT_NAME}.desktop\"
                WORKDIR \"${CMAKE_CURRENT_BINARY_DIR}\"
                APPDIR \"${PROJECT_NAME}-v${PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}.AppDir\"
                ARCH \"${CMAKE_SYSTEM_PROCESSOR}\"
           )"
        )
    endif()
endif()

if (DEFINED ENV{GITHUB_OUTPUT})
    file(APPEND
        "$ENV{GITHUB_OUTPUT}"
        "config_version=${PROJECT_VERSION}\n"
    )
endif()

include(InstallRequiredSystemLibraries)
set (CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Cakewalk to Standard MIDI Files Translator")
set (CPACK_PACKAGE_VENDOR "wrk2mid.sourceforge.io")
set (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_INSTALL_DIRECTORY "${PROJECT_NAME}-${PROJECT_VERSION}")
set (CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME}" "WRK to SMF Translator")
set (CPACK_PACKAGE_VERSION "v${PROJECT_VERSION}")
set (CPACK_SOURCE_STRIP_FILES OFF)
set (CPACK_SOURCE_IGNORE_FILES "/.git/;/build/;.user;${CPACK_SOURCE_IGNORE_FILES}")

# Windows NSIS setup package
if (WIN32)
    set (CPACK_NSIS_DISPLAY_NAME "Cakewalk to MIDI Files Translator")
    set (CPACK_NSIS_WELCOME_TITLE_3LINES ON)
    set (CPACK_NSIS_FINISH_TITLE_3LINES ON)
    set (CPACK_NSIS_HELP_LINK "https://sourceforge.net/p/wrk2mid")
    set (CPACK_NSIS_URL_INFO_ABOUT "https://wrk2mid.sourceforge.io")
    set (CPACK_NSIS_CONTACT "plcl@users.sf.net")
    set (CPACK_NSIS_EXECUTABLES_DIRECTORY ".")
    set (CPACK_NSIS_MODIFY_PATH ON)
endif()

include(CPack)
