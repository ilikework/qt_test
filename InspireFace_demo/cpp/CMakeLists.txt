cmake_minimum_required(VERSION 3.16)
project(insightface_cpp_demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "insightface_cpp_demo requires x64 toolchain.")
endif()

set(DEMO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(ORT_ROOT "${DEMO_ROOT}/third_party/onnxruntime"
    CACHE PATH "ONNX Runtime root (include + lib)")

if(NOT OpenCV_ROOT)
    if(EXISTS "${DEMO_ROOT}/third_party/opencv/build/include/opencv2/opencv.hpp")
        set(OpenCV_ROOT "${DEMO_ROOT}/third_party/opencv/build")
    elseif(EXISTS "D:/opencv/build/include/opencv2/opencv.hpp")
        set(OpenCV_ROOT "D:/opencv/build")
    elseif(EXISTS "C:/opencv/build/include/opencv2/opencv.hpp")
        set(OpenCV_ROOT "C:/opencv/build")
    else()
        set(OpenCV_ROOT "D:/opencv/build" CACHE PATH "OpenCV build root (same as LibFA64)")
    endif()
endif()
set(OpenCV_INCLUDE_DIRS "${OpenCV_ROOT}/include")
set(OpenCV_LIB_DIR "${OpenCV_ROOT}/x64/vc15/lib")

if(NOT EXISTS "${ORT_ROOT}/include/onnxruntime_cxx_api.h")
    message(FATAL_ERROR "ONNX Runtime not found at ${ORT_ROOT}. Run scripts/setup_deps.ps1 first.")
endif()
if(NOT EXISTS "${OpenCV_INCLUDE_DIRS}/opencv2/opencv.hpp")
    message(FATAL_ERROR "OpenCV not found at ${OpenCV_ROOT}. Set -DOpenCV_ROOT=... (LibFA64 uses D:/opencv/build)")
endif()
if(NOT EXISTS "${OpenCV_LIB_DIR}/opencv_world343.lib")
    message(FATAL_ERROR "opencv_world343.lib not found in ${OpenCV_LIB_DIR}")
endif()

add_executable(insightface_cpp_demo
    main.cpp
    scrfd_onnx.cpp
    landmark106_onnx.cpp
)

target_include_directories(insightface_cpp_demo PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${ORT_ROOT}/include"
    "${OpenCV_INCLUDE_DIRS}"
)

target_link_directories(insightface_cpp_demo PRIVATE "${ORT_ROOT}/lib")
target_link_libraries(insightface_cpp_demo PRIVATE
    onnxruntime.lib
    "$<$<CONFIG:Debug>:${OpenCV_LIB_DIR}/opencv_world343d.lib>"
    "$<$<NOT:$<CONFIG:Debug>>:${OpenCV_LIB_DIR}/opencv_world343.lib>"
)

if(MSVC)
    target_compile_options(insightface_cpp_demo PRIVATE /W3 /EHsc /utf-8)
    set_property(TARGET insightface_cpp_demo PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

set_target_properties(insightface_cpp_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

set(OPENCV_DLL_CANDIDATES
    "${OpenCV_ROOT}/x64/vc15/bin/opencv_world343.dll"
    "${DEMO_ROOT}/../test64bit/libs/opencv_world343.dll"
)

set(OPENCV_DLL "")
foreach(cand IN LISTS OPENCV_DLL_CANDIDATES)
    if(EXISTS "${cand}")
        set(OPENCV_DLL "${cand}")
        break()
    endif()
endforeach()

add_custom_command(TARGET insightface_cpp_demo POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:insightface_cpp_demo>/demo_output"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ORT_ROOT}/lib/onnxruntime.dll"
        "$<TARGET_FILE_DIR:insightface_cpp_demo>"
    COMMENT "Copy onnxruntime.dll to output dir"
)

if(OPENCV_DLL)
    add_custom_command(TARGET insightface_cpp_demo POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${OPENCV_DLL}"
            "$<TARGET_FILE_DIR:insightface_cpp_demo>"
        COMMENT "Copy opencv_world343.dll to output dir"
    )
endif()

message(STATUS "ORT_ROOT: ${ORT_ROOT}")
message(STATUS "OpenCV: ${OpenCV_ROOT}")
