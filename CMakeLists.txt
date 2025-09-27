cmake_minimum_required(VERSION 3.12)
project(imagetoascii VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenCV REQUIRED COMPONENTS core imgcodecs imgproc)

add_library(ascii_lib
	src/ascii_converter.cpp
	src/ascii_reverse.cpp
)
target_include_directories(ascii_lib PUBLIC src)
target_link_libraries(ascii_lib PUBLIC ${OpenCV_LIBS})

add_executable(imagetoascii main.cpp)
target_link_libraries(imagetoascii PRIVATE ascii_lib)

include(CTest)
enable_testing()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
