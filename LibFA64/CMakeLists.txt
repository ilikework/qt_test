cmake_minimum_required(VERSION 3.16)
project(LibFA64 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "LibFA64 requires a 64-bit (x64) toolchain.")
endif()

# ---------- 依赖路径（可用 -D 覆盖） ----------
set(DLIB_ROOT "D:/MagicMirror/svn/DreamMirror/dlib-19.9/dlib"
    CACHE PATH "dlib CMake root (contains CMakeLists.txt)")
set(OpenCV_ROOT "D:/opencv/build"
    CACHE PATH "OpenCV build root (include/ + x64/vc15/lib/)")

if(NOT EXISTS "${OpenCV_ROOT}/include/opencv2/opencv.hpp")
    message(FATAL_ERROR "OpenCV headers not found under OpenCV_ROOT=${OpenCV_ROOT}")
endif()

set(OpenCV_INCLUDE_DIRS "${OpenCV_ROOT}/include")
set(OpenCV_LIB_DIR "${OpenCV_ROOT}/x64/vc15/lib")
if(NOT EXISTS "${OpenCV_LIB_DIR}/opencv_world343.lib")
    message(FATAL_ERROR "opencv_world343.lib not found in ${OpenCV_LIB_DIR}")
endif()

# ---------- dlib ----------
if(NOT EXISTS "${DLIB_ROOT}/CMakeLists.txt")
    message(FATAL_ERROR "dlib not found at DLIB_ROOT=${DLIB_ROOT}")
endif()
set(DLIB_NO_GUI_SUPPORT ON CACHE BOOL "" FORCE)
add_subdirectory("${DLIB_ROOT}" "${CMAKE_BINARY_DIR}/dlib_build" EXCLUDE_FROM_ALL)

add_library(LibFA64 SHARED
    src/libfa_api.cpp
    src/extra_info.cpp
    src/face_detect.cpp
    src/analysis_bridge.cpp
)

target_include_directories(LibFA64
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor
        ${OpenCV_INCLUDE_DIRS}
)

target_compile_definitions(LibFA64 PRIVATE LIBFA64_EXPORTS)

target_link_libraries(LibFA64
    PRIVATE
        dlib::dlib
        "$<$<CONFIG:Debug>:${OpenCV_LIB_DIR}/opencv_world343d.lib>"
        "$<$<NOT:$<CONFIG:Debug>>:${OpenCV_LIB_DIR}/opencv_world343.lib>"
)

if(MSVC)
    target_compile_options(LibFA64 PRIVATE /W3 /EHsc)
    # 与 Qt 6.10 / MSVC 2022 常用设置一致：动态 CRT
    set_property(TARGET LibFA64 PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set_target_properties(LibFA64 PROPERTIES
    OUTPUT_NAME "LibFA64"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# 生成 import lib，供 Qt/其他工程链接
install(TARGETS LibFA64
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION bin
    ARCHIVE DESTINATION lib
)
install(FILES include/LibFA.h DESTINATION include)

message(STATUS "LibFA64 OpenCV include: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "LibFA64 OpenCV lib dir: ${OpenCV_LIB_DIR}")
message(STATUS "LibFA64 dlib: ${DLIB_ROOT}")
