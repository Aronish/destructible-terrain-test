cmake_minimum_required(VERSION 3.23.2)

project(engineering_game)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags and predefined macros
set(CMAKE_CXX_FLAGS "/EHsc /W4 /wd4706 /arch:AVX2")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/Zi /O2 /Ob2")
set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob3")

set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG:FULL")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(PHYSX_CONFIG "debug")
elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(PHYSX_CONFIG "checked")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(PHYSX_CONFIG "release")
endif()

add_compile_definitions(_DLL $<$<CONFIG:Debug>:ENG_DEBUG> $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:ENG_LOG_ENABLED> $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>>:NDEBUG>)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(engineering_game "")
add_subdirectory(src)

# Resources
message("Copying resources")
function(copy_resource resource_folder file_type)
    file(GLOB resources "${CMAKE_CURRENT_SOURCE_DIR}/${resource_folder}/*${file_type}")
    foreach(resource ${resources})
        get_filename_component(outputFileName ${resource} NAME)
        configure_file(${resource} ${CMAKE_CURRENT_BINARY_DIR}/${resource_folder}/${outputFileName} COPYONLY)
    endforeach()
endfunction()

copy_resource("res/shaders" "")
copy_resource("res/textures" "")

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
copy_resource("lib/physx/bin/${PHYSX_CONFIG}" "lib")
target_include_directories(engineering_game PRIVATE lib/physx/include/)
target_include_directories(engineering_game PRIVATE lib/physx/pxshared/include/)
target_link_libraries(engineering_game lib/physx/bin/${PHYSX_CONFIG}/PhysX_64)
target_link_libraries(engineering_game lib/physx/bin/${PHYSX_CONFIG}/PhysXCommon_64)
target_link_libraries(engineering_game lib/physx/bin/${PHYSX_CONFIG}/PhysXFoundation_64)
target_link_libraries(engineering_game lib/physx/bin/${PHYSX_CONFIG}/PhysXCooking_64)
target_link_libraries(engineering_game lib/physx/bin/${PHYSX_CONFIG}/PhysXPvdSDK_static_64)
target_link_libraries(engineering_game lib/physx/bin/${PHYSX_CONFIG}/PhysXExtensions_static_64)
target_link_libraries(engineering_game lib/physx/bin/${PHYSX_CONFIG}/PhysXCharacterKinematic_static_64)

# physx pre-build library copying
file(GLOB physx_dlls ${CMAKE_CURRENT_SOURCE_DIR}/lib/physx/bin/${PHYSX_CONFIG}/*.dll)
foreach(physx_dll ${physx_dlls})
    add_custom_command(
        TARGET engineering_game PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${physx_dll}
            $<TARGET_FILE_DIR:engineering_game>
    )
endforeach()