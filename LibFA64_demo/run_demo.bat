@echo off
setlocal

set ROOT=%~dp0
set BUILD=%ROOT%build
set LIBFA_BIN=%ROOT%..\LibFA64\build\bin\Release
set DEMO_BIN=%BUILD%\bin\Release
set OPENCV_DLL=%ROOT%..\test64bit\libs\opencv_world343.dll

echo [1/3] Configure demo...
cmake -S "%ROOT%" -B "%BUILD%" -G "Visual Studio 17 2022" -A x64
if errorlevel 1 exit /b 1

echo [2/3] Build demo...
cmake --build "%BUILD%" --config Release
if errorlevel 1 exit /b 1

echo [3/3] Run demo...
cd /d "%DEMO_BIN%"

if exist "%ROOT%sample.jpg" (
    if exist "%DEMO_BIN%\shape_predictor_68_face_landmarks.dat" (
        libfa64_demo.exe --face --image "%ROOT%sample.jpg"
    ) else (
        echo WARN: copy shape_predictor_68_face_landmarks.dat to %DEMO_BIN%
    )
    libfa64_demo.exe --image "%ROOT%sample.jpg" --all
) else if exist "%ROOT%sample.bmp" (
    if exist "%DEMO_BIN%\shape_predictor_68_face_landmarks.dat" (
        libfa64_demo.exe --face --image "%ROOT%sample.bmp"
    )
    libfa64_demo.exe --image "%ROOT%sample.bmp" --all
) else (
    echo No sample.jpg in LibFA64_demo folder, running with synthetic image...
    libfa64_demo.exe --all
)

echo.
echo Done. Check %DEMO_BIN%\demo_output\
pause
