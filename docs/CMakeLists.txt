if(NOT EXISTS ${PANDOC_EXECUTABLE})
    find_program(PANDOC_EXECUTABLE pandoc)
    mark_as_advanced(PANDOC_EXECUTABLE)
    if(NOT EXISTS ${PANDOC_EXECUTABLE})
        message(FATAL_ERROR "Pandoc not found. Install Pandoc or set the cache variable PANDOC_EXECUTABLE.")
    endif()
endif()

if (EXISTS ${PANDOC_EXECUTABLE})
    macro(add_manpage out_file)
        set(_src  ${CMAKE_CURRENT_SOURCE_DIR}/${out_file}.md)
        unset(_date)
        execute_process (
            COMMAND bash -c "LANG=C;date +'%B %d, %Y'"
            OUTPUT_VARIABLE _date
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
#       set(_date "Jul 31, 2021")
        set(_footer "${PROJECT_NAME} ${PROJECT_VERSION}")
        add_custom_command (
            OUTPUT ${out_file}
            COMMAND ${PANDOC_EXECUTABLE} -s -t man -Vdate=${_date} -Vfooter=${_footer} ${_src} -o ${out_file}
            DEPENDS ${_src}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            VERBATIM
        )
        add_custom_target(manpages ALL DEPENDS ${out_file})
    endmacro()
    set(MANPAGE ${PROJECT_NAME}.1)
    add_manpage(${MANPAGE})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${MANPAGE}
            DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
endif()
