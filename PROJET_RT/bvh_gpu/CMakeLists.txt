cmake_minimum_required(VERSION 3.0)
project(bvh_gpu LANGUAGES CXX)

add_library(${PROJECT_NAME} STATIC scene.h scene.cpp bvh.h bvh.cpp gpu_bvh_scene.h gpu_bvh_scene.cpp glsl_rec.h glsl_rec.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(${PROJECT_NAME} PUBLIC easycppogl)
