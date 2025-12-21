cmake_minimum_required(VERSION 3.16)

project(test64bit VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Charts Quick3D)
find_package(Qt6 REQUIRED COMPONENTS Core)

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
    #QMLContent/customerAnalyse3.qml
)

file(GLOB_RECURSE QML_SOURCES
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    QMLContent/*.qml
)

qt_add_qml_module(apptest64bit_module
    URI test64bit
    # URI Custom3D
    QML_FILES
        Main.qml
        ${QML_SOURCES}
)


# ... (ターゲットプロパティはそのまま) ...
set_target_properties(apptest64bit PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# --- リンク設定 ---
# 実行ファイルに両方のモジュールターゲットと必要なQtライブラリをリンク
target_link_libraries(apptest64bit
    PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Quick
    Qt6::Charts
    Qt6::Quick3D
    apptest64bit_module
)
target_link_libraries(apptest64bit PRIVATE Qt6::Core)

include(GNUInstallDirs)
install(DIRECTORY ${QT_QML_DIR}/QtQuick3D
        DESTINATION qml/QtQuick3D)
install(DIRECTORY ${QT_QML_DIR}/QtQuick3D/AssetUtils
        DESTINATION qml/QtQuick3D/AssetUtils)
install(TARGETS apptest64bit
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


add_custom_command(
    TARGET apptest64bit_module POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/QMLContent
            ${CMAKE_CURRENT_BINARY_DIR}/QMLContent
)
