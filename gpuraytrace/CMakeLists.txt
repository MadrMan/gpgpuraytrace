cmake_minimum_required(VERSION 2.6)

MACRO(ADD_PROJECT_PCH fileName)
	if(MSVC) 
		set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/Yu${fileName}.h") 
		set_source_files_properties(${fileName}.cpp PROPERTIES COMPILE_FLAGS "/Yc${fileName}.h")
	endif() 
ENDMACRO() 

MACRO(DELAYLOAD_DX11)
	if(MSVC)
		set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/DELAYLOAD:d3d11.dll /DELAYLOAD:d3dcompiler_43.dll /DELAYLOAD:d3dcompiler_44.dll")
	endif(MSVC)
ENDMACRO()

project(gpgpuraytrace)

set(PROJ_ADAPTER
	${gpgpuraytrace_SOURCE_DIR}/Adapters/DirectoryWinAPI.h
	${gpgpuraytrace_SOURCE_DIR}/Adapters/DirectoryWinAPI.cpp
	${gpgpuraytrace_SOURCE_DIR}/Adapters/WindowWinAPI.h
	${gpgpuraytrace_SOURCE_DIR}/Adapters/WindowWinAPI.cpp
	${gpgpuraytrace_SOURCE_DIR}/Adapters/DeviceDirect3D.h
	${gpgpuraytrace_SOURCE_DIR}/Adapters/DeviceDirect3D.cpp
	${gpgpuraytrace_SOURCE_DIR}/Adapters/ComputeDirect3D.h
	${gpgpuraytrace_SOURCE_DIR}/Adapters/ComputeDirect3D.cpp
	${gpgpuraytrace_SOURCE_DIR}/Adapters/InputWinAPI.h
	${gpgpuraytrace_SOURCE_DIR}/Adapters/InputWinAPI.cpp
	${gpgpuraytrace_SOURCE_DIR}/Adapters/ShaderVariableDirect3D.h
	${gpgpuraytrace_SOURCE_DIR}/Adapters/ShaderVariableDirect3D.cpp
	${gpgpuraytrace_SOURCE_DIR}/Adapters/TimerWinAPI.h
	${gpgpuraytrace_SOURCE_DIR}/Adapters/TimerWinAPI.cpp
	${gpgpuraytrace_SOURCE_DIR}/Adapters/TextureDirect3D.h
	${gpgpuraytrace_SOURCE_DIR}/Adapters/TextureDirect3D.cpp
	${gpgpuraytrace_SOURCE_DIR}/Adapters/RecorderWinAPI.h
	${gpgpuraytrace_SOURCE_DIR}/Adapters/RecorderWinAPI.cpp
)

set(PROJ_FACTORY
	${gpgpuraytrace_SOURCE_DIR}/Factories/Factory.h
	${gpgpuraytrace_SOURCE_DIR}/Factories/WindowFactory.h
	${gpgpuraytrace_SOURCE_DIR}/Factories/WindowFactory.cpp
	${gpgpuraytrace_SOURCE_DIR}/Factories/DeviceFactory.h
	${gpgpuraytrace_SOURCE_DIR}/Factories/DeviceFactory.cpp
	${gpgpuraytrace_SOURCE_DIR}/Factories/RecorderFactory.h
	${gpgpuraytrace_SOURCE_DIR}/Factories/RecorderFactory.cpp
	${gpgpuraytrace_SOURCE_DIR}/Factories/ICompute.h
	${gpgpuraytrace_SOURCE_DIR}/Factories/IWindow.h
	${gpgpuraytrace_SOURCE_DIR}/Factories/IDevice.h
	${gpgpuraytrace_SOURCE_DIR}/Factories/ITexture.h
	${gpgpuraytrace_SOURCE_DIR}/Factories/IRecorder.h
)

set (PROJ_GRAPHICS
	${gpgpuraytrace_SOURCE_DIR}/Graphics/Noise.h
	${gpgpuraytrace_SOURCE_DIR}/Graphics/Noise.cpp
	${gpgpuraytrace_SOURCE_DIR}/Graphics/Camera.h
	${gpgpuraytrace_SOURCE_DIR}/Graphics/Camera.cpp
	${gpgpuraytrace_SOURCE_DIR}/Graphics/IShaderVariable.h
	${gpgpuraytrace_SOURCE_DIR}/Graphics/Terrain.cpp
	${gpgpuraytrace_SOURCE_DIR}/Graphics/Terrain.h
	${gpgpuraytrace_SOURCE_DIR}/Graphics/DDSTextureLoader.cpp
	${gpgpuraytrace_SOURCE_DIR}/Graphics/DDSTextureLoader.h
)

set(PROJ_SOURCES
	${gpgpuraytrace_SOURCE_DIR}/Raytracer.h
	${gpgpuraytrace_SOURCE_DIR}/main.cpp
	${gpgpuraytrace_SOURCE_DIR}/Raytracer.cpp
	${gpgpuraytrace_SOURCE_DIR}/Common.h
	${gpgpuraytrace_SOURCE_DIR}/Common.cpp
	${gpgpuraytrace_SOURCE_DIR}/Resource.h
	${gpgpuraytrace_SOURCE_DIR}/Resource.rc
)

set(PROJ_COMMON
	${gpgpuraytrace_SOURCE_DIR}/Common/Types.h
	${gpgpuraytrace_SOURCE_DIR}/Common/Directory.h
	${gpgpuraytrace_SOURCE_DIR}/Common/Directory.cpp
	${gpgpuraytrace_SOURCE_DIR}/Common/IInput.h
	${gpgpuraytrace_SOURCE_DIR}/Common/IInput.cpp
	${gpgpuraytrace_SOURCE_DIR}/Common/Logger.h
	${gpgpuraytrace_SOURCE_DIR}/Common/Logger.cpp
	${gpgpuraytrace_SOURCE_DIR}/Common/VariableManager.h
	${gpgpuraytrace_SOURCE_DIR}/Common/VariableManager.cpp
	${gpgpuraytrace_SOURCE_DIR}/Common/Timer.h
	${gpgpuraytrace_SOURCE_DIR}/Common/Timer.cpp
	${gpgpuraytrace_SOURCE_DIR}/Common/CRC32.h
	${gpgpuraytrace_SOURCE_DIR}/Common/CRC32.cpp
	${gpgpuraytrace_SOURCE_DIR}/Common/Settings.h
	${gpgpuraytrace_SOURCE_DIR}/Common/VFS.cpp
	${gpgpuraytrace_SOURCE_DIR}/Common/VFS.h
)

set(PROJ_GAMEPLAY
	${gpgpuraytrace_SOURCE_DIR}/Gameplay/Flyby.h
	${gpgpuraytrace_SOURCE_DIR}/Gameplay/Flyby.cpp
)

source_group(Adapters FILES ${PROJ_ADAPTER})
source_group(Factories FILES ${PROJ_FACTORY})
source_group(Common FILES ${PROJ_COMMON})
source_group(Graphics FILES ${PROJ_GRAPHICS})
source_group(Gameplay FILES ${PROJ_GAMEPLAY})
source_group("" FILES ${PROJ_SOURCES})

include_directories("${gpgpuraytrace_SOURCE_DIR}")

if(MSVC)
	add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
	add_definitions(-D_WIN32_WINNT=0x601)
endif()
if(CMAKE_COMPILER_IS_GNUCXX)
	add_definitions(-std=gnu++11)
endif()
if(MSVC_VERSION LESS 1700)
	add_definitions(-DRT_HAS_DXSDK)
	include_directories("$ENV{DXSDK_DIR}/include")
	link_directories("$ENV{DXSDK_DIR}/lib/x86")
endif()
link_libraries(d3d11.lib d3dcompiler.lib dxguid.lib ws2_32.lib mfreadwrite.lib mfplat.lib mfuuid.lib dxgi.lib)

add_executable(gpgpuraytrace ${PROJ_SOURCES} ${PROJ_ADAPTER} ${PROJ_FACTORY} ${PROJ_GRAPHICS} ${PROJ_COMMON} ${PROJ_GAMEPLAY})

ADD_PROJECT_PCH("Common")
DELAYLOAD_DX11()
