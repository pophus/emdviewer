cmake_minimum_required(VERSION 3.1.0)

project(emdviewer)

set(CMAKE_INCUDE_CURRENT_DIR ON)

# To auto-run the Qt MOC
set(CMAKE_AUTOMOC ON)

IF (WIN32)
    string(REPLACE "\\" "/" QT_DIR "$ENV{QT_DIR}")
    string(REPLACE "\\" "/" HDF5_ROOT_DIR_HINT "$ENV{HDF5_ROOT_DIR_HINT}")
ENDIF (WIN32)

set(CMAKE_PREFIX_PATH
    ${QT_DIR}
    ${HDF5_ROOT_DIR_HINT}
)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/install")

set(emdviewer_VERSION_MAJOR "0")
set(emdviewer_VERSION_MINOR "4")
set(emdviewer_VERSION_PATCH "0")

set(emdviewer_VERSION "${emdviewer_VERSION_MAJOR}.${emdviewer_VERSION_MINOR}.${emdviewer_VERSION_PATCH}")

IF (APPLE)
    # Add this path so that cmake can find the MacOSXBundleInfo.plist.in file.
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/platform/osx")

    set(MACOSX_BUNDLE_INFO_STRING "emdviewer ${emdviewer_VERSION}")

ENDIF (APPLE)

IF (WIN32)
    set (CMAKE_LIBRARY_PATH “C:/Program Files (x86)/Windows Kits/8.0/Lib/win8/um/x64”) 
ENDIF (WIN32)

find_package(Qt5Widgets)

add_subdirectory(emdlib)
add_subdirectory(emdpluginlib)
add_subdirectory(plugins)

add_subdirectory(include)
add_subdirectory(src)

include_directories(
    include
    emdlib/include
    emdpluginlib/include
)

qt5_add_resources(RESOURCE res/res.qrc)

add_executable(emdViewer MACOSX_BUNDLE WIN32
    ${EMDVIEWER_SOURCES}
    ${EMDVIEWER_HEADERS}
    ${RESOURCE}
)

target_link_libraries(emdViewer
    emd
    emdplugin
)

qt5_use_modules(emdViewer Widgets)

add_custom_command(TARGET emdViewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/workflows $<TARGET_FILE_DIR:emdViewer>/workflows
)

IF (APPLE)
    set(INSTALL_APP "${CMAKE_INSTALL_PREFIX}/bin/emdViewer.app/Contents/MacOs/emdviewer")
    set(BUILD_APP "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/\${CMAKE_INSTALL_CONFIG_NAME}/emdViewer.app/Contents/MacOs/emdviewer")

    set_target_properties(emdViewer PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/platform/osx/emdViewer.plist.in
        MACOSX_BUNDLE true
        MACOSX_BUNDLE_BUNDLE_NAME emdViewer
        MACOSX_BUNDLE_INFO_STRING "emdViewer beta ${emdviewer_VERSION}"
        MACOSX_BUNDLE_LONG_VERSION_STRING "${emdviewer_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${emdviewer_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${emdviewer_VERSION}"
        #MACOSX_BUNDLE_ICON_FILE xrick.icns
    )
ELSE (WIN32)
    set(INSTALL_APP "${CMAKE_INSTALL_PREFIX}/bin/emdViewer.exe")
    set(BUILD_APP "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/\${CMAKE_INSTALL_CONFIG_NAME}/emdViewer.exe")
ENDIF (APPLE)

INSTALL(TARGETS emdViewer
    RUNTIME
    DESTINATION bin
    COMPONENT applications
)

SET(DIRS)

list(APPEND DIRS "${QT_DIR}/bin")
list(APPEND DIRS "${HDF5_ROOT_DIR_HINT}/bin")

#add_custom_command(TARGET emdViewer POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E fixup_bundle(\"$<TARGET_FILE:emdViewer>\"   \"\"   \"${DIRS}\")
#)

INSTALL(DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/\${CMAKE_INSTALL_CONFIG_NAME}/"
    DESTINATION bin
    COMPONENT dependencies
    FILES_MATCHING PATTERN "emd*.dll"
)

INSTALL(CODE "
    include(BundleUtilities)
    fixup_bundle(\"${INSTALL_APP}\"   \"\"   \"${DIRS}\")
")

INSTALL(DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/workflows"
    DESTINATION bin
    COMPONENT workflows
)

INSTALL(FILES "${CMAKE_CURRENT_LIST_DIR}/res/colourmaps.xml"
    DESTINATION bin
    COMPONENT resources
)

INSTALL(DIRECTORY "${CMAKE_INSTALL_PREFIX}/bin/"
    DESTINATION bin
    COMPONENT dependencies
    FILES_MATCHING PATTERN "*.dll"
)

IF (WIN32)
    INSTALL(FILES "${QT_DIR}/plugins/platforms/qwindows.dll"
        DESTINATION bin/platforms
        COMPONENT dependencies
    )
ENDIF()

INCLUDE(InstallRequiredSystemLibraries)

SET(CPACK_INSTALL_CMAKE_PROJECTS "${CMAKE_CURRENT_BINARY_DIR}" "emdviewer" "ALL" "/")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "emdViewer Installer")
SET(CPACK_PACKAGE_VENDOR "Mesozoic Binaries")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
SET(CPACK_PACKAGE_VERSION_MAJOR "${emdviewer_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${emdviewer_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${emdviewer_VERSION_PATCH}")
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "emdViewer")
IF(WIN32 AND NOT UNIX)
  # There is a bug in NSI that does not handle full unix paths properly. Make
  # sure there is at least one set of four (4) backslashes.
  #SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/Utilities/Release\\\\InstallIcon.bmp")
  SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\emdViewer-${emdviewer_VERSION}.exe")
  SET(CPACK_NSIS_DISPLAY_NAME "emdViewer")
  SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\emdatasets.lbl.gov")
  SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\emdatasets.lbl.gov")
  SET(CPACK_NSIS_CONTACT "clophus@lbl.gov")
  SET(CPACK_NSIS_MODIFY_PATH ON)
ELSE(WIN32 AND NOT UNIX)
  #SET(CPACK_STRIP_FILES "bin/MyExecutable")
  #SET(CPACK_SOURCE_STRIP_FILES "")
ENDIF(WIN32 AND NOT UNIX)

SET(CPACK_PACKAGE_EXECUTABLES "emdViewer" "emdViewer")

INCLUDE(CPack)
