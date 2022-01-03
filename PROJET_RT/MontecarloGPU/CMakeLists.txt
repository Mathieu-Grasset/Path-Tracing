cmake_minimum_required(VERSION 3.0)
project(MontecarloGPU LANGUAGES CXX)

add_executable(${PROJECT_NAME} montecarlo.cpp )

target_link_libraries(${PROJECT_NAME} bvh_gpu)

target_compile_definitions(${PROJECT_NAME}
		PRIVATE
		-DSHADER_PATH=std::string\(\"${SHADER_PATH}/\"\)
		-DTP_SHADER_PATH=std::string\(\"${TP_SHADER_PATH}/\"\)
		-DDATA_PATH=std::string\(\"${SYS_DATA_PATH}/\"\)
		)