cmake_minimum_required(VERSION 3.8)
project(leastprivelegetask)

set(CMAKE_C_STANDARD 90)

set(SOURCE_FILES src/logger/main.c
				 src/config.c
				 src/daemonize.c
				 src/err_handling.c
				 src/ini.c
				 src/interface_listener.c
				 src/listen_changes.c
				 src/signal_queue.c)
include_directories(./include)
set(EXECUTABLE_NAME leastprivelegetask)
add_executable(leastprivelegetask ${SOURCE_FILES})

find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIRS})
    link_directories(${OPENSSL_LIBRARIES})
    message(STATUS "OpenSSL version: ${OPENSSL_VERSION}")
    target_link_libraries(leastprivelegetask ${OPENSSL_LIBRARIES})
endif()

find_package(Threads REQUIRED)
target_link_libraries(leastprivelegetask ${CMAKE_THREAD_LIBS_INIT})

# Needed for signal handling with F_SETSIG
add_definitions(-D_GNU_SOURCE=1	)

# INIH configuration
add_definitions(-DINI_USE_STACK=0)

# Interface Listener's definitions
add_definitions(-D_INTERFACE_LISTENER_BACKLOG=5)

# Project configuration
add_definitions(-D_EXECUTABLE_NAME="${EXECUTABLE_NAME}")
add_definitions(-D_SERVICETASK_USE_SIGNALS=1)
