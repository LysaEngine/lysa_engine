#
# Copyright (c) 2024-2025 Henri Michelon
#
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT
#
########################################################################################################################
message(NOTICE "Fetching Jolt Physics..")
# https://github.com/jrouwe/JoltPhysicsHelloWorld/blob/main/Build/CMakeLists.txt
# When turning this option on, the library will be compiled using doubles for positions. This allows for much bigger worlds.
set(DOUBLE_PRECISION OFF)
# When turning this option on, the library will be compiled with debug symbols
set(GENERATE_DEBUG_SYMBOLS ON)
# When turning this option on, the library will be compiled in such a way to attempt to keep the simulation deterministic across platforms
set(CROSS_PLATFORM_DETERMINISTIC OFF)
# When turning this option on, the library will be compiled with interprocedural optimizations enabled, also known as link-time optimizations or link-time code generation.
# Note that if you turn this on you need to use SET_INTERPROCEDURAL_OPTIMIZATION() or set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON) to enable LTO specificly for your own project as well.
# If you don't do this you may get an error: /usr/bin/ld: libJolt.a: error adding symbols: file format not recognized
#set(INTERPROCEDURAL_OPTIMIZATION ON)
# When turning this on, in Debug and Release mode, the library will emit extra code to ensure that the 4th component of a 3-vector is kept the same as the 3rd component
# and will enable floating point exceptions during simulation to detect divisions by zero.
# Note that this currently only works using MSVC. Clang turns Float2 into a SIMD vector sometimes causing floating point exceptions (the option is ignored).
set(FLOATING_POINT_EXCEPTIONS_ENABLED OFF)
# Number of bits to use in ObjectLayer. Can be 16 or 32.
set(OBJECT_LAYER_BITS 32)
# Select X86 processor features to use, by default the library compiles with AVX2, if everything is off it will be SSE2 compatible.
set(USE_SSE4_1 ON)
set(USE_SSE4_2 ON)
set(USE_AVX OFF)
set(USE_AVX2 OFF)
set(USE_AVX512 OFF)
set(USE_TZCNT OFF)
set(USE_F16C OFF)
set(USE_FMADD OFF)
add_compile_definitions(JPH_ENABLE_VULKAN ON)
add_compile_definitions(JPH_DEBUG_RENDERER ON)
add_compile_definitions(JPH_ENABLE_ASSERTS ON)
add_compile_definitions(JPH_DISABLE_CUSTOM_ALLOCATOR ON)
#add_compile_definitions(JPH_USE_STD_VECTOR ON)
if (MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()
# When turning this on, the library will be compiled with C++ RTTI enabled.
# This adds some overhead and Jolt doesn't use RTTI so by default it is off.
set(CPP_RTTI_ENABLED ON)
FetchContent_Declare(
        JoltPhysics
        GIT_REPOSITORY "https://github.com/jrouwe/JoltPhysics"
        GIT_TAG "v5.5.0"
        SOURCE_SUBDIR "Build"
)
FetchContent_MakeAvailable(JoltPhysics)
if (LINUX AND NOT APPLE)
    target_compile_options(Jolt PRIVATE -stdlib=libc++)
endif ()

