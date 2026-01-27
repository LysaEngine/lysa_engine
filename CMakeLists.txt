#
# Copyright (c) 2025-present Henri Michelon
#
# This software is released under the MIT License.
# https://opensource.org/licenses/MIT
#
cmake_minimum_required(VERSION 3.29)

#######################################################
project(lysa_engine)
if (NOT DEFINED VIREO_RHI_PROJECT_DIR)
    if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.env.cmake")
        message(FATAL_ERROR "Please create a .env.cmake file with the VIREO_RHI_PROJECT_DIR variable")
    else()
        include(.env.cmake)
    endif ()
endif ()
if (NOT DEFINED VIREO_RHI_PROJECT_DIR)
    message(FATAL_ERROR "Please set VIREO_RHI_PROJECT_DIR in the .env.cmake file")
endif()

#######################################################
set(LYSA_ENGINE_TARGET ${PROJECT_NAME})
if (NOT DEFINED LYSA_ENGINE_PROJECT_DIR)
    message("Building Lysa Engine as standalone project")
    set(DIRECTX_BACKEND ON)
    set(LUA_BINDING OFF)
    set(FORWARD_RENDERER ON)
    set(DEFERRED_RENDERER ON)
    set(PHYSIC_ENGINE_JOLT ON)
    set(PHYSIC_ENGINE_PHYSX OFF)
endif()
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
#add_compile_definitions(SHADOW_TRANSPARENCY_COLOR_ENABLED ON)

#######################################################
if (LYSA_CONSOLE)
    add_compile_definitions(LYSA_CONSOLE)
endif ()
if (LUA_BINDING)
    add_compile_definitions(LUA_BINDING)
endif ()

#######################################################
if (FORWARD_RENDERER)
    add_compile_definitions(FORWARD_RENDERER)
endif ()
if (DEFERRED_RENDERER)
    add_compile_definitions(DEFERRED_RENDERER)
endif ()

#######################################################
if(WIN32)
    set(USE_STATIC_MSVC_RUNTIME_LIBRARY ON)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
    else ()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    endif ()
    add_compile_definitions(UNICODE _UNICODE)
elseif (LINUX AND NOT APPLE)
    if (NOT DEFINED USE_SDL3)
        set(USE_SDL3 ON)
        add_compile_definitions(USE_SDL3)
    endif ()
endif ()

#######################################################
if (PHYSIC_ENGINE_JOLT AND PHYSIC_ENGINE_PHYSX)
    message(FATAL_ERROR "Please deactivate a physics library")
endif()
if (PHYSIC_ENGINE_JOLT)
    set(ENABLE_PHYSICS_ENGINE ON)
    add_compile_definitions(PHYSIC_ENGINE_JOLT)
endif()
if (PHYSIC_ENGINE_PHYSX)
    set(ENABLE_PHYSICS_ENGINE ON)
    if (NOT DEFINED PHYSX_ROOT)
        message(FATAL_ERROR "Please set PHYSX_ROOT in the .env.cmake file")
    endif()
    if (NOT DEFINED PHYSX_LIBRARIES)
        message(FATAL_ERROR "Please set PHYSX_LIBRARIES in the .env.cmake file")
    endif()
    add_compile_definitions(PHYSIC_ENGINE_PHYSX PX_PHYSX_STATIC_LIB PX_FOUNDATION_STATIC_LIB PX_PHYSX_COMMON_STATIC_LIB)
endif()
if (ENABLE_PHYSICS_ENGINE)
    add_compile_definitions(ENABLE_PHYSICS_ENGINE)
endif()

#######################################################
include(FetchContent)
if(PHYSIC_ENGINE_JOLT)
    include(cmake/jolt.cmake)
endif()
include(cmake/libraries.cmake)
include(cmake/shaders.cmake)

#######################################################
function(lysa_compile_options TARGET_NAME)
    compile_options(${TARGET_NAME})
    if(MSVC)
        target_compile_options(${TARGET_NAME} PRIVATE /wd4291) # for Flecs 'new' placement operator
    endif ()
endfunction()

#######################################################
set(VIREO_RHI_TARGET "vireo_rhi")
add_subdirectory(${VIREO_RHI_PROJECT_DIR} ${VIREO_RHI_TARGET})

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(ENGINE_SRC_DIR ${SRC_DIR})
set(SHADERS_SRC_DIR ${ENGINE_SRC_DIR}/shaders)
set(SHADERS_INCLUDE_DIR ${ENGINE_SRC_DIR}/shaders/include)
set(SHADERS_BUILD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(DEPENDS_SRC_DIR ${ENGINE_SRC_DIR}/depends)

set(HLSLPP_TARGET hlslpp)
set(HLSLPP_SRC_DIR ${DEPENDS_SRC_DIR}/${HLSLPP_TARGET})

#######################################################
# Slang shaders
file(MAKE_DIRECTORY ${SHADERS_BUILD_DIR})
set(SHADERS_SOURCE_FILES
        "${SHADERS_SRC_DIR}/default.vert.slang"
        "${SHADERS_SRC_DIR}/depth_prepass.vert.slang"
        "${SHADERS_SRC_DIR}/frustum_culling.comp.slang"
        "${SHADERS_SRC_DIR}/frustum_culling_shadowmap.comp.slang"
        "${SHADERS_SRC_DIR}/quad.vert.slang"
        "${SHADERS_SRC_DIR}/vector.slang"
        "${SHADERS_SRC_DIR}/vector_ui.slang"
        "${SHADERS_SRC_DIR}/glyph.slang"
        "${SHADERS_SRC_DIR}/glyph_ui.slang"
        "${SHADERS_SRC_DIR}/deferred/gbuffers.frag.slang"
        "${SHADERS_SRC_DIR}/deferred/glighting.frag.slang"
        "${SHADERS_SRC_DIR}/deferred/glighting_bloom.frag.slang"
        "${SHADERS_SRC_DIR}/deferred/ssao.frag.slang"
        "${SHADERS_SRC_DIR}/forward/forward.frag.slang"
        "${SHADERS_SRC_DIR}/forward/forward_bloom.frag.slang"
        "${SHADERS_SRC_DIR}/forward/transparency_oit.frag.slang"
        "${SHADERS_SRC_DIR}/forward/transparency_oit_composite.frag.slang"
        "${SHADERS_SRC_DIR}/postprocess/bloom.frag.slang"
        "${SHADERS_SRC_DIR}/postprocess/blur.frag.slang"
        "${SHADERS_SRC_DIR}/postprocess/passthrough.frag.slang"
        "${SHADERS_SRC_DIR}/postprocess/depth_buffer.frag.slang"
        "${SHADERS_SRC_DIR}/postprocess/greyscale.frag.slang"
        "${SHADERS_SRC_DIR}/postprocess/ssao_blur.frag.slang"
        "${SHADERS_SRC_DIR}/postprocess/fxaa.frag.slang"
        "${SHADERS_SRC_DIR}/postprocess/smaa_edge_detect.frag.slang"
        "${SHADERS_SRC_DIR}/postprocess/smaa_blend_weight.frag.slang"
        "${SHADERS_SRC_DIR}/postprocess/smaa_neighborhood_blend.frag.slang"
        "${SHADERS_SRC_DIR}/postprocess/gamma_correction.frag.slang"
        "${SHADERS_SRC_DIR}/postprocess/reinhard.frag.slang"
        "${SHADERS_SRC_DIR}/postprocess/aces.frag.slang"
        "${SHADERS_SRC_DIR}/shadows/shadowmap.vert.slang"
        "${SHADERS_SRC_DIR}/shadows/shadowmap.frag.slang"
        "${SHADERS_SRC_DIR}/shadows/shadowmap_cubemap.frag.slang"
)
add_shaders(${LYSA_ENGINE_TARGET}_shaders ${SHADERS_BUILD_DIR} ${SHADERS_INCLUDE_DIR} ${SHADERS_SOURCE_FILES})

#######################################################
# xxHash library
add_library(xxhash STATIC
        ${xxhash_SOURCE_DIR}/xxhash.c
)
target_include_directories(xxhash PUBLIC
        ${xxhash_SOURCE_DIR}
)
compile_options(xxhash)

#######################################################
if(ENABLE_PHYSICS_ENGINE)
    set(PHYSICS_COMMON_SRC
            ${ENGINE_SRC_DIR}/physics/PhysicsEngine.cpp
            ${ENGINE_SRC_DIR}/renderers/DebugRenderer.cpp
            ${ENGINE_SRC_DIR}/resources/physics/CollisionShapes.cpp
    )
    set(PHYSICS_COMMON_MODULES
            ${ENGINE_SRC_DIR}/physics/Configuration.ixx
            ${ENGINE_SRC_DIR}/physics/PhysicsEngine.ixx
            ${ENGINE_SRC_DIR}/physics/PhysicsMaterial.ixx
            ${ENGINE_SRC_DIR}/renderers/DebugRenderer.ixx
            ${ENGINE_SRC_DIR}/resources/physics/CollisionArea.ixx
            ${ENGINE_SRC_DIR}/resources/physics/CollisionObject.ixx
            ${ENGINE_SRC_DIR}/resources/physics/CollisionShapes.ixx
            ${ENGINE_SRC_DIR}/resources/physics/KinematicBody.ixx
            ${ENGINE_SRC_DIR}/resources/physics/PhysicsBody.ixx
            ${ENGINE_SRC_DIR}/resources/physics/RayCast.ixx
            ${ENGINE_SRC_DIR}/resources/physics/RigidBody.ixx
            ${ENGINE_SRC_DIR}/resources/physics/StaticBody.ixx
    )
    if(PHYSIC_ENGINE_JOLT)
        set(PHYSICS_SRC
                ${PHYSICS_COMMON_SRC}
                ${ENGINE_SRC_DIR}/physics/jolt/CollisionArea.cpp
                ${ENGINE_SRC_DIR}/physics/jolt/CollisionObject.cpp
                ${ENGINE_SRC_DIR}/physics/jolt/CollisionShapes.cpp
                ${ENGINE_SRC_DIR}/physics/jolt/KinematicBody.cpp
                ${ENGINE_SRC_DIR}/physics/jolt/PhysicsBody.cpp
                ${ENGINE_SRC_DIR}/physics/jolt/PhysicsEngine.cpp
                ${ENGINE_SRC_DIR}/physics/jolt/RayCast.cpp
                ${ENGINE_SRC_DIR}/physics/jolt/RigidBody.cpp
                ${ENGINE_SRC_DIR}/physics/jolt/StaticBody.cpp
        )
        set(PHYSICS_MODULES
                ${PHYSICS_COMMON_MODULES}
                ${ENGINE_SRC_DIR}/physics/jolt/PhysicsEngine.ixx
        )
    endif ()
    if(PHYSIC_ENGINE_PHYSX)
        set(PHYSICS_SRC
                ${PHYSICS_COMMON_SRC}
        )
        set(PHYSICS_MODULES
                ${PHYSICS_COMMON_MODULES}
        )
    endif ()
endif ()


#######################################################
if(WIN32)
    if (NOT LYSA_CONSOLE)
        set(WIN32_UI_SRC
                ${ENGINE_SRC_DIR}/platform/win32/Input.cpp
                ${ENGINE_SRC_DIR}/platform/win32/Lysa.cpp
                ${ENGINE_SRC_DIR}/platform/win32/RenderingWindow.cpp
        )
    endif ()
    set(WIN32_SRC
            ${WIN32_UI_SRC}
            ${ENGINE_SRC_DIR}/platform/win32/DirectoryWatcher.cpp
            ${ENGINE_SRC_DIR}/platform/win32/Main.cpp
    )
    set(WIN32_MODULES ""
    )
elseif (UNIX AND NOT APPLE)
    if (NOT LYSA_CONSOLE)
        set(SDL_SRC
                ${ENGINE_SRC_DIR}/platform/sdl/Lysa.cpp
                ${ENGINE_SRC_DIR}/platform/sdl/Input.cpp
                ${ENGINE_SRC_DIR}/platform/sdl/RenderingWindow.cpp
        )
    endif ()
    set(POSIX_SRC
            ${SDL_SRC}
            ${ENGINE_SRC_DIR}/platform/posix/Main.cpp
#            ${ENGINE_SRC_DIR}/platform/win32/DirectoryWatcher.cpp
    )
    set(POSIX_MODULES ""
    )
endif ()

#######################################################
if(LUA_BINDING)
    set(LUA_BINDINGS_SRC
            ${ENGINE_SRC_DIR}/bindings/lua/Lua.cpp
            ${lua_socket_SOURCE_DIR}/src/luasocket.h
            ${lua_socket_SOURCE_DIR}/src/except.c
            ${lua_socket_SOURCE_DIR}/src/luasocket.c
            ${lua_socket_SOURCE_DIR}/src/timeout.c
            ${lua_socket_SOURCE_DIR}/src/buffer.c
            ${lua_socket_SOURCE_DIR}/src/io.c
            ${lua_socket_SOURCE_DIR}/src/auxiliar.c
            ${lua_socket_SOURCE_DIR}/src/options.c
            ${lua_socket_SOURCE_DIR}/src/inet.c
            ${lua_socket_SOURCE_DIR}/src/tcp.c
            ${lua_socket_SOURCE_DIR}/src/udp.c
            ${lua_socket_SOURCE_DIR}/src/select.c
            ${lua_socket_SOURCE_DIR}/src/wsocket.c
    )
    set(LUA_BINDINGS_MODULES
            ${ENGINE_SRC_DIR}/bindings/lua/Lua.ixx
    )
endif ()

#######################################################
if(FORWARD_RENDERER)
    set(FORWARD_RENDERER_SRC
            ${ENGINE_SRC_DIR}/renderers/ForwardRenderer.cpp
            ${ENGINE_SRC_DIR}/renderers/renderpasses/ForwardColorPass.cpp
    )
    set(FORWARD_RENDERER_MODULES
            ${ENGINE_SRC_DIR}/renderers/ForwardRenderer.ixx
            ${ENGINE_SRC_DIR}/renderers/renderpasses/ForwardColorPass.ixx
    )
endif ()
if(DEFERRED_RENDERER)
    set(DEFERRED_RENDERER_SRC
            ${ENGINE_SRC_DIR}/renderers/DeferredRenderer.cpp
            ${ENGINE_SRC_DIR}/renderers/renderpasses/GBufferPass.cpp
            ${ENGINE_SRC_DIR}/renderers/renderpasses/LightingPass.cpp
            ${ENGINE_SRC_DIR}/renderers/renderpasses/SSAOPass.cpp
    )
    set(DEFERRED_RENDERER_MODULES
            ${ENGINE_SRC_DIR}/renderers/DeferredRenderer.ixx
            ${ENGINE_SRC_DIR}/renderers/renderpasses/GBufferPass.ixx
            ${ENGINE_SRC_DIR}/renderers/renderpasses/LightingPass.ixx
            ${ENGINE_SRC_DIR}/renderers/renderpasses/SSAOPass.ixx
    )
endif ()

#######################################################
add_library(${LYSA_ENGINE_TARGET} STATIC
        ${ENGINE_SRC_DIR}/AsyncQueue.cpp
        ${ENGINE_SRC_DIR}/AssetsPack.cpp
        ${ENGINE_SRC_DIR}/Context.cpp
        ${ENGINE_SRC_DIR}/Event.cpp
        ${ENGINE_SRC_DIR}/Input.cpp
        ${ENGINE_SRC_DIR}/Lysa.cpp
        ${ENGINE_SRC_DIR}/Math.cpp
        ${ENGINE_SRC_DIR}/Memory.cpp
        ${ENGINE_SRC_DIR}/VirtualFS.cpp

        ${DEFERRED_RENDERER_SRC}
        ${FORWARD_RENDERER_SRC}
        ${ENGINE_SRC_DIR}/renderers/GlobalDescriptorSet.cpp
        ${ENGINE_SRC_DIR}/renderers/GraphicPipelineData.cpp
        ${ENGINE_SRC_DIR}/renderers/Renderer.cpp
        ${ENGINE_SRC_DIR}/renderers/SceneFrameData.cpp
        ${ENGINE_SRC_DIR}/renderers/Vector2DRenderer.cpp
        ${ENGINE_SRC_DIR}/renderers/Vector3DRenderer.cpp
        ${ENGINE_SRC_DIR}/renderers/pipelines/FrustumCulling.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpasses/BloomPass.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpasses/DepthPrepass.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpasses/PostProcessing.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpasses/Renderpass.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpasses/ShaderMaterialPass.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpasses/ShadowMapPass.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpasses/SMAAPass.cpp
        ${ENGINE_SRC_DIR}/renderers/renderpasses/TransparencyPass.cpp

        ${ENGINE_SRC_DIR}/resources/Animation.cpp
        ${ENGINE_SRC_DIR}/resources/AnimationTrack.cpp
        ${ENGINE_SRC_DIR}/resources/Font.cpp
        ${ENGINE_SRC_DIR}/resources/Image.cpp
        ${ENGINE_SRC_DIR}/resources/Material.cpp
        ${ENGINE_SRC_DIR}/resources/Mesh.cpp
        ${ENGINE_SRC_DIR}/resources/MeshInstance.cpp
        ${ENGINE_SRC_DIR}/resources/RenderTarget.cpp
        ${ENGINE_SRC_DIR}/resources/RenderingWindow.cpp
        ${ENGINE_SRC_DIR}/resources/Samplers.cpp
        ${ENGINE_SRC_DIR}/resources/Scene.cpp

        ${ENGINE_SRC_DIR}/utils/AABB.cpp
        ${ENGINE_SRC_DIR}/utils/AsyncTasksPool.cpp
        ${ENGINE_SRC_DIR}/utils/DeferredTasksBuffer.cpp
        ${ENGINE_SRC_DIR}/utils/Frustum.cpp
        ${ENGINE_SRC_DIR}/utils/Log.cpp
        ${ENGINE_SRC_DIR}/utils/Utils.cpp

        ${POSIX_SRC}
        ${WIN32_SRC}
        ${PHYSICS_SRC}
        ${LUA_BINDINGS_SRC}
        ${DEPENDS_SRC_DIR}/stb/stb.cpp
)
target_sources(${LYSA_ENGINE_TARGET}
    PUBLIC
    FILE_SET CXX_MODULES
    FILES
        ${ENGINE_SRC_DIR}/AsyncQueue.ixx
        ${ENGINE_SRC_DIR}/AssetsPack.ixx
        ${ENGINE_SRC_DIR}/Context.ixx
        ${ENGINE_SRC_DIR}/Event.ixx
        ${ENGINE_SRC_DIR}/Exception.ixx
        ${ENGINE_SRC_DIR}/Input.ixx
        ${ENGINE_SRC_DIR}/InputEvent.ixx
        ${ENGINE_SRC_DIR}/Lysa.ixx
        ${ENGINE_SRC_DIR}/Math.ixx
        ${ENGINE_SRC_DIR}/Memory.ixx
        ${ENGINE_SRC_DIR}/Types.ixx
        ${ENGINE_SRC_DIR}/VirtualFS.ixx

        ${FORWARD_RENDERER_MODULES}
        ${DEFERRED_RENDERER_MODULES}
        ${ENGINE_SRC_DIR}/renderers/Configuration.ixx
        ${ENGINE_SRC_DIR}/renderers/GlobalDescriptorSet.ixx
        ${ENGINE_SRC_DIR}/renderers/GraphicPipelineData.ixx
        ${ENGINE_SRC_DIR}/renderers/Renderer.ixx
        ${ENGINE_SRC_DIR}/renderers/SceneFrameData.ixx
        ${ENGINE_SRC_DIR}/renderers/Vector2DRenderer.ixx
        ${ENGINE_SRC_DIR}/renderers/Vector3DRenderer.ixx
        ${ENGINE_SRC_DIR}/renderers/pipelines/FrustumCulling.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpasses/BloomPass.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpasses/DepthPrepass.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpasses/DisplayAttachment.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpasses/FXAAPass.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpasses/GammaCorrectionPass.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpasses/PostProcessing.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpasses/Renderpass.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpasses/ShaderMaterialPass.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpasses/ShadowMapPass.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpasses/SMAAPass.ixx
        ${ENGINE_SRC_DIR}/renderers/renderpasses/TransparencyPass.ixx

        ${ENGINE_SRC_DIR}/resources/Animation.ixx
        ${ENGINE_SRC_DIR}/resources/AnimationLibrary.ixx
        ${ENGINE_SRC_DIR}/resources/AnimationTrack.ixx
        ${ENGINE_SRC_DIR}/resources/Camera.ixx
        ${ENGINE_SRC_DIR}/resources/Environment.ixx
        ${ENGINE_SRC_DIR}/resources/Font.ixx
        ${ENGINE_SRC_DIR}/resources/Image.ixx
        ${ENGINE_SRC_DIR}/resources/Light.ixx
        ${ENGINE_SRC_DIR}/resources/Material.ixx
        ${ENGINE_SRC_DIR}/resources/Mesh.ixx
        ${ENGINE_SRC_DIR}/resources/MeshInstance.ixx
        ${ENGINE_SRC_DIR}/resources/RenderingWindow.ixx
        ${ENGINE_SRC_DIR}/resources/RenderTarget.ixx
        ${ENGINE_SRC_DIR}/resources/RenderView.ixx
        ${ENGINE_SRC_DIR}/resources/Resources.ixx
        ${ENGINE_SRC_DIR}/resources/ResourcesManager.ixx
        ${ENGINE_SRC_DIR}/resources/ResourcesRegistry.ixx
        ${ENGINE_SRC_DIR}/resources/Samplers.ixx
        ${ENGINE_SRC_DIR}/resources/Scene.ixx
        ${ENGINE_SRC_DIR}/resources/Texture.ixx

        ${ENGINE_SRC_DIR}/utils/AABB.ixx
        ${ENGINE_SRC_DIR}/utils/AsyncTasksPool.ixx
        ${ENGINE_SRC_DIR}/utils/BlurData.ixx
        ${ENGINE_SRC_DIR}/utils/DeferredTasksBuffer.ixx
        ${ENGINE_SRC_DIR}/utils/DirectoryWatcher.ixx
        ${ENGINE_SRC_DIR}/utils/Frustum.ixx
        ${ENGINE_SRC_DIR}/utils/Log.ixx
        ${ENGINE_SRC_DIR}/utils/Rect.ixx
        ${ENGINE_SRC_DIR}/utils/Utils.ixx

        ${POSIX_MODULES}
        ${WIN32_MODULES}
        ${PHYSICS_MODULES}
        ${LUA_BINDINGS_MODULES}
)
lysa_compile_options(${LYSA_ENGINE_TARGET})
target_include_directories(${LYSA_ENGINE_TARGET} PUBLIC
        ${INCLUDE_DIR}
        ${HLSLPP_SRC_DIR}/include
        ${xxhash_SOURCE_DIR}
        ${DEPENDS_SRC_DIR}
        ${DEPENDS_SRC_DIR}/json
        ${DEPENDS_SRC_DIR}/stb
        ${freetype_SOURCE_DIR}/include
        ${harfbuzz_SOURCE_DIR}/src
        ${msdfgen_SOURCE_DIR}
        ${msdf_atlas_gen_SOURCE_DIR}
)
target_link_libraries(${LYSA_ENGINE_TARGET} ${VIREO_RHI_TARGET}
        std-cxx-modules
        xxhash
        Freetype::Freetype
        harfbuzz
)
add_dependencies(${LYSA_ENGINE_TARGET} ${VIREO_RHI_TARGET})

if(PHYSIC_ENGINE_JOLT)
    target_include_directories(${LYSA_ENGINE_TARGET} PRIVATE ${JoltPhysics_SOURCE_DIR}/..)
    target_link_libraries(${LYSA_ENGINE_TARGET} Jolt)
endif()
if (PHYSIC_ENGINE_PHYSX)
    target_include_directories(${LYSA_ENGINE_TARGET} PRIVATE ${PHYSX_ROOT}/include)
    target_link_directories(${LYSA_ENGINE_TARGET} PRIVATE ${PHYSX_LIBRARIES})
    target_link_libraries(${LYSA_ENGINE_TARGET}
            PhysX_static_64
            PhysXCharacterKinematic_static_64
            PhysXCommon_static_64
            PhysXCooking_static_64
            PhysXExtensions_static_64
            PhysXFoundation_static_64
            PhysXPvdSDK_static_64
    )
endif()
if (USE_SDL3)
    target_link_libraries(${LYSA_ENGINE_TARGET} SDL3::SDL3)
endif ()
if(LUA_BINDING)
    target_link_libraries(${LYSA_ENGINE_TARGET} ${VIREO_RHI_TARGET}
        lua
    )
    target_include_directories(${LYSA_ENGINE_TARGET} PUBLIC
        ${LUA_INCLUDE_DIR} # defined in Vireo
    )
endif ()
if(WIN32)
    target_precompile_headers(${LYSA_ENGINE_TARGET} PRIVATE
        "<intrin.h>"
    )
    target_link_libraries(${LYSA_ENGINE_TARGET} Ws2_32)
    if (DIRECTX_BACKEND)
        target_link_libraries(${LYSA_ENGINE_TARGET} d3d12)
    endif ()
    if(MINGW)
        target_link_options(${LYSA_ENGINE_TARGET} PRIVATE "-mwindows")
    endif()
    set_target_properties(${LYSA_ENGINE_TARGET} PROPERTIES
            WIN32_EXECUTABLE TRUE)
endif ()
if (UNIX AND NOT APPLE)
    target_compile_options(${LYSA_ENGINE_TARGET} PRIVATE -stdlib=libc++)
endif ()
set_property(TARGET ${LYSA_ENGINE_TARGET} PROPERTY COMPILE_WARNING_AS_ERROR ON)


#######################################################
find_program(DOXYPRESS_EXECUTABLE doxypress)

if (DOXYPRESS_EXECUTABLE)
    message(STATUS "DoxyPress found: ${DOXYPRESS_EXECUTABLE}")
    set(DOCS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/docs")

    add_custom_target(lysa_docs
            COMMAND ${DOXYPRESS_EXECUTABLE}
            COMMAND ${CMAKE_COMMAND} -E copy
            ${DOCS_DIRECTORY}/navtree.js
            ${DOCS_DIRECTORY}/html/navtree.js
            WORKING_DIRECTORY ${DOCS_DIRECTORY}
            COMMENT "Generating Lysa documentation with DoxyPress"
            VERBATIM
    )
else()
    message(STATUS "DoxyPress not found, doxypress target will not be available.")
endif()
