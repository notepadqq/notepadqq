# All of the UI files go here for compilation.
set (ui_sources
    mainwindow.ui
    frmpreferences.ui
    Search/dlgsearching.ui
    Search/frmsearchreplace.ui
    frmindentationmode.ui
    frmencodingchooser.ui
    frmlinenumberchooser.ui
    frmabout.ui
    Extensions/installextension.ui
)

set (include_sources
    include/mainwindow.h
    include/topeditorcontainer.h
    include/editortabwidget.h
    include/docengine.h
    include/frmabout.h
    include/notepadqq.h
    include/frmpreferences.h
    include/iconprovider.h
    include/EditorNS/editor.h
    include/EditorNS/bannerfilechanged.h
    include/EditorNS/bannerbasicmessage.h
    include/EditorNS/bannerfileremoved.h
    include/EditorNS/customqwebview.h
    include/clickablelabel.h
    include/frmencodingchooser.h
    include/EditorNS/bannerindentationdetected.h
    include/frmindentationmode.h
    include/singleapplication.h
    include/localcommunication.h
    include/Search/filesearchresultswidget.h
    include/Search/filesearchresult.h
    include/Search/frmsearchreplace.h
    include/Search/searchinfilesworker.h
    include/Search/replaceinfilesworker.h
    include/Search/searchhelpers.h
    include/Search/searchstring.h
    include/Search/dlgsearching.h
    include/Search/searchresultsitemdelegate.h
    include/Extensions/extension.h
    include/frmlinenumberchooser.h
    include/Extensions/extensionsserver.h
    include/Extensions/Stubs/stub.h
    include/Extensions/runtimesupport.h
    include/Extensions/Stubs/windowstub.h
    include/Extensions/Stubs/notepadqqstub.h
    include/Extensions/Stubs/editorstub.h
    include/Extensions/extensionsloader.h
    include/globals.h
    include/Extensions/Stubs/menuitemstub.h
    include/Extensions/installextension.h
    include/keygrabber.h
    include/Sessions/sessions.h
    include/Sessions/persistentcache.h
    include/nqqsettings.h
    include/nqqrun.h
)

set (cpp_sources
	main.cpp
    mainwindow.cpp
    topeditorcontainer.cpp
    editortabwidget.cpp
    docengine.cpp
    frmabout.cpp
    notepadqq.cpp
    frmpreferences.cpp
    iconprovider.cpp
    EditorNS/editor.cpp
    EditorNS/bannerfilechanged.cpp
    EditorNS/bannerbasicmessage.cpp
    EditorNS/bannerfileremoved.cpp
    EditorNS/customqwebview.cpp
    clickablelabel.cpp
    frmencodingchooser.cpp
    EditorNS/bannerindentationdetected.cpp
    frmindentationmode.cpp
    singleapplication.cpp
    localcommunication.cpp
    Search/filesearchresultswidget.cpp
    Search/frmsearchreplace.cpp
    Search/searchinfilesworker.cpp
    Search/searchstring.cpp
    Search/replaceinfilesworker.cpp
    Search/dlgsearching.cpp
    Search/searchresultsitemdelegate.cpp
    Extensions/extension.cpp
    frmlinenumberchooser.cpp
    Extensions/extensionsserver.cpp
    Extensions/Stubs/stub.cpp
    Extensions/runtimesupport.cpp
    Extensions/Stubs/windowstub.cpp
    Extensions/Stubs/notepadqqstub.cpp
    Extensions/Stubs/editorstub.cpp
    Extensions/extensionsloader.cpp
    globals.cpp
    Extensions/Stubs/menuitemstub.cpp
    Extensions/installextension.cpp
    keygrabber.cpp
    Sessions/sessions.cpp
    Sessions/persistentcache.cpp
    nqqsettings.cpp
    nqqrun.cpp
)

set(sources
    ${ui_sources}
    ${include_sources}
    ${cpp_sources}
)

#configure_file(resources.qrc ${CMAKE_CURRENT_BINARY_DIR}/resources.qrc COPYONLY)
file(COPY icons resources.qrc DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
add_executable(${PROJECT_NAME} ${sources} ${QM_FILES} ${CMAKE_CURRENT_BINARY_DIR}/resources.qrc)
add_dependencies(${PROJECT_NAME} translations)
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME ${PROJECT_NAME}-bin)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets Qt5::Core Qt5::WebKitWidgets Qt5::PrintSupport)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/${PROJECT_NAME})

