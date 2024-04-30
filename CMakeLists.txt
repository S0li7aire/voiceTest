cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(Main VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O1")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(Q_BUILD_TEST OFF)
option(Q_BUILD_EXAMPLES OFF)

#Qt5
# find_package(FFTW3 REQUIRED)
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
        src/audioRecorder.cpp
        src/audioRecorder.h
        src/WavFile.h
        src/WavFile.cpp
        src/FFTGraph.h
        src/FFTGraph.cpp
        src/FFTGraph.ui
        src/qcustomplot.cpp
        src/qcustomplot.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Main
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
endif()

set_source_files_properties(src/qcustomplot.cpp PROPERTIES COMPILE_FLAGS "-O2")

# find_library(FFTW_LIBRARY fftw3)  # Assuming fftw3 is the library name
# find_path(FFTW_INCLUDE_DIR fftw3.h)  # Assuming fftw3.h is the header file
# include_directories(${FFTW_INCLUDE_DIR})
set(FFTW_DIR ${CMAKE_SOURCE_DIR}/FFTW)
target_link_libraries(Main PRIVATE ${FFTW_DIR}/libfftw3-3.dll ${FFTW_DIR}/libfftw3f-3.dll ${FFTW_DIR}/libfftw3l-3.dll)
target_include_directories(${PROJECT_NAME} PUBLIC ${FFTW_DIR})


target_link_libraries(Main PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::PrintSupport m)
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
