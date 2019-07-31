link_libraries(gfx)

if (WIN32)
    set(ICON src/game.rc)
elseif (APPLE)
    set(ICON src/game.icns)
    set_source_files_properties(src/game.icns PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")
endif()

add_executable(computeTest WIN32 MACOSX_BUNDLE src/computeTest.cpp ${ICON})

configure_file(src/ptnc.vert ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(src/xyzuvn.vert ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(src/solid.frag ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(src/textured.frag ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(src/gentex.comp ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(src/gendraw.comp ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)
configure_file(src/gengrid.comp ${CMAKE_CURRENT_BINARY_DIR} COPYONLY)

set(BINARIES computeTest)

set_target_properties(${BINARIES} PROPERTIES FOLDER "apps")

if (MSVC)
    # Tell MSVC to use main instead of WinMain for Windows subsystem executables
    set_target_properties(${BINARIES} PROPERTIES
                          LINK_FLAGS "/ENTRY:mainCRTStartup /SUBSYSTEM:CONSOLE")
endif()

if (APPLE)
    set_target_properties(computeTest PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "ComputeTest")

    set_target_properties(${BINARIES} PROPERTIES
                          RESOURCE src/game.icns
                          MACOSX_BUNDLE_SHORT_VERSION_STRING ${GLFW_VERSION}
                          MACOSX_BUNDLE_LONG_VERSION_STRING ${GLFW_VERSION_FULL}
                          MACOSX_BUNDLE_ICON_FILE src/game.icns
                          MACOSX_BUNDLE_INFO_PLIST "${GLFW_SOURCE_DIR}/CMake/MacOSXBundleInfo.plist.in")
endif()

