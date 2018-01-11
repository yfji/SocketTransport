cmake_minimum_required(VERSION 3.2)
project(SocketTransport)

include_directories(
	src
	/usr/local/include
	/usr/include/mysql
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/build)

link_directories(
	/usr/local/lib
	/usr/local/lib/opencv_310
	/usr/lib/x86_64-linux-gnu
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}
	opencv_core
	opencv_highgui
	opencv_imgproc
	opencv_imgcodecs
	opencv_videoio
	mysqlclient
	hiredis
	z
	m
	dl
	pthread
)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCE_FILES)

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/build)



