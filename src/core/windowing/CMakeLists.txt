target_sources(lumos_core
    PRIVATE
        events.h
        window.cpp
        window.h
)

target_link_libraries(lumos_core
    PUBLIC glfw
)
