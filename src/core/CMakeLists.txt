add_library(core
    SHARED
    windowing/window.cpp
    windowing/window.h
    windowing/events.h
    renderer/renderfactory.h
    renderer/errorcheck.h
    renderer/utils.h
    renderer/device.h
    renderer/device.cpp
    renderer/renderer.h
    renderer/renderer.cpp
    renderer/commandpool.h
    renderer/commandpool.cpp
    renderer/pipeline.h
    renderer/pipeline.cpp
    renderer/buffer.h
    renderer/buffer.cpp
    renderer/swapchain.h
    renderer/swapchain.cpp
    renderer/camera.h
    renderer/camera.cpp
    renderer/image.h
    renderer/image.cpp
    renderer/sampler.h
    renderer/sampler.cpp
    renderer/texture.h
    renderer/texture.cpp
    renderer/descriptor.h
    renderer/descriptor.cpp
    renderer/spriterenderer.h
    renderer/spriterenderer.cpp
    renderer/scenerenderer.h
    renderer/scenerenderer.cpp
    renderer/3drenderer.h
    renderer/3drenderer.cpp
    renderer/vertex.h
    renderer/baserenderer.h
    ecs/componentpool.h
    ecs/components.h
    ecs/ecs.h
    ecs/view.h
    json.hpp
    scene.h
    scene.cpp
    resourcemanager.h
    resourcemanager.cpp
)

find_package(Vulkan REQUIRED)
find_package(slang REQUIRED)

target_link_libraries(core
    PUBLIC glm
    PUBLIC glfw
    PUBLIC Vulkan::Vulkan
    PUBLIC slang::slang
    PUBLIC ktx
)

target_include_directories(core
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(core
    PRIVATE CORE_EXPORT
    PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE
)

find_program(SLANGC_EXECUTABLE slangc HINTS $ENV{VULKAN_SDK}/bin REQUIRED)
# function(add_slang_shader_target TARGET)
#     cmake_parse_arguments("SHADER" "" "" "SOURCES" ${ARGN})
#     set(SHADERS_DIR ${CMAKE_BINARY_DIR}/shaders)
#     set(ENTRY_POINTS -entry vertMain -entry fragMain)
#     add_custom_command(
#         OUTPUT ${SHADERS_DIR}
#         COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
#     )
#     add_custom_command(
#         OUTPUT ${SHADERS_DIR}/slang.spv
#         COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_4 -emit-spirv-directly
#         -fvk-use-entrypoint-name ${ENTRY_POINTS} -o slang.spv
#         WORKING_DIRECTORY ${SHADERS_DIR}
#         DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES}
#         COMMENT "Compiling slang shaders"
#         VERBATIM
#     )
#     add_custom_target(${TARGET} DEPENDS ${SHADERS_DIR}/slang.spv)
# endfunction()
function(add_slang_shader_target TARGET)
    cmake_parse_arguments("SHADER" "" "" "SOURCES" ${ARGN})
    if (NOT SHADER_SOURCES)
        message(FATAL_ERROR "add_slang_shader_target(${TARGET}) requires SOURCES")
    endif()

    set(SHADERS_DIR "${CMAKE_BINARY_DIR}/shaders")
    set(ENTRY_POINTS -entry vertMain -entry fragMain)
    file(MAKE_DIRECTORY "${SHADERS_DIR}")

    set(SHADER_OUTPUTS)
    
    foreach(SHADER_SOURCE IN LISTS SHADER_SOURCES)
        get_filename_component(SHADER_NAME "${SHADER_SOURCE}" NAME_WE)
        set(SHADER_OUTPUT "${SHADERS_DIR}/${SHADER_NAME}.spv")
        add_custom_command(
            OUTPUT "${SHADER_OUTPUT}"
            COMMAND "${SLANGC_EXECUTABLE}" ${SHADER_SOURCE} -target spirv -profile spirv_1_4 -emit-spirv-directly
            -fvk-use-entrypoint-name ${ENTRY_POINTS} -o "${SHADER_OUTPUT}"
            DEPENDS ${SHADER_SOURCES}
            COMMENT "Compiling ${SHADER_SOURCES}"
            VERBATIM
        )
        list(APPEND SHADER_OUTPUTS "${SHADER_OUTPUT}")
    endforeach()
    add_custom_target(${TARGET} DEPENDS ${SHADER_OUTPUTS})
endfunction()

add_slang_shader_target(compile_shaders
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/shaders/sprite.slang
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/shaders/presentation.slang
    ${CMAKE_CURRENT_SOURCE_DIR}/renderer/shaders/cube.slang

)
add_dependencies(core compile_shaders)
