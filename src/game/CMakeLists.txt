add_executable(game
    main.cpp
    application.cpp
    application.h
    inputsystem.cpp
    inputsystem.h
)

target_link_libraries(game
    PUBLIC Lumos::Core)
