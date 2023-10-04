cmake_minimum_required(VERSION 3.5)

project(ZCamPfcs VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets LinguistTools Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Network)
set(TS_FILES ZCamPfcs_pt_BR.ts)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.h
        mainwindow.cpp
        mainwindow.ui
        zopenprotocol.h
        zopenprotocol.cpp
        zpfcsprotocol.h
        zpfcsprotocol.cpp
        zscrewdata.h
        zscrewdata.cpp
        zscrewdata.ui
        screwinfo.h
        zcampfcs.rc
        CAMreflected.ico
        ${TS_FILES}
)
set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/zcampfcs.rc")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ZCamPfcs
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ZCamPfcs APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation

    qt_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    if(ANDROID)
        add_library(ZCamPfcs SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(ZCamPfcs
            ${PROJECT_SOURCES}
            ${APP_ICON_RESOURCE_WINDOWS}
            zcampfcs.qrc
        )
    endif()

    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
endif()

target_link_libraries(ZCamPfcs PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(ZCamPfcs PRIVATE Qt${QT_VERSION_MAJOR}::Network)

set_target_properties(ZCamPfcs PROPERTIES
    WIN32_EXECUTABLE TRUE
)

install(TARGETS ZCamPfcs
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ZCamPfcs)
endif()
