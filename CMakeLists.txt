project(libp2p)

set(CMAKE_CXX_STANDARD 17)
#配置boost
CMAKE_MINIMUM_REQUIRED(VERSION 2.6 FATAL_ERROR)
SET(Boost_USE_STATIC_LIBS ON)
SET(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})
find_package(Boost 1.67.0 COMPONENTS system filesystem REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

#配置glog
LINK_DIRECTORIES(/usr/local/lib)

#配置qt

set(QT_SRC
    qt-demo/main.cpp
    qt-demo/main_widget.cpp
    qt-demo/main_widget.h
    qt-demo/main_widget.ui
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5 REQUIRED Widgets)

#配置编译文件
aux_source_directory(${ROOT_PATH}/src SRC )
aux_source_directory(${ROOT_PATH}/demo DEMO)
aux_source_directory(${ROOT_PATH}/qt-demo QT_DEMO)

ADD_LIBRARY(p2p STATIC
	${SRC}	
)
ADD_EXECUTABLE(p2p-demo
  ${DEMO}
)
add_executable(p2p-qt-demo  ${QT_SRC})

include_directories(${ROOT_PATH}/include)
#target_include_directories(p2p-demo ${ROOT_PATH}/include)
target_link_libraries(p2p-demo boost_system boost_thread)
target_link_libraries(p2p-demo glog)
target_link_libraries(p2p-demo p2p)
target_link_libraries(p2p-qt-demo Qt5::Widgets)
target_link_libraries(p2p-qt-demo p2p)
target_link_libraries(p2p-qt-demo boost_system boost_thread)
target_link_libraries(p2p-qt-demo glog)
