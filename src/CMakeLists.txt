add_executable(game
    main.cpp
    application.cpp
    application.h
    rendersystem.h
    rendersystem.cpp
    physicssystem.h
    physicssystem.cpp
    tilemap.h
    tilemap.cpp
    textureatlas.h
    textureatlas.cpp
    atlasloader.h
    atlasloader.cpp
    defaulttypes.h
    passes/terrainpass.h
    passes/terrainpass.cpp
    passes/scenepass.h
    passes/scenepass.cpp
    passes/passconfig.h
)

set_property(TARGET game PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/buildvs")

target_include_directories(game
    PRIVATE "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(game
    PRIVATE core
)

#set(SRC_SHADER_DIRECTORY "${CMAKE_SOURCE_DIR}/src/game/shaders")
#set(DST_SHADER_DIRECTORY "${CMAKE_BINARY_DIR}/shaders")
#file(GLOB_RECURSE SOURCE_FILES "${SRC_SHADER_DIRECTORY}/*")
#set(STAMP_FILE "${CMAKE_BINARY_DIR}/shaders/dir_copy.stamp")
#add_custom_command(
#    OUTPUT "${STAMP_FILE}"
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#            ${SRC_SHADER_DIRECTORY}
#            ${DST_SHADER_DIRECTORY}
#    COMMAND ${CMAKE_COMMAND} -E touch "${STAMP_FILE}"
#    DEPENDS ${SOURCE_FILES}
#    COMMENT "Checking and Copying shaders"
#)

#add_custom_target(copy_shader_directory ALL
#    DEPENDS "${STAMP_FILE}"
#)
