cmake_minimum_required(VERSION 3.10)

include(GNUInstallDirs)

project(iotsend
	VERSION 0.1
    DESCRIPTION "Utility to send messages to the IOTHub service"
)

set(CMAKE_C_STANDARD 99)

add_executable( ${PROJECT_NAME}
	src/iotsend.c
)

target_include_directories( ${PROJECT_NAME}
	PRIVATE inc
)

target_link_libraries( ${PROJECT_NAME}
	iotclient
	${LIB_RT}
	varserver
)

install(TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
