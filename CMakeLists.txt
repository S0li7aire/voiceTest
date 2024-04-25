cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(Main VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Qt5
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets PrintSupport)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets PrintSupport)

set(PROJECT_SOURCES
        src/main.cpp
        src/MainWin.cpp
        src/MainWin.h
        src/MainWin.ui
        src/LoginUi.h
        src/LoginUi.cpp
        src/LoginUi.ui
        src/AudioDeviceSelector.h
        src/AudioDeviceSelector.cpp
        src/AudioDeviceSelector.ui
        src/WavFile.h
        src/WavFile.cpp
        src/qcustomplot.cpp
        src/qcustomplot.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Main
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(Main PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::PrintSupport)

include(FetchContent)

FetchContent_Declare(
  libqio
  GIT_REPOSITORY https://github.com/cycfi/Q.git
  GIT_BRANCH develop
  GIT_SHALLOW TRUE
  GIT_PROGRESS ON
  GIT_SUBMODULES infra q_io/external/portaudio q_io/external/portmidi
)

set(FETCHCONTENT_QUIET off)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${libqio_SOURCE_DIR}/infra/include
    ${libqio_SOURCE_DIR}/q_io/include
    ${libqio_SOURCE_DIR}/q_io/external/include
    ${libqio_SOURCE_DIR}/q_lib/include)

FetchContent_MakeAvailable(libqio)
target_link_libraries(${PROJECT_NAME} PRIVATE libqio)

include(GNUInstallDirs)
install(TARGETS Main
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Main)
endif()
