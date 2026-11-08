add_executable(game
    main.cpp
    application.cpp
    application.h
    rendersystem.h
    rendersystem.cpp
    movementsystem.h
    movementsystem.cpp
    tilemap.h
    tilemap.cpp
    defaulttypes.h
    passes/terrainpass.h
    passes/terrainpass.cpp
    passes/scenepass.h
    passes/scenepass.cpp
    passes/passconfig.h
)

target_link_libraries(game
    PUBLIC Lumos::Core)
