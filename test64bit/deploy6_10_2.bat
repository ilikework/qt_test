chcp 65001 >nul
@echo off
setlocal EnableExtensions

REM --- 配置 ---
SET "PROJECT_ROOT=%~dp0"
SET "BUILD_DIR=%PROJECT_ROOT%build\Desktop_Qt_6_10_2_MSVC2022_64bit-Release"
REM 部署目录：test64bit 的上一级 deploy（路径不要用 "xxx\" 结尾，会吞掉引号导致 if/else 乱套）
for %%I in ("%PROJECT_ROOT%..\deploy") do SET "DEPLOY_DIR=%%~fI"
SET "QT_BIN_DIR=C:\Qt\6.10.2\msvc2022_64\bin"
SET "APP_EXE_NAME=apptest64bit.exe"
for %%I in ("%PROJECT_ROOT%..\LibFA64\build\bin\Release\LibFA64.dll") do SET "LIBFA64_DLL=%%~fI"
SET "ERR=0"

if not exist "%BUILD_DIR%\%APP_EXE_NAME%" (
    echo [ERROR] Release 主程序不存在: "%BUILD_DIR%\%APP_EXE_NAME%"
    echo 请先用 Qt Creator / CMake 编译 Desktop_Qt_6_10_2_MSVC2022_64bit-Release。
    set "ERR=1"
    goto :finish
)

if not exist "%QT_BIN_DIR%\windeployqt.exe" (
    echo [ERROR] 找不到 windeployqt: "%QT_BIN_DIR%\windeployqt.exe"
    set "ERR=1"
    goto :finish
)

REM --- 1. 清理并创建部署目录 ---
if exist "%DEPLOY_DIR%" (
    echo Cleaning old deployment directory: %DEPLOY_DIR%
    rmdir /s /q "%DEPLOY_DIR%"
)
mkdir "%DEPLOY_DIR%"
echo Created clean deployment directory: %DEPLOY_DIR%
echo.

REM --- 2. 运行 windeployqt 收集 Qt 依赖 ---
echo Running windeployqt to gather Qt dependencies...
"%QT_BIN_DIR%\windeployqt.exe" ^
  --release ^
  --qmldir "%PROJECT_ROOT%QMLContent" ^
  --dir "%DEPLOY_DIR%" ^
  "%BUILD_DIR%\%APP_EXE_NAME%"
if errorlevel 1 (
    echo [ERROR] windeployqt failed.
    set "ERR=1"
    goto :finish
)
echo.

REM --- 3. 复制主程序和所有第三方依赖 ---
echo Copying application and all other dependencies...
copy /Y "%BUILD_DIR%\%APP_EXE_NAME%" "%DEPLOY_DIR%" >nul
if errorlevel 1 (
    echo [ERROR] 复制 %APP_EXE_NAME% 失败
    set "ERR=1"
)

if exist "%PROJECT_ROOT%libs\opencv_world343.dll" (
    copy /Y "%PROJECT_ROOT%libs\opencv_world343.dll" "%DEPLOY_DIR%" >nul
) else (
    echo [ERROR] 缺少 opencv_world343.dll
    set "ERR=1"
)
if exist "%PROJECT_ROOT%libs\dllProc.dll" (
    copy /Y "%PROJECT_ROOT%libs\dllProc.dll" "%DEPLOY_DIR%" >nul
) else (
    echo [ERROR] 缺少 dllProc.dll
    set "ERR=1"
)

REM 皮肤分析 / 自动轮廓：LibFA64
set "LIBFA_SRC="
if exist "%LIBFA64_DLL%" set "LIBFA_SRC=%LIBFA64_DLL%"
if not defined LIBFA_SRC if exist "%BUILD_DIR%\LibFA64.dll" set "LIBFA_SRC=%BUILD_DIR%\LibFA64.dll"
if defined LIBFA_SRC (
    copy /Y "%LIBFA_SRC%" "%DEPLOY_DIR%\LibFA64.dll" >nul
    echo   + LibFA64.dll
    echo     from %LIBFA_SRC%
) else (
    echo [ERROR] 缺少 LibFA64.dll
    echo         期望: %LIBFA64_DLL%
    echo         或: %BUILD_DIR%\LibFA64.dll
    echo         请先编译 LibFA64 Release，并编译一次 Release 主程序。
    set "ERR=1"
)

REM dlib 68 点模型
set "LANDMARK_DAT="
if exist "%PROJECT_ROOT%libs\shape_predictor_68_face_landmarks.dat" set "LANDMARK_DAT=%PROJECT_ROOT%libs\shape_predictor_68_face_landmarks.dat"
if not defined LANDMARK_DAT if exist "%BUILD_DIR%\shape_predictor_68_face_landmarks.dat" set "LANDMARK_DAT=%BUILD_DIR%\shape_predictor_68_face_landmarks.dat"
if not defined LANDMARK_DAT if exist "%PROJECT_ROOT%build\Desktop_Qt_6_10_2_MSVC2022_64bit-Debug\shape_predictor_68_face_landmarks.dat" set "LANDMARK_DAT=%PROJECT_ROOT%build\Desktop_Qt_6_10_2_MSVC2022_64bit-Debug\shape_predictor_68_face_landmarks.dat"
if not defined LANDMARK_DAT if exist "%PROJECT_ROOT%..\LibFA64_demo\build\bin\Release\shape_predictor_68_face_landmarks.dat" set "LANDMARK_DAT=%PROJECT_ROOT%..\LibFA64_demo\build\bin\Release\shape_predictor_68_face_landmarks.dat"
if defined LANDMARK_DAT (
    copy /Y "%LANDMARK_DAT%" "%DEPLOY_DIR%\shape_predictor_68_face_landmarks.dat" >nul
    echo   + shape_predictor_68_face_landmarks.dat
) else (
    echo [ERROR] 缺少 shape_predictor_68_face_landmarks.dat
    echo         请放到 test64bit\libs\ 或 Release/Debug 输出目录后再部署。
    set "ERR=1"
)

if exist "%PROJECT_ROOT%..\MMCameraCtrl\Release\MMCameraCtrl.exe" copy /Y "%PROJECT_ROOT%..\MMCameraCtrl\Release\MMCameraCtrl.exe" "%DEPLOY_DIR%" >nul
if exist "%PROJECT_ROOT%..\MMCameraCtrl\Release\EDSDK.dll" copy /Y "%PROJECT_ROOT%..\MMCameraCtrl\Release\EDSDK.dll" "%DEPLOY_DIR%" >nul
if exist "%PROJECT_ROOT%..\MMCameraCtrl\Release\EdsImage.dll" copy /Y "%PROJECT_ROOT%..\MMCameraCtrl\Release\EdsImage.dll" "%DEPLOY_DIR%" >nul
if exist "%PROJECT_ROOT%..\MMCameraCtrl\Release\kzdsc_880.dll" copy /Y "%PROJECT_ROOT%..\MMCameraCtrl\Release\kzdsc_880.dll" "%DEPLOY_DIR%" >nul
echo.

REM --- 4. 配置与数据库 ---
echo Copying config and initializing database...
if exist "%PROJECT_ROOT%init_files\MMFace_.json" (
    copy /Y "%PROJECT_ROOT%init_files\MMFace_.json" "%DEPLOY_DIR%\MMFace_.json" >nul
    echo   + MMFace_.json
) else (
    echo [ERROR] 缺少 init_files\MMFace_.json
    set "ERR=1"
)

pushd "%PROJECT_ROOT%init_files"

set "PYEXE="
where python >nul 2>&1 && set "PYEXE=python"
if "%PYEXE%"=="" where py >nul 2>&1 && set "PYEXE=py"

if not "%PYEXE%"=="" (
    if exist "MMFace_.db" del /Q "MMFace_.db"
    %PYEXE% init_db.py
    if exist "MMFace_.db" (
        copy /Y "MMFace_.db" "%DEPLOY_DIR%\MMFace_.db" >nul
        echo   + MMFace_.db
    ) else (
        echo [WARN] init_db.py 未生成 MMFace_.db
        set "ERR=1"
    )
) else (
    echo [WARN] 未找到 Python，跳过重新生成 MMFace_.db
    if exist "MMFace_.db" (
        copy /Y "MMFace_.db" "%DEPLOY_DIR%\MMFace_.db" >nul
        echo   + MMFace_.db ^(existing^)
    ) else (
        echo [ERROR] 无 Python 且无现成 MMFace_.db
        set "ERR=1"
    )
)

popd

echo.
echo --- Deploy contents ^(key files^) ---
if exist "%DEPLOY_DIR%\%APP_EXE_NAME%" (echo   OK  %APP_EXE_NAME%) else (echo   MISSING %APP_EXE_NAME% & set "ERR=1")
if exist "%DEPLOY_DIR%\LibFA64.dll" (echo   OK  LibFA64.dll) else (echo   MISSING LibFA64.dll & set "ERR=1")
if exist "%DEPLOY_DIR%\opencv_world343.dll" (echo   OK  opencv_world343.dll) else (echo   MISSING opencv_world343.dll & set "ERR=1")
if exist "%DEPLOY_DIR%\shape_predictor_68_face_landmarks.dat" (echo   OK  shape_predictor_68_face_landmarks.dat) else (echo   MISSING shape_predictor_68_face_landmarks.dat & set "ERR=1")
if exist "%DEPLOY_DIR%\MMFace_.json" (echo   OK  MMFace_.json) else (echo   MISSING MMFace_.json & set "ERR=1")
if exist "%DEPLOY_DIR%\MMFace_.db" (echo   OK  MMFace_.db) else (echo   MISSING MMFace_.db & set "ERR=1")
echo.
echo 提示: FaceReconCPU.exe 路径由 MMFace_.json 的 FaceReconExePath 指定，不随本脚本打包。
echo       目标机上请保持该绝对路径可用，或改 JSON 后部署。
echo.

:finish
if "%ERR%"=="0" (
    echo Deployment finished OK: %DEPLOY_DIR%
) else (
    echo Deployment finished with ERRORS: %DEPLOY_DIR%
)
pause
endlocal
exit /b %ERR%
