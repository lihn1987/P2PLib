project(libp2p)
#配置boost
CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
SET(Boost_USE_STATIC_LIBS ON)
SET(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 ")
find_package(Boost 1.67.0 COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

#配置glog
LINK_DIRECTORIES(/usr/local/lib)

#配置编译文件
aux_source_directory(${ROOT_PATH}/src SRC )
aux_source_directory(${ROOT_PATH}/demo DEMO)

ADD_LIBRARY(p2p STATIC
	${SRC}	
)
ADD_EXECUTABLE(p2p-demo
	${DEMO}
)
include_directories(${ROOT_PATH}/include)
#target_include_directories(p2p-demo ${ROOT_PATH}/include)
target_link_libraries(p2p-demo boost_system boost_thread)
target_link_libraries(p2p-demo glog)
target_link_libraries(p2p-demo p2p)

