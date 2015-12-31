cmake_minimum_required(VERSION 2.8.11)

cmake_policy(SET CMP0043 NEW)

project(FourierTransformPlugin)

set(CMAKE_INCUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

set(CMAKE_PREFIX_PATH $ENV{QT_DIR})

set(CMAKE_CXX_STANDARD 11)

find_package(Qt5Widgets)

include_directories(
    include
    external/kiss_fft
    ../../emdlib/include
    ../../emdpluginlib/include
)

add_library(fouriertransform SHARED
    src/FourierTransformPlugin.cpp
    src/FourierTransformModule.cpp
    include/FourierTransformPlugin.h
    include/FourierTransformModule.h
    external/kiss_fft/kiss_fft.c
    external/kiss_fft/kiss_fftnd.c
)

target_link_libraries(fouriertransform
    emdplugin
    emd
)

qt5_use_modules(fouriertransform Widgets)

#add_custom_command(TARGET fouriertransform POST_BUILD 
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "$<TARGET_FILE_DIR:emdViewer>/fouriertransform.dll"
 #       "$<TARGET_FILE_DIR:emdViewer>/plugins/fouriertransform.dll"
#) 

INSTALL(TARGETS fouriertransform
    RUNTIME DESTINATION bin/plugins
)    
