project(libp2p)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
SET(Boost_USE_STATIC_LIBS ON)
SET(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

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
target_link_libraries(p2p-demo p2p)
