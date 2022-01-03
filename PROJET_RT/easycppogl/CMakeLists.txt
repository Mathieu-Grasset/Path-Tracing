cmake_minimum_required(VERSION 3.0)
project(easycppogl LANGUAGES C CXX)

if(DEFINED ENV{VCPKG_DEFAULT_TRIPLET} AND NOT DEFINED VCPKG_TARGET_TRIPLET)
  set(VCPKG_TARGET_TRIPLET "$ENV{VCPKG_DEFAULT_TRIPLET}" CACHE STRING "")
endif()

set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)

find_package(glfw3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(Eigen3 CONFIG REQUIRED)

set(HEADER_FILES
gl3w.c
imconfig.h
imgui.h
imgui_impl_glfw.h
imgui_impl_opengl3.h
imgui_internal.h
imstb_rectpack.h
imstb_textedit.h
imstb_truetype.h
portable_file_dialogs.h

gl_eigen.h
ebo.h
vbo.h
vao.h
shader_program.h
transform_feedback.h
texture2d.h
texture3d.h
texturebuffer.h
fbo.h
camera.h
gl_viewer.h
mframe.h
mesh.h
)

set(SOURCE_FILES
gl3w.c
imgui.cpp
imgui_draw.cpp
imgui_impl_glfw.cpp
imgui_impl_opengl3.cpp
imgui_widgets.cpp

vao.cpp
gl_eigen.cpp
shader_program.cpp
transform_feedback.cpp
texture2d.cpp
texture3d.cpp
texturebuffer.cpp
fbo.cpp
camera.cpp
gl_viewer.cpp
mesh.cpp
)

if (WIN32 OR APPLE)
    set (LIBS "Eigen3::Eigen;glfw;assimp::assimp;${CMAKE_DL_LIBS}")
else()
    set (LIBS "Eigen3::Eigen;glfw;${ASSIMP_LIBRARIES};${CMAKE_DL_LIBS} -lpthread")
endif()


add_library(${PROJECT_NAME} STATIC ${HEADER_FILES} ${SOURCE_FILES})

target_compile_definitions(${PROJECT_NAME} PUBLIC "_USE_MATH_DEFINES;NOMINMAX")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_SOURCE_DIR}/..")
#target_include_directories ( ${PROJECT_NAME} PRIVATE  ${ASSIMP_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBS})
