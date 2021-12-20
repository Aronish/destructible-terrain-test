cmake_minimum_required(VERSION 3.20.2)

project(engineering_game)

# Compiler flags and predefined macros
if ("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
    set(CMAKE_CXX_FLAGS "/EHsc /Zi /W4 /arch:AVX2")
    set(CMAKE_EXE_LINKER_FLAGS "/DEBUG:FULL")
    add_compile_definitions(ENG_DEBUG)
    add_compile_definitions(ENG_LOG_ENABLED)
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL Release)
    set(CMAKE_CXX_FLAGS "/EHsc /O2 /W4 /arch:AVX2")
    add_compile_definitions(ENG_LOG_ENABLED)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(engineering_game "")
add_subdirectory(src)

# Resources
function(copy_resource resource_folder)
    file(GLOB resources "${CMAKE_CURRENT_SOURCE_DIR}/res/${resource_folder}/*")
    foreach(resource ${resources})
        get_filename_component(outputFileName ${resource} NAME)
        configure_file(${resource} ${CMAKE_CURRENT_BINARY_DIR}/res/${resource_folder}/${outputFileName} COPYONLY)
    endforeach()
endfunction(copy_resource)

copy_resource("shaders")
copy_resource("textures")

## Libraries
# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw/)
target_link_libraries(engineering_game glfw)

# GLAD
add_subdirectory(lib/glad/)
target_link_libraries(engineering_game glad)

# GLM
add_subdirectory(lib/glm/)
target_compile_options(glm INTERFACE /DGLM_FORCE_CXX2A /wd4201 /DGLM_FORCE_INTRINSICS)
target_link_libraries(engineering_game glm)

# stb_image
add_subdirectory(lib/stb/stb_image/)
target_link_libraries(engineering_game stb_image)

# imgui
add_subdirectory(lib/imgui/)
target_link_libraries(engineering_game imgui)