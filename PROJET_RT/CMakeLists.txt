cmake_minimum_required(VERSION 3.0)
project(RayTracing_TP)

set (SYS_DATA_PATH $ENV{HOME} CACHE PATH "Path where to read image mesh ...")
set(SHADER_PATH ${CMAKE_SOURCE_DIR}/shaders/ CACHE PATH "Path to shader directory")
set(TP_SHADER_PATH ${CMAKE_SOURCE_DIR}/tp/ CACHE PATH "Path to shader directory")
set (CMAKE_CXX_STANDARD 14)

add_definitions("-DEIGEN_NO_DEBUG=1 -EIGEN_MAX_STATIC_ALIGN_BYTES=16 -DEIGEN_STRONG_INLINE=1")

if(MSVC)
add_compile_options(/arch:AVX2)
else()
    add_compile_options(-msse4)
endif()

if(APPLE)
	link_directories("/usr/local/lib")
	message(WARNING MACOSX)
endif()

add_subdirectory(easycppogl)
add_subdirectory(bvh_gpu)
add_subdirectory(RaytracingGPU)
add_subdirectory(MontecarloGPU)
add_subdirectory(DrawSampling)
