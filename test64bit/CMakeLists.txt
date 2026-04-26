cmake_minimum_required(VERSION 3.16)

project(test64bit VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Sql Gui Quick Charts Quick3D)

qt_standard_project_setup()

# --- C++ ソースファイルを定義 ---
set(APP_SOURCES
    main.cpp
    #mycustom3ditem.cpp
)

set(APP_HEADERS
    #mycustom3ditem.h # ここにヘッダーを含める

)

# --- 実行ファイルの定義 ---
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
    BaseDrawingItem.h BaseDrawingItem.cpp
    ImageEditor.h ImageEditor.cpp
    MM3DManager.h MM3DManager.cpp
    MMLogger.h MMLogger.cpp
)

# --- Gather all QML and resource files from the QMLContent directory ---
file(GLOB_RECURSE QML_RESOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/QMLContent/*"
)

# --- 变脸 shader 必须打进 qrc，否则 CustomMaterial 会报 Failed to read shader code from :/morph_blend.vert ---
list(APPEND QML_RESOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/QMLContent/morph_blend.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/QMLContent/morph_blend_lit.frag"
    "${CMAKE_CURRENT_SOURCE_DIR}/QMLContent/morph_blend_unlit.frag"
)

# --- 将 QMLContent 目录下的所有文件作为资源编译进程序 ---
# 使用 BASE 和 PREFIX 可以自动处理子目录结构
qt_add_resources(apptest64bit "qml_resources"
    BASE ${CMAKE_CURRENT_SOURCE_DIR}/QMLContent
    PREFIX "/"
    FILES ${QML_RESOURCE_FILES}
)

# ShaderEffect（放大镜）：不用 qt_add_shaders，避免 Qt 6.10 + Ninja 在 Windows 上生成错误路径
# （.qsb/D:/...）。改用 qsb 可执行文件 + qt_add_resources。
find_program(QT_QSB_EXECUTABLE
    NAMES qsb qsb.exe
    REQUIRED
    HINTS "${Qt6_DIR}/../../../bin" "${Qt6_DIR}/../../bin" "$ENV{QTDIR}/bin"
    DOC "Qt Shader Baker (ShaderEffect .qsb)"
)
set(MM_MAGNIFIER_FRAG "${CMAKE_CURRENT_SOURCE_DIR}/shaders/magnifier_glass.frag")
set(MM_MAGNIFIER_QSB "${CMAKE_CURRENT_BINARY_DIR}/magnifier_glass.frag.qsb")
# --qt6：生成 GLSL(100es/120/150)+HLSL(SM5.0)+MSL，与 qt_add_shaders 默认一致；缺 HLSL 时 Windows D3D 会报 pipeline 失败
add_custom_command(
    OUTPUT "${MM_MAGNIFIER_QSB}"
    COMMAND "${QT_QSB_EXECUTABLE}" --qt6 "${MM_MAGNIFIER_FRAG}" -o "${MM_MAGNIFIER_QSB}"
    DEPENDS "${MM_MAGNIFIER_FRAG}"
    COMMENT "Baking magnifier_glass.frag for ShaderEffect (multi-backend)"
    VERBATIM
)
# Qt 6.10+：构建树里的绝对路径进 qrc 时必须设别名，才能在资源里按 PREFIX+别名访问
set_source_files_properties("${MM_MAGNIFIER_QSB}" PROPERTIES
    QT_RESOURCE_ALIAS "magnifier_glass.frag.qsb"
    GENERATED TRUE
)
qt_add_resources(apptest64bit "mm_magnifier_qsb"
    PREFIX "/shaders"
    FILES "${MM_MAGNIFIER_QSB}"
)


# ... (ターゲットプロパティはそのまま) ...
set_target_properties(apptest64bit PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# --- 编译选项 ---
# 禁用 C4828 警告，该警告在将二进制文件（如图片）作为资源编译时由 MSVC 产生
if(MSVC)
    target_compile_options(apptest64bit PRIVATE /wd4828)
endif()

# --- 1. 指定头文件搜索路径 (为了能找到 dllProc.h) ---
target_include_directories(apptest64bit PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libs")

# --- リンク設定 ---
# 実行ファイルに両方のモジュールターゲットと必要なQtライブラリをリンク
# target_link_libraries(apptest64bit
#     PRIVATE
#     Qt6::Core
#     Qt6::Sql
#     Qt6::Gui
#     Qt6::Quick
#     Qt6::Charts
#     Qt6::Quick3D
#     apptest64bit_module
#     # 使用绝对路径直接链接 lib
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
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/dllProc.lib"
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
