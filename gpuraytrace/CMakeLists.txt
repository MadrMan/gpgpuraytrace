cmake_minimum_required(VERSION 2.4)

project(gpgpuraytrace)

set(PROJ_ADAPTER
	${gpgpuraytrace_SOURCE_DIR}/WindowWinAPI.h
	${gpgpuraytrace_SOURCE_DIR}/WindowWinAPI.cpp
	${gpgpuraytrace_SOURCE_DIR}/DeviceDirect3D.h
	${gpgpuraytrace_SOURCE_DIR}/DeviceDirect3D.cpp
	${gpgpuraytrace_SOURCE_DIR}/ICompute.h
	${gpgpuraytrace_SOURCE_DIR}/ComputeDirect3D.cpp
)

set(PROJ_FACTORY
	${gpgpuraytrace_SOURCE_DIR}/Factory.h
	${gpgpuraytrace_SOURCE_DIR}/WindowFactory.h
	${gpgpuraytrace_SOURCE_DIR}/DeviceFactory.h
	${gpgpuraytrace_SOURCE_DIR}/WindowFactory.cpp
	${gpgpuraytrace_SOURCE_DIR}/DeviceFactory.cpp
)

set(PROJ_SOURCES
	${gpgpuraytrace_SOURCE_DIR}/Common.h
	${gpgpuraytrace_SOURCE_DIR}/Raytracer.h
	${gpgpuraytrace_SOURCE_DIR}/Camera.h
	${gpgpuraytrace_SOURCE_DIR}/main.cpp
	${gpgpuraytrace_SOURCE_DIR}/Raytracer.cpp
	${gpgpuraytrace_SOURCE_DIR}/Camera.cpp
	${PROJ_ADAPTER}
	${PROJ_FACTORY}
)

source_group(Adapter FILES ${PROJ_ADAPTER})
source_group(Factory FILES ${PROJ_FACTORY})

add_executable(gpgpuraytrace ${PROJ_SOURCES})