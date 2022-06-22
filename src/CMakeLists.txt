target_sources(engineering_game
    PRIVATE
        application.cpp application.hpp
        first_person_camera.cpp first_person_camera.hpp
        game_system.hpp game_system.cpp
        logger.hpp
        main.cpp
        window.cpp window.hpp
)

target_include_directories(engineering_game PRIVATE ${CMAKE_CURRENT_LIST_DIR})

add_subdirectory(event)
add_subdirectory(graphics)
add_subdirectory(world)