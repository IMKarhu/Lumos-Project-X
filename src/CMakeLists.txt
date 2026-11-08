add_subdirectory(core)
add_subdirectory(game)

# add_executable(game
#     main.cpp
#     application.cpp
#     application.h
#     rendersystem.h
#     rendersystem.cpp
#     movementsystem.h
#     movementsystem.cpp
#     tilemap.h
#     tilemap.cpp
#     textureatlas.h
#     textureatlas.cpp
#     atlasloader.h
#     atlasloader.cpp
#     defaulttypes.h
#     passes/terrainpass.h
#     passes/terrainpass.cpp
#     passes/scenepass.h
#     passes/scenepass.cpp
#     passes/passconfig.h
# )
#
# set_property(TARGET game PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/buildvs")
#
# target_include_directories(game
#     PRIVATE "${CMAKE_SOURCE_DIR}/src"
# )
#
# target_link_libraries(game
#     PRIVATE lumos_core
# )
