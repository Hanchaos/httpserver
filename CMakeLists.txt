cmake_minimum_required(VERSION 2.8)


include(cmake/hobot_util.cmake)
project(xhttpserver)

set(CMAKE_BUILD_TYPE Release)
message("build types: ${CMAKE_BUILD_TYPE}")
if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x  -fPIC ")
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lzookeeper_mt -lboost_system -lglog -lboost_filesystem -std=c++0x -fPIC ")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x  -DEBUG -fPIC ")
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lzookeeper_mt -lboost_system -lglog -lboost_filesystem -std=c++0x -DEBUG -fPIC ")
endif ()

include_directories(
        include
        src
)

file(GLOB PROJECT_INCLUDE "include/xhttpserver/*.hpp")

file(GLOB PROJECT_SRC "src/*/*.cpp")

set(SOURCE_FILES ${PROJECT_SRC} ${PROJECT_INCLUDE})

find_library(glog libglog.a)

add_library(xhttpserver SHARED ${SOURCE_FILES})

add_executable(demo ${SOURCE_FILES})

target_link_libraries(
        demo
        xhttpserver
        glog
	    pthread
        boost_system
		zookeeper_mt
)
#add_subdirectory(test)

set(OUTPUT_ROOT ${PROJECT_SOURCE_DIR}/output)

set(MY_OUTPUT_ROOT ${OUTPUT_ROOT}/xhttpserver)
install(TARGETS xhttpserver
        DESTINATION ${MY_OUTPUT_ROOT}/lib)
install(FILES ${PROJECT_SOURCE_DIR}/include/xhttpserver/api.hpp
        ${PROJECT_SOURCE_DIR}/include/xhttpserver/config.hpp
        ${PROJECT_SOURCE_DIR}/include/xhttpserver/connection.hpp
        ${PROJECT_SOURCE_DIR}/include/xhttpserver/connection_manager.hpp
        ${PROJECT_SOURCE_DIR}/include/xhttpserver/control.hpp
        ${PROJECT_SOURCE_DIR}/include/xhttpserver/filter.hpp
        ${PROJECT_SOURCE_DIR}/include/xhttpserver/handler.hpp
        ${PROJECT_SOURCE_DIR}/include/xhttpserver/header.hpp
        ${PROJECT_SOURCE_DIR}/include/xhttpserver/header_parser.hpp
        ${PROJECT_SOURCE_DIR}/include/xhttpserver/request.hpp
        ${PROJECT_SOURCE_DIR}/include/xhttpserver/response.hpp
        ${PROJECT_SOURCE_DIR}/include/xhttpserver/router.hpp
        ${PROJECT_SOURCE_DIR}/include/xhttpserver/server.hpp
        ${PROJECT_SOURCE_DIR}/include/xhttpserver/static_handler.hpp
		${PROJECT_SOURCE_DIR}/include/xhttpserver/thread_pool.hpp
		${PROJECT_SOURCE_DIR}/include/xhttpserver/master_redirect.hpp
		${PROJECT_SOURCE_DIR}/include/xhttpserver/server_discovery.hpp
        DESTINATION ${MY_OUTPUT_ROOT}/include/xhttpserver/)
