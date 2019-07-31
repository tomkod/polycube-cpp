link_libraries(glfw)
link_libraries(utils)

if (MATH_LIBRARY)
    link_libraries("${MATH_LIBRARY}")
endif()

if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if (GLFW_USE_OSMESA)
    add_definitions(-DUSE_NATIVE_OSMESA)
endif()

set(GLAD "${GLFW_SOURCE_DIR}/deps/glad/glad.h"
         "${GLFW_SOURCE_DIR}/deps/glad.c")

set(GFX_SOURCES
    src/glHelpers.cpp
    include/glHelpers.h)

add_library(gfx ${GFX_SOURCES} ${GLAD})

target_include_directories(gfx
    PUBLIC "include"
	PUBLIC ${utils_INCLUDE_DIRS}
	PUBLIC ${glfw_INCLUDE_DIRS}
	PUBLIC "${GLFW_SOURCE_DIR}/deps")
