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
function(add_slang_shader_target TARGET)
    cmake_parse_arguments("SHADER" "" "" "SOURCES" ${ARGN})
    set(SHADERS_DIR ${CMAKE_BINARY_DIR}/shaders)
    set(ENTRY_POINTS -entry vertMain -entry fragMain)
    add_custom_command(
        OUTPUT ${SHADERS_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
    )
    add_custom_command(
        OUTPUT ${SHADERS_DIR}/slang.spv
        COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCES} -target spirv -profile spirv_1_4 -emit-spirv-directly
        -fvk-use-entrypoint-name ${ENTRY_POINTS} -o slang.spv
        WORKING_DIRECTORY ${SHADERS_DIR}
        DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES}
        COMMENT "Compiling slang shaders"
        VERBATIM
    )
    add_custom_target(${TARGET} DEPENDS ${SHADERS_DIR}/slang.spv)
endfunction()

add_slang_shader_target(test SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/renderer/shaders/shader.slang)
add_dependencies(core test)
