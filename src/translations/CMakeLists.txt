# Prevent deletion of .ts files if make clean is ran
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

#Generate list of translation files
file(GLOB TS_FILES *.ts)
qt5_create_translation(QM_FILES
    ${sources} ${TS_FILES})

add_custom_target(translations ALL DEPENDS ${QM_FILES})

# Move compiled translations out of source to keep things clean
add_custom_command(TARGET translations POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory
    "${CMAKE_BINARY_DIR}/appdata/translations"
    )
add_custom_command(TARGET translations POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy
    "*.qm"
    "${CMAKE_BINARY_DIR}/appdata/translations")

install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}/translations)

