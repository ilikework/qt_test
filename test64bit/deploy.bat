chcp 65001 >nul
REM 例子路径，按你实际的改
REM exe 在：
REM   D:\MagicMirror\git\qt_test\test64bit\build\release\test64bit.exe
REM 我们想要一个干净目录：
REM   D:\MagicMirror\git\qt_test\deploy

pushd "C:\Qt\6.7.3\msvc2022_64\bin"

REM 使用 UTF-8 编码，与文件编码保持一致，避免中文输出乱码


windeployqt ^
  --release ^
  --qmldir D:\MagicMirror\git\qt_test\test64bit ^
  --dir D:\MagicMirror\git\qt_test\deploy ^
  D:\MagicMirror\git\qt_test\test64bit\build\Desktop_Qt_6_7_3_MSVC2022_64bit-Release\apptest64bit.exe

copy D:\MagicMirror\git\qt_test\test64bit\build\Desktop_Qt_6_7_3_MSVC2022_64bit-Release\apptest64bit.exe D:\MagicMirror\git\qt_test\deploy
xcopy D:\MagicMirror\git\qt_test\test64bit\QMLContent D:\MagicMirror\git\qt_test\deploy\QMLContent  /E /I /Y
REM 64-bit OpenCV (release)
if not exist "..\deploy\" mkdir "..\deploy\"
copy /Y "D:\MagicMirror\git\qt_test\test64bit\libs\opencv_world343.dll" "D:\MagicMirror\git\qt_test\deploy\"

REM 复制 dllProc.dll
if exist "%~dp0libs\dllProc.dll" (
  echo 复制 dllProc.dll 到 deploy ...
  copy /Y "%~dp0libs\dllProc.dll" "%~dp0..\deploy\"
) else (
  echo 未找到 dllProc.dll，跳过复制。
)


REM --- 初始化数据库并复制到上一级 deploy 目录 ---
pushd "%~dp0init_files"

REM 选择可用的 Python 解释器（python 或 py）
set "PYEXE="
where python >nul 2>&1 && set "PYEXE=python"
if "%PYEXE%"=="" where py >nul 2>&1 && set "PYEXE=py"

if "%PYEXE%"=="" (
  echo Python 未找到，跳过数据库初始化（请确保 PATH 中有 python 或 py）。
) else (
  echo 使用 %PYEXE% 运行 init_db.py ...
  if exist "MMFace_.db" del /Q "MMFace_.db"
  %PYEXE% init_db.py
  if exist "MMFace_.db" (
    copy /Y "MMFace_.db" "%~dp0..\deploy\MMFace_.db"
    echo 已复制 MMFace_.db 到 %~dp0..\deploy\
  ) else (
    echo MMFace_.db 未生成，跳过复制。
  )
REM 复制 MMFace_.json
  if exist "MMFace_.json" (
    echo 复制 MMFace_.json 到 deploy ...
    copy /Y "MMFace_.json" "%~dp0..\deploy\"
  ) else (
    echo 未找到 MMFace_.json，跳过复制。
  )
  
)

popd

REM --- 复制后端可执行与依赖到 deploy 目录（如果存在） ---
if exist "%~dp0..\MMCameraCtrl\Release\MMCameraCtrl.exe" (
  echo 复制 MMCameraCtrl.exe 到 deploy ...
  copy /Y "%~dp0..\MMCameraCtrl\Release\MMCameraCtrl.exe" "%~dp0..\deploy\"
) else (
  echo 未找到 MMCameraCtrl.exe，跳过复制。
)

if exist "%~dp0..\MMCameraCtrl\Release\EDSDK.dll" (
  echo 复制 EDSDK.dll 到 deploy ...
  copy /Y "%~dp0..\MMCameraCtrl\Release\EDSDK.dll" "%~dp0..\deploy\"
) else (
  echo 未找到 EDSDK.dll，跳过复制。
)

if exist "%~dp0..\MMCameraCtrl\Release\EdsImage.dll" (
  echo 复制 EdsImage.dll 到 deploy ...
  copy /Y "%~dp0..\MMCameraCtrl\Release\EdsImage.dll" "%~dp0..\deploy\"
) else (
  echo 未找到 EdsImage.dll，跳过复制。
)

popd
