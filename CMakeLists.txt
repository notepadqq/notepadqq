cmake_minimum_required (VERSION 2.8.12)
project (notepadqq)

# Automation for QT 
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set our output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

#Find QT packages required for build
find_package(Qt5Core REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5WebKitWidgets REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5LinguistTools REQUIRED)

# Initialize git submodules if git is available
find_package(Git)
if(GIT_FOUND)
    message("git found: ${GIT_EXECUTABLE}")
    if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
        message("Initializing submodules.")
        execute_process(COMMAND "${GIT_EXECUTABLE}" submodule init)
        execute_process(COMMAND "${GIT_EXECUTABLE}" submodule update)
    endif()
endif()


set (cmdir "${CMAKE_SOURCE_DIR}/src/editor/libs/codemirror")
set (cmerror "Error: CodeMirror submodule not available at '${cmdir}'! Make sure you downloaded the source code using\
 git as specified in README.md.")

file(GLOB cmresult ${cmdir})
list(LENGTH cmresult cmempty)
if(NOT EXISTS ${cmdir} OR cmempty EQUAL 0)
    message(FATAL_ERROR ${cmerror})
endif()

# Installation instructions
install(DIRECTORY support_files/icons/ DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons)
install(DIRECTORY ${CMAKE_BINARY_DIR}/appdata/translations src/editor src/extension_tools DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME})
install(FILES images/notepadqq.svg DESTINATION ${CMAKE_INSTALL_PREFIX}/share/pixmaps)
install(FILES support_files/shortcuts/notepadqq.desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)
install(FILES support_files/manpage/notepadqq.1 DESTINATION ${CMAKE_INSTALL_PREFIX}/share/man/man1)
install(FILES CONTRIBUTORS.md CONTRIBUTING.md DESTINATION ${CMAKE_INSTALL_PREFIX}/share/doc/${PROJECT_NAME})
#Set some permissions since the "binary" is actually a script
install(FILES support_files/launch/notepadqq PERMISSIONS 
    OWNER_EXECUTE 
    OWNER_READ 
    OWNER_WRITE 
    GROUP_READ 
    GROUP_EXECUTE
    WORLD_READ 
    WORLD_EXECUTE 
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

# Copy instructions which allow in-source runtime
file(COPY src/editor src/extension_tools DESTINATION ${CMAKE_BINARY_DIR}/appdata)
file(COPY support_files/launch/notepadqq DESTINATION ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)
