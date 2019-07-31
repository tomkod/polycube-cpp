set(UTILS_SOURCES
    src/glslMathTest.cpp
    src/perlinNoise.cpp
    include/glslMath.h
    include/perlinNoise.h)

add_library(utils ${UTILS_SOURCES})
target_include_directories(utils PUBLIC "include")
