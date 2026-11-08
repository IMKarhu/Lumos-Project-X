project(LumosCore VERSION 0.1.0 LANGUAGES CXX)

add_library(lumos_core STATIC)
add_library(Lumos::Core ALIAS lumos_core)

target_compile_features(lumos_core PUBLIC cxx_std_23)

target_include_directories(lumos_core
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

find_package(Vulkan REQUIRED)
find_package(slang REQUIRED)

add_subdirectory(ecs)
add_subdirectory(windowing)
add_subdirectory(renderer)

# add_library(core
#     SHARED
#     windowing/window.cpp
#     windowing/window.h
#     windowing/events.h

#     json.hpp
#     scene.h
#     scene.cpp
#     resourcemanager.h
#     resourcemanager.cpp
# )


# target_link_libraries(core
#     PUBLIC renderer
#     PUBLIC ecs
#     PUBLIC glm
#     PUBLIC glfw
# )

# target_include_directories(core
#     PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
# )
#
# target_compile_definitions(core
#     PRIVATE CORE_EXPORT
#     PUBLIC GLM_FORCE_DEPTH_ZERO_TO_ONE
# )
#

