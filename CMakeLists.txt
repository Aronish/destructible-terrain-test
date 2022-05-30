cmake_minimum_required(VERSION 3.20.2)

project(engineering_game)

# Compiler flags and predefined macros
if ("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
    set(CMAKE_CXX_FLAGS "/EHsc /Zi /W4 /wd4706 /arch:AVX2")
    set(CMAKE_EXE_LINKER_FLAGS "/DEBUG:FULL")
    add_compile_definitions(ENG_DEBUG)
    add_compile_definitions(ENG_LOG_ENABLED)
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL Release)
    set(CMAKE_CXX_FLAGS "/EHsc /O2 /W4 /wd4706 /arch:AVX2")
    add_compile_definitions(ENG_LOG_ENABLED)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(engineering_game "")
add_subdirectory(src)

# Resources
message("Copying resources")
function(copy_resource resource_folder)
    file(GLOB resources "${CMAKE_CURRENT_SOURCE_DIR}/${resource_folder}/*")
    foreach(resource ${resources})
        get_filename_component(outputFileName ${resource} NAME)
        configure_file(${resource} ${CMAKE_CURRENT_BINARY_DIR}/${resource_folder}/${outputFileName} COPYONLY)
    endforeach()
endfunction(copy_resource)

copy_resource("res/shaders")
copy_resource("res/textures")
copy_resource("lib/physx/bin/release")

## Libraries
# GLFW
message("Linking libraries")
message("[LINK]: GLFW")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw/)
target_link_libraries(engineering_game glfw)

# GLAD
message("[LINK]: GLAD")
add_subdirectory(lib/glad/)
target_link_libraries(engineering_game glad)

# GLM
message("[LINK]: GLM")
add_subdirectory(lib/glm/)
target_compile_options(glm INTERFACE /DGLM_FORCE_CXX2A /wd4201 /DGLM_FORCE_INTRINSICS)
target_link_libraries(engineering_game glm)

# stb_image
message("[LINK]: stb_image")
add_subdirectory(lib/stb/stb_image/)
target_link_libraries(engineering_game stb_image)

# imgui
message("[LINK]: ImGui")
add_subdirectory(lib/imgui/)
target_link_libraries(engineering_game imgui)

# physx
message("[LINK]: PhysX")
target_include_directories(engineering_game PRIVATE lib/physx/include/)
target_include_directories(engineering_game PRIVATE lib/physx/pxshared/include/)
target_link_libraries(engineering_game lib/physx/bin/release/PhysX_64)
target_link_libraries(engineering_game lib/physx/bin/release/PhysXCommon_64)
target_link_libraries(engineering_game lib/physx/bin/release/PhysXFoundation_64)

# physx pre-build dll copy
message("Copying DLLs")
function(copy_dlls lib_path)
    file(GLOB physx_dlls ${lib_path}/*.dll)
    foreach(physx_dll ${physx_dlls})
        add_custom_command(
            TARGET engineering_game PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${physx_dll}
                $<TARGET_FILE_DIR:engineering_game>
        )
    endforeach()
endfunction()

if ("${CMAKE_BUILD_TYPE}" STREQUAL Debug)
    message("[PhysX]: checked")
    copy_dlls(${CMAKE_CURRENT_SOURCE_DIR}/lib/physx/bin/checked)
endif()

if ("${CMAKE_BUILD_TYPE}" STREQUAL Release)
    message("[PhysX]: release")
    copy_dlls(${CMAKE_CURRENT_SOURCE_DIR}/lib/physx/bin/release)
endif()
