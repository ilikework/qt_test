@echo off
setlocal
cd /d "%~dp0"

set "CMAKE_EXE=D:\Qt\Tools\CMake_64\bin\cmake.exe"
if not exist "%CMAKE_EXE%" set "CMAKE_EXE=cmake"

echo === InsightFace C++ demo (ONNX Runtime, Windows) ===
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0scripts\setup_deps.ps1"
if errorlevel 1 exit /b 1

if not exist "cpp\build" mkdir "cpp\build"
cd cpp\build

"%CMAKE_EXE%" .. -G "Visual Studio 16 2019" -A x64 -DOpenCV_ROOT=D:/opencv/build
if errorlevel 1 (
  "%CMAKE_EXE%" .. -G "Visual Studio 17 2022" -A x64 -DOpenCV_ROOT=D:/opencv/build
)
if errorlevel 1 (
  echo FAIL: cmake configure failed. Ensure OpenCV at D:/opencv/build (same as LibFA64)
  exit /b 1
)

"%CMAKE_EXE%" --build . --config Release
if errorlevel 1 exit /b 1

cd ..\..

if not exist "cpp\build\bin\Release\models\buffalo_l" (
  mkdir "cpp\build\bin\Release\models\buffalo_l" 2>nul
  xcopy /E /I /Y "models\buffalo_l\*" "cpp\build\bin\Release\models\buffalo_l\"
)

echo.
echo Build OK: cpp\build\bin\Release\insightface_cpp_demo.exe
echo.
if "%~1"=="" (
  echo Usage: run_cpp_demo.bat D:\path\to\01_R.jpg
) else (
  cd cpp\build\bin\Release
  insightface_cpp_demo.exe --image "%~1" --models models\buffalo_l
  echo.
  echo Open demo_output\landmarks_*.jpg
)
endlocal
