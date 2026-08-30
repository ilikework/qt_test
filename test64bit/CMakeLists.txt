cmake_minimum_required(VERSION 3.16)

project(test64bit VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Sql Gui Quick Charts Quick3D PrintSupport Concurrent)

qt_standard_project_setup()

# --- C++ ?????????? ---
set(APP_SOURCES
    main.cpp
    #mycustom3ditem.cpp
)

set(APP_HEADERS
    #mycustom3ditem.h # ????????????
)

# --- ??????????---
qt_add_executable(apptest64bit
    ${APP_SOURCES}
    ${APP_HEADERS}
    cameraclient.h cameraclient.cpp
    CameraImageProvider.h
    appdb.h appdb.cpp
    appconfig.h appconfig.cpp
    SocketWorker.h SocketWorker.cpp
    FlowBase.h FlowBase.cpp
    CaptureFlow.h CaptureFlow.cpp
    StartPreviewFlow.h StartPreviewFlow.cpp
    CustomerManager.h CustomerManager.cpp
    AnalyseManager.h AnalyseManager.cpp
    MM_Const_Define.h
    BackupManager.h BackupManager.cpp
    PreRecordManager.h PreRecordManager.cpp
    CustomerReportManager.h CustomerReportManager.cpp
    BaseDrawingItem.h BaseDrawingItem.cpp
    ImageEditor.h ImageEditor.cpp
    FaceContourMath.h FaceContourMath.cpp
    FaceAnalyseManager.h FaceAnalyseManager.cpp
    MM3DManager.h MM3DManager.cpp
    MMLogger.h MMLogger.cpp
    PrintHelper.h PrintHelper.cpp
)

# --- Gather all QML and resource files from the QMLContent directory ---
file(GLOB_RECURSE QML_RESOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/QMLContent/*"
)

# --- ?? shader ???? qrc????CustomMaterial ?? Failed to read shader code from :/morph_blend.vert ---
list(APPEND QML_RESOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/QMLContent/morph_blend.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/QMLContent/morph_blend_lit.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/QMLContent/morph_blend_unlit.frag"
)

# --- ? QMLContent ????????????????? ---
# ?? BASE ? PREFIX ???????????
qt_add_resources(apptest64bit "qml_resources"
    BASE ${CMAKE_CURRENT_SOURCE_DIR}/QMLContent
    PREFIX "/"
    FILES ${QML_RESOURCE_FILES}
)

# ShaderEffect???????? qt_add_shaders??? Qt 6.10 + Ninja ? Windows ???????
# ?? qsb/D:/...???? qsb ????? + qt_add_resources
find_program(QT_QSB_EXECUTABLE
    NAMES qsb qsb.exe
    REQUIRED
    HINTS "${Qt6_DIR}/../../../bin" "${Qt6_DIR}/../../bin" "$ENV{QTDIR}/bin"
    DOC "Qt Shader Baker (ShaderEffect .qsb)"
)
set(MM_MAGNIFIER_FRAG "${CMAKE_CURRENT_SOURCE_DIR}/shaders/magnifier_glass.frag")
set(MM_MAGNIFIER_QSB "${CMAKE_CURRENT_BINARY_DIR}/magnifier_glass.frag.qsb")
# --qt6????GLSL(100es/120/150)+HLSL(SM5.0)+MSL?? qt_add_shaders ???????HLSL ??Windows D3D ?? pipeline ??
add_custom_command(
    OUTPUT "${MM_MAGNIFIER_QSB}"
    COMMAND "${QT_QSB_EXECUTABLE}" --qt6 "${MM_MAGNIFIER_FRAG}" -o "${MM_MAGNIFIER_QSB}"
    DEPENDS "${MM_MAGNIFIER_FRAG}"
    COMMENT "Baking magnifier_glass.frag for ShaderEffect (multi-backend)"
    VERBATIM
)
# Qt 6.10+????????????qrc ?????????????? PREFIX+????
set_source_files_properties("${MM_MAGNIFIER_QSB}" PROPERTIES
    QT_RESOURCE_ALIAS "magnifier_glass.frag.qsb"
    GENERATED TRUE
)
qt_add_resources(apptest64bit "mm_magnifier_qsb"
    PREFIX "/shaders"
    FILES "${MM_MAGNIFIER_QSB}"
)


# ... (???????????????? ...
set_target_properties(apptest64bit PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# --- ???? ---
# ?? C4828 ?????????????????????????? MSVC ??
if(MSVC)
    target_compile_options(apptest64bit PRIVATE /wd4828)
endif()

# --- 1. ??????????---
set(LIBFA64_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../LibFA64")
set(LIBFA64_LIB_RELEASE "${LIBFA64_ROOT}/build/lib/Release/LibFA64.lib")
set(LIBFA64_LIB_DEBUG "${LIBFA64_ROOT}/build/Desktop_Qt_6_10_2_MSVC2022_64bit-Debug/lib/LibFA64.lib")
target_include_directories(apptest64bit PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/libs"
    "${LIBFA64_ROOT}/include"
)

# --- ??????---
# ????????????????????????Qt??????????# target_link_libraries(apptest64bit
#     PRIVATE
#     Qt6::Core
#     Qt6::Sql
#     Qt6::Gui
#     Qt6::Quick
#     Qt6::Charts
#     Qt6::Quick3D
#     apptest64bit_module
#     # ?????????? lib
#     "${CMAKE_CURRENT_SOURCE_DIR}/libs/dllProc.lib"
# )
# target_link_libraries(apptest64bit PRIVATE Qt6::Core)
# target_link_libraries(apptest64bit PRIVATE Qt6::GuiPrivate)
target_link_libraries(apptest64bit
    PRIVATE
    Qt6::Core
    Qt6::Sql
    Qt6::Gui
    Qt6::Quick
    Qt6::Charts
    Qt6::Quick3D
    Qt6::PrintSupport
    Qt6::Concurrent
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/dllProc.lib"
    "${LIBFA64_LIB_RELEASE}"
)

# target_link_libraries(apptest64bit_module
#     PRIVATE
#     Qt6::Core Qt6::Gui Qt6::Quick
# )

include(GNUInstallDirs)
install(TARGETS apptest64bit
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

add_custom_command(TARGET apptest64bit POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/dllProc.dll"
        $<TARGET_FILE_DIR:apptest64bit>
)
add_custom_command(TARGET apptest64bit POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/libs/opencv_world343.dll"
        $<TARGET_FILE_DIR:apptest64bit>
)
add_custom_command(TARGET apptest64bit POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/init_files/MMFace_.json"
        $<TARGET_FILE_DIR:apptest64bit>
)
add_custom_command(TARGET apptest64bit POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${LIBFA64_ROOT}/build/bin/Release/LibFA64.dll"
        $<TARGET_FILE_DIR:apptest64bit>
)
