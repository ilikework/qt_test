cmake_minimum_required(VERSION 3.16)
project(LibFA64_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "LibFA64_demo requires x64 toolchain.")
endif()

# 复用已构建的 LibFA64（默认指向 LibFA64 的 Release 输出）
set(LIBFA64_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../LibFA64"
    CACHE PATH "LibFA64 source tree")
set(LIBFA64_LIB_DIR "${LIBFA64_ROOT}/build/lib/Release"
    CACHE PATH "Directory containing LibFA64.lib")
set(LIBFA64_DLL_DIR "${LIBFA64_ROOT}/build/bin/Release"
    CACHE PATH "Directory containing LibFA64.dll")

if(NOT EXISTS "${LIBFA64_LIB_DIR}/LibFA64.lib")
    message(FATAL_ERROR "LibFA64.lib not found in ${LIBFA64_LIB_DIR}. Build LibFA64 first.")
endif()

add_executable(libfa64_demo main.cpp)

set(OPENCV_ROOT "D:/opencv/build" CACHE PATH "OpenCV build root")
set(OPENCV_LIB_DIR "${OPENCV_ROOT}/x64/vc15/lib")

target_include_directories(libfa64_demo PRIVATE "${LIBFA64_ROOT}/include" "${OPENCV_ROOT}/include")
target_link_libraries(libfa64_demo PRIVATE
    "${LIBFA64_LIB_DIR}/LibFA64.lib"
    "$<$<CONFIG:Debug>:${OPENCV_LIB_DIR}/opencv_world343d.lib>"
    "$<$<NOT:$<CONFIG:Debug>>:${OPENCV_LIB_DIR}/opencv_world343.lib>"
)

if(MSVC)
    set_property(TARGET libfa64_demo PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set_target_properties(libfa64_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# 运行目录：复制 DLL 和 OpenCV
set(OPENCV_DLL "${CMAKE_CURRENT_SOURCE_DIR}/../test64bit/libs/opencv_world343.dll")
add_custom_command(TARGET libfa64_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:libfa64_demo>/demo_output"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LIBFA64_DLL_DIR}/LibFA64.dll"
        "$<TARGET_FILE_DIR:libfa64_demo>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${OPENCV_DLL}"
        "$<TARGET_FILE_DIR:libfa64_demo>"
    COMMENT "Copy LibFA64.dll, opencv_world343.dll to demo output dir"
)

message(STATUS "LibFA64_demo links: ${LIBFA64_LIB_DIR}/LibFA64.lib")
