#
# Copyright (c) 2025-present Henri Michelon
#
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT
#
#######################################################
function(lysa_compile_options TARGET_NAME)
    vireo_compile_options(${TARGET_NAME})
    if (PHYSIC_ENGINE_JOLT)
        add_compile_definitions(PHYSIC_ENGINE_JOLT)
        add_compile_definitions(JPH_ENABLE_VULKAN ON)
        add_compile_definitions(JPH_DEBUG_RENDERER ON)
        add_compile_definitions(JPH_ENABLE_ASSERTS ON)
        add_compile_definitions(JPH_DISABLE_CUSTOM_ALLOCATOR ON)
    endif()
    if (PHYSIC_ENGINE_PHYSX)
        add_compile_definitions(PHYSIC_ENGINE_PHYSX)
    endif()
    if (ENABLE_PHYSICS_ENGINE)
        add_compile_definitions(ENABLE_PHYSICS_ENGINE)
    endif()
    target_include_directories(${TARGET_NAME} PUBLIC ${xxhash_SOURCE_DIR})
endfunction()

#######################################################
function(build_target TARGET_NAME SRCS MODULES)
    message("Building target ${TARGET_NAME}")
    add_library(${TARGET_NAME} ${SRCS})
    target_sources(${TARGET_NAME}
            PUBLIC
            FILE_SET CXX_MODULES
            FILES
            ${MODULES}
    )
    lysa_compile_options(${TARGET_NAME})
    add_dependencies(${TARGET_NAME} ${LYSA_ENGINE_TARGET})
    target_link_libraries(${TARGET_NAME} ${LYSA_ENGINE_TARGET})
endfunction()
