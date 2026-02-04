REM 例子路径，按你实际的改
REM exe 在：
REM   D:\MagicMirror\git\qt_test\test64bit\build\release\test64bit.exe
REM 我们想要一个干净目录：
REM   D:\MagicMirror\git\qt_test\test64bit\deploy

pushd "C:\Qt\6.7.3\msvc2022_64\bin"

windeployqt ^
  --release ^
  --qmldir D:\MagicMirror\git\qt_test\test64bit ^
  --dir D:\MagicMirror\git\qt_test\test64bit\deploy ^
  D:\MagicMirror\git\qt_test\test64bit\build\Desktop_Qt_6_7_3_MSVC2022_64bit-Release\apptest64bit.exe

copy D:\MagicMirror\git\qt_test\test64bit\build\Desktop_Qt_6_7_3_MSVC2022_64bit-Release\apptest64bit.exe D:\MagicMirror\git\qt_test\test64bit\deploy 
xcopy D:\MagicMirror\git\qt_test\test64bit\QMLContent D:\MagicMirror\git\qt_test\test64bit\deploy\QMLContent  /E /I /Y
REM 64-bit OpenCV (release)
copy /Y "%~dp0libs\opencv_world343.dll" "%~dp0deploy\"

popd