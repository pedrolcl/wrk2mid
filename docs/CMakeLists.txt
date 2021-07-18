include(pandoc_manpage)
if (EXISTS ${PANDOC_EXECUTABLE})
    set(MANPAGE ${PROJECT_NAME}.1)
    add_manpage(${MANPAGE})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${MANPAGE}
            DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
endif()
