target_sources(lumos_core
    PUBLIC
        renderfactory.h
        errorcheck.h
        utils.h
        device.h
        device.cpp
        commandpool.h
        commandpool.cpp
        pipeline.h
        pipeline.cpp
        buffer.h
        buffer.cpp
        swapchain.h
        swapchain.cpp
        image.h
        image.cpp
        sampler.h
        sampler.cpp
        texture.h
        texture.cpp
        descriptor.h
        descriptor.cpp
        vertex.h
        baserenderer.h
        resourcemanager.h
        resourcemanager.cpp
        json.hpp
        scene.h
        scene.cpp
)


target_link_libraries(lumos_core
    PUBLIC glm
    PUBLIC Vulkan::Vulkan
    PUBLIC slang::slang
    PUBLIC ktx
)

find_program(SLANGC_EXECUTABLE slangc HINTS $ENV{VULKAN_SDK}/bin REQUIRED)
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
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/sprite.slang
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/presentation.slang
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/cube.slang
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/debug.slang
)
add_dependencies(lumos_core compile_shaders)
