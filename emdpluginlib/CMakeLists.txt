cmake_minimum_required(VERSION 2.8.11)

project(emdpluginlib)

cmake_policy(SET CMP0042 NEW)
cmake_policy(SET CMP0043 NEW)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

set(CMAKE_PREFIX_PATH $ENV{QT_DIR})

set(CMAKE_CXX_STANDARD 11)

find_package(Qt5Widgets)
find_package(Qt5Xml)

add_subdirectory(include)
add_subdirectory(src)

include_directories(
    include
    ../emdlib/include
)

add_library(emdplugin SHARED
    ${EMDPLUGINLIB_SOURCES}
    ${EMDPLUGINLIB_HEADERS}
)

target_link_libraries(emdplugin
    emd
)

target_compile_definitions(emdplugin PRIVATE BUILD_EMDPLUGINLIB=1)

qt5_use_modules(emdplugin Widgets Xml)

install(TARGETS emdplugin
    RUNTIME DESTINATION bin
    COMPONENT dependencies
)
