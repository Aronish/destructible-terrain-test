cmake_minimum_required(VERSION 3.20.2)
project(stb_image)

add_library(stb_image include/stb_image/stb_image.hpp src/stb_image_impl.cpp)
target_include_directories(stb_image PUBLIC include/)