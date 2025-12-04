cmake_minimum_required(VERSION 3.16)

project(test64bit VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Quick Charts Quick3D OpenGL)

qt_standard_project_setup()

# --- C++ ソースファイルを定義 ---
set(APP_SOURCES
    main.cpp
    mycustom3ditem.cpp # ここに C++ ファイルを含める
)

set(APP_HEADERS
    mycustom3ditem.h # ここにヘッダーを含める
)

# --- 実行ファイルの定義 ---
qt_add_executable(apptest64bit
    ${APP_SOURCES}
    ${APP_HEADERS}
    QMLContent/customerAnalyse3.qml
)

# --- 1. アプリ本体の QML モジュール定義 ---
qt_add_qml_module(apptest64bit_module
    URI test64bit
    # URI Custom3D
    QML_FILES
        Main.qml
        QMLContent/backupAndRestore.qml
        QMLContent/components/MMCheckBox.qml
        QMLContent/components/MMRadioButton.qml
)

# --- 2. Custom3D メタデータ生成用モジュール定義 ---
# ソースファイルは含めない（空にする）


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
    Qt6::OpenGL
    apptest64bit_module
)

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



