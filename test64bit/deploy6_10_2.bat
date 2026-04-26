chcp 65001 >nul
@echo off
setlocal

REM --- 配置 ---
SET "PROJECT_ROOT=%~dp0"
SET "BUILD_DIR=%PROJECT_ROOT%build\Desktop_Qt_6_10_2_MSVC2022_64bit-Release"
REM 部署目录：当前目录(test64bit)的上一级目录下的 deploy
SET "DEPLOY_DIR=%PROJECT_ROOT%..\deploy"
SET "QT_BIN_DIR=C:\Qt\6.10.2\msvc2022_64\bin"
SET "APP_EXE_NAME=apptest64bit.exe"

REM 使用 UTF-8 编码，与文件编码保持一致，避免中文输出乱码

REM --- 1. 清理并创建部署目录 ---
if exist "%DEPLOY_DIR%" (
    echo Cleaning old deployment directory: %DEPLOY_DIR%
    rmdir /s /q "%DEPLOY_DIR%"
)
mkdir "%DEPLOY_DIR%"
echo Created clean deployment directory.
echo.

REM --- 2. 运行 windeployqt 收集 Qt 依赖 ---
echo Running windeployqt to gather Qt dependencies...
"%QT_BIN_DIR%\windeployqt.exe" ^
  --release ^
  --qmldir "%PROJECT_ROOT%QMLContent" ^
  --dir "%DEPLOY_DIR%" ^
  "%BUILD_DIR%\%APP_EXE_NAME%"
echo.

REM --- 3. 复制主程序和所有第三方依赖 ---
echo Copying application and all other dependencies...
copy /Y "%BUILD_DIR%\%APP_EXE_NAME%" "%DEPLOY_DIR%\"
copy /Y "%PROJECT_ROOT%libs\opencv_world343.dll" "%DEPLOY_DIR%\"
copy /Y "%PROJECT_ROOT%libs\dllProc.dll" "%DEPLOY_DIR%\"
if exist "%PROJECT_ROOT%..\MMCameraCtrl\Release\MMCameraCtrl.exe" copy /Y "%PROJECT_ROOT%..\MMCameraCtrl\Release\MMCameraCtrl.exe" "%DEPLOY_DIR%\"
if exist "%PROJECT_ROOT%..\MMCameraCtrl\Release\EDSDK.dll" copy /Y "%PROJECT_ROOT%..\MMCameraCtrl\Release\EDSDK.dll" "%DEPLOY_DIR%\"
if exist "%PROJECT_ROOT%..\MMCameraCtrl\Release\EdsImage.dll" copy /Y "%PROJECT_ROOT%..\MMCameraCtrl\Release\EdsImage.dll" "%DEPLOY_DIR%\"
if exist "%PROJECT_ROOT%..\MMCameraCtrl\Release\kzdsc_880.dll" copy /Y "%PROJECT_ROOT%..\MMCameraCtrl\Release\kzdsc_880.dll" "%DEPLOY_DIR%\"
echo.

REM --- 初始化数据库并复制到上一级 deploy 目录 ---
echo Initializing and copying database...
pushd "%PROJECT_ROOT%init_files"

REM 选择可用的 Python 解释器（python 或 py）
set "PYEXE="
where python >nul 2>&1 && set "PYEXE=python"
if "%PYEXE%"=="" where py >nul 2>&1 && set "PYEXE=py"

if not "%PYEXE%"=="" (
  if exist "MMFace_.db" del /Q "MMFace_.db"
  %PYEXE% init_db.py
   if exist "MMFace_.db" copy /Y "MMFace_.db" "%DEPLOY_DIR%\"
   if exist "MMFace_.json" copy /Y "MMFace_.json" "%DEPLOY_DIR%\"
)

popd

echo.
echo Deployment finished!
pause
