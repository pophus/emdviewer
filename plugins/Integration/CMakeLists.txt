cmake_minimum_required(VERSION 2.8.11)

cmake_policy(SET CMP0043 NEW)

project(IntegrationPlugin)

set(CMAKE_INCUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

set(CMAKE_PREFIX_PATH $ENV{QT_DIR})

set(CMAKE_CXX_STANDARD 11)

find_package(Qt5Widgets)

include_directories(
    include
    ../../emdlib/include
    ../../emdpluginlib/include
)

add_library(integration SHARED
    src/IntegrationPlugin.cpp
    src/IntegrationModule.cpp
    include/IntegrationPlugin.h
    include/IntegrationModule.h
)

target_link_libraries(integration
    emdplugin
    emd
)

qt5_use_modules(integration Widgets)

#add_custom_command(TARGET integration POST_BUILD 
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        "$<TARGET_FILE_DIR:emdViewer>/integration.dll"
#        "$<TARGET_FILE_DIR:emdViewer>/plugins/integration.dll"
#)     

INSTALL(TARGETS integration
    RUNTIME DESTINATION bin/plugins
)
