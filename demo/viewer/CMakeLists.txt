set(CMAKE_PREFIX_PATH "/home/fd/lidj/qt-5.12.10-static")
#find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
#find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
# 指定Qt的路径（替换为您的Qt静态库路径）
#set(QT_DIR "/home/fd/lidj/qt-5.12.10-static/lib/cmake/Qt5")
#set(Qt5Core_DIR "/home/fd/lidj/qt-5.12.10-static/lib/cmake/Qt5Core")
#set(Qt5Gui_DIR "/home/fd/lidj/qt-5.12.10-static/lib/cmake/Qt5Gui")
#set(Qt5Widgets_DIR "/home/fd/lidj/qt-5.12.10-static/lib/cmake/Qt5Widgets")
#set(Qt5_DIR"/home/fd/lidj/qt-5.12.10-static/lib/cmake/Qt5")

# 查找包含静态库的Qt模块
#find_package(Qt5Widgets REQUIRED STATIC)
#find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS Widgets REQUIRED)
#set(CMAKE_INSTALL_RPATH "$ORIGIN/../depends")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(CMAKE_EXE_LINKER_FLAGS "-static")

set(PROJECT_SOURCES
    about_dialog.cpp
    info_dialog.cpp
    layerdelegate.cpp
    layerlistmodel.cpp
    layerwidget.cpp
    layer_info_dialog.cpp
    main.cpp
    mainwindow.cpp
    mapwidget.cpp
    styles_model.cpp
    forms/about.ui
    forms/info.ui
    forms/layer_info.ui    
    mapnik_viewer.qrc
    roadmerger.h
    roadmerger.cpp
    waitingspinnerwidget.h
    waitingspinnerwidget.cpp
    cehuidatainfo.hpp
    completeRoadsWidget.hpp
    completeRoadsWidget.cpp
    groupinfo.hpp
    treewidget.hpp
)

#if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    #qt_add_executable(mapnik-viewer
   #     MANUAL_FINALIZATION
  #      ${PROJECT_SOURCES}
 #   )
#else()
    if(ANDROID)
        add_library(mapnik-viewer SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(mapnik-viewer
            ${PROJECT_SOURCES}
        )
    endif()
#endif()

set_target_properties(mapnik-viewer PROPERTIES
    AUTOUIC_SEARCH_PATHS forms
    AUTORCC ON    
    AUTOUIC ON
    AUTOMOC ON
    LIBRARY_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}"
    ARCHIVE_OUTPUT_DIRECTORY "${MAPNIK_OUTPUT_DIR}/lib"
)

target_link_libraries(mapnik-viewer PRIVATE 
    Qt${QT_VERSION_MAJOR}::Widgets
    mapnik::agg
    mapnik::mapnik
)

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/viewer.ini 
"[mapnik]
plugins_dir=${PLUGINS_INSTALL_DIR}
fonts/1/dir=${FONTS_INSTALL_DIR}"
)

#if(QT_VERSION_MAJOR EQUAL 6)
   # qt_finalize_executable(mapnik-viewer)
#endif()

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/viewer.ini DESTINATION bin)
mapnik_install_utility(mapnik-viewer)
