# InsightFace C++ Demo（Windows）

**InspireFace 官方没有 Windows 预编译 SDK**，但 InspireFace 内部用的就是 InsightFace 模型族。  
本 demo 用 **C++ + ONNX Runtime + buffalo_l（det_10g + 2d106det）**，可直接编进你们 MSVC / Qt 工程。

## 和 InspireFace 的关系

| | InspireFace SDK | 本 C++ demo |
|--|-----------------|-------------|
| 平台 | Linux/macOS/Android/iOS | **Windows x64** |
| 推理引擎 | MNN | **ONNX Runtime** |
| 检测 | SCRFD | `det_10g.onnx`（SCRFD） |
| Landmark | dense 106 | `2d106det.onnx`（106 点） |
| 集成方式 | `libInspireFace.so` | `onnxruntime.dll` + 自研薄封装 |

模型能力同级，便于评估侧脸效果后再决定是否迁入 `LibFA64`。

## 一键运行

```bat
cd InspireFace_demo
run_cpp_demo.bat D:\path\to\01_R.jpg
```

首次会自动：
1. 下载 ONNX Runtime 1.17.3（约 50MB）
2. 下载 `buffalo_l` 模型包（约 100MB）
3. CMake 编译 `insightface_cpp_demo.exe`

## 输出

`cpp\build\bin\Release\demo_output\`：

- `landmarks_all.jpg` — 106 点
- `landmarks_jaw.jpg` — 下颌轮廓（0–32）
- `landmarks_roi13.jpg` — 13 点 ROI 示意（点阵预览用，索引可再调）

## 手动步骤

```bat
powershell -File scripts\setup_deps.ps1
cd cpp\build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
cd bin\Release
insightface_cpp_demo.exe --image your.jpg --models models\buffalo_l
```

## 依赖

- Visual Studio 2019/2022（x64）
- CMake 3.16+
- OpenCV 3.4.3（默认 `D:/opencv/build`，与 LibFA64 相同）
- ONNX Runtime（脚本自动下载到 `third_party/onnxruntime`）

## 迁入 LibFA64 的路径

1. 把 `scrfd_onnx.cpp` / `landmark106_onnx.cpp` 并入 `LibFA64`
2. 新增 API：`autoMarkLeftFaceByFileV2` 等，内部走 106 点 → 映射 13 点
3. POST_BUILD 拷贝 `onnxruntime.dll` + `models/buffalo_l/*.onnx`
