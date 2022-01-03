# EasyCppOGL

## Compilation

### Dependencies

* eigen3
* glfw3
* assimp

#### Linux

#### Windows

	Use vckpg
	Use triplet x64-windows (var env: VCPKG_DEFAULT_TRIPLET=x64-windows)

	Use cmake-gui version:
	Generator Visual Studio 15 2017
	Optional platform: x64
	Specificy toolchain file : C:\VCPKG\scripts\buildsystems\vcpkg.cmake
	or by command line
		cmake.exe ..\easycppogl\ -DCMAKE_GENERATOR_PLATFORM=x64 -DCMAKE_TOOLCHAIN_FILE="C:\VCPKG\scripts\buildsystems\vcpkg.cmake"

	For QtCreator
	* add dir of jom.exe in path var (it is in QtCreator/bin)
	* launch VS Tool x64 command shell 
	* create build dir and go into.
	* cmake.exe ..\easycppogl\ -DCMAKE_TOOLCHAIN_FILE="C:/VCPKG/scripts/buildsystems/vcpkg.cmake" -G "NMake Makefiles JOM"
	* in QtCreator import the build dir
