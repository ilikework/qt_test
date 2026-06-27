# LibFA64

原 MagicFace 32 位 `LibFA.dll` 的 **64 位（x64）** 移植版，供 Qt / C++ 宿主调用。功能包括：

- 正脸 / 侧脸自动轮廓定位（36 / 13 点）
- 皮肤分析：毛孔、色斑、皱纹、痤疮、均匀度
- 与 MagicFace `LibFA.h` 保持相同的 C 导出接口风格

算法来源：DreamMirror 人脸定位 + `lib.h` 分析核心。

---

## 环境要求

| 项 | 版本 / 说明 |
|----|-------------|
| 平台 | **Windows x64** |
| 编译器 | Visual Studio 2022（MSVC v143） |
| CMake | 3.16+ |
| OpenCV | 3.4.3（`opencv_world343.lib` / `.dll`） |
| dlib | 19.9（工程内通过 CMake 编译，无需 GUI） |

默认 CMake 路径（可用 `-D` 覆盖）：

- `OpenCV_ROOT` = `D:/opencv/build`
- `DLIB_ROOT` = `D:/MagicMirror/svn/DreamMirror/dlib-19.9/dlib`

---

## 构建

```bat
cmake -S D:\MagicMirror\git\qt_test\LibFA64 -B D:\MagicMirror\git\qt_test\LibFA64\build ^
  -G "Visual Studio 17 2022" -A x64

cmake --build D:\MagicMirror\git\qt_test\LibFA64\build --config Release
```

输出：

| 文件 | 路径 |
|------|------|
| DLL | `build/bin/Release/LibFA64.dll` |
| 导入库 | `build/lib/Release/LibFA64.lib` |
| 头文件 | `include/LibFA.h` |

---

## 运行时依赖

部署目录需包含：

1. `LibFA64.dll`
2. `opencv_world343.dll`（与编译时 OpenCV 版本一致）
3. **`shape_predictor_68_face_landmarks.dat`**（dlib 68 点模型，与 exe/DLL **同目录**）

人脸检测接口调用前必须：

```cpp
initFaceDetector();  // 成功返回 true，进程内调用一次即可
```

---

## 集成到宿主工程

### 1. 链接

```cmake
target_include_directories(your_app PRIVATE path/to/LibFA64/include)
target_link_libraries(your_app PRIVATE path/to/LibFA64/build/lib/Release/LibFA64.lib)
```

### 2. 头文件

```cpp
#include "LibFA.h"
```

### 3. 运行时

将 `LibFA64.dll`、`opencv_world343.dll`、`shape_predictor_68_face_landmarks.dat` 放到 exe 同目录（或 PATH）。

---

## API 概览

头文件：`include/LibFA.h`

### 初始化

```cpp
bool initFaceDetector();
void setExtraInfo(int age, int gender, int source_type);
```

`source_type`：`SOURCE_RGB` / `SOURCE_UV365` / `SOURCE_UV405` / `SOURCE_PL_POSITIVE` / `SOURCE_PL_NEGATIVE`

### 自动轮廓定位

| 函数 | 用途 | 点数 |
|------|------|------|
| `autoMarkFaceByFile` | 正脸 | 36 |
| `autoMarkRightFaceByFile` | 拍摄文件 `*_R.jpg` | 13 |
| `autoMarkLeftFaceByFile` | 拍摄文件 `*_L.jpg` | 13 |

```cpp
T_CONTOUR c = autoMarkFaceByFile(path);
// c.count, c.x[], c.y[]
freeContour(&c);
```

**侧脸说明：** API 名称与拍摄文件后缀一致（`*_R` → `--right`，`*_L` → `--left`）。`*_R` / `*_L` 均使用 `LeftFaceMask`，但 landmark 构建对称：

| 文件 | API | 检测 | landmark |
|------|-----|------|----------|
| `*_R.jpg` | `autoMarkRightFaceByFile` | 优先原图方向 | 标准顺序 |
| `*_L.jpg` | `autoMarkLeftFaceByFile` | 优先镜像 | 镜像坐标，**不做** kIndex 重排 |

请勿对 `01_L.jpg` 使用 `--right`（会走 R 图路径，轮廓会偏到对侧）。

### 皮肤分析

```cpp
T_ANA_RESULT analysePoresByFile(char *in, char *out, int *pxl, int pxl_cnt, int nMin, int nMax);
T_ANA_RESULT analyseSpotsByFile(...);
T_ANA_RESULT analyseWrinkleByFile(...);
T_ANA_RESULT analyseAcnesByFile(...);
T_ANA_RESULT analyseEvennessByFile(...);  // nMin/nMax 传入但不参与算法
```

- `inFile`：原图路径（BMP/JPG 等，OpenCV 可读）
- `outFile`：叠加标注结果图
- `pxl`：多边形 ROI，`{x0,y0, x1,y1, ...}`，**原图像素坐标，OpenCV 左上角为原点**
- `pxl_cnt`：坐标个数（偶数，至少 4）
- 返回 `T_ANA_RESULT`：`value` = 数量或分数，`percent` = 占比×10000（均匀度为 `value×100`）

### 合成图（尚未实现）

以下接口为桩，当前返回 `false`：

- `generateBrownSunburnPictureByFile`
- `generateRedBloodPictureByFile`
- `generateMixedSpotPictureByFile`

---

## 坐标系

`autoMark*FaceByFile` 返回 **768 宽逻辑坐标**，Y 轴 **自下而上**（与 MagicFace 一致）。

映射到原图（OpenCV 显示/分析用）：

```text
scale   = imageWidth / 768.0
fullX   = logicalX * scale
fullY   = imageHeight - 1 - logicalY * scale   // 转为 OpenCV top-origin
```

**分析接口 `analyse*ByFile` 的 `pxl` 必须使用 full 像素坐标**，不能直接用 logical 坐标。

示例：

```cpp
T_CONTOUR c = autoMarkRightFaceByFile(path);
cv::Mat img = cv::imread(path);
const double scale = img.cols / 768.0;

int pxl[200];
int n = 0;
for (int i = 0; i < c.count; ++i) {
    pxl[n++] = (int)(c.x[i] * scale + 0.5);
    pxl[n++] = (int)(img.rows - 1 - c.y[i] * scale + 0.5);
}

setExtraInfo(30, 1, SOURCE_RGB);
T_ANA_RESULT r = analysePoresByFile(inPath, outPath, pxl, n, 75, 125);
freeContour(&c);
```

---

## 分析参数参考

LibFA64 内部还固定使用（与 MagicFace 默认 ini 一致）：

- `ConVal = 80`
- `MinArea = 30`，`MaxArea = 100`（色斑 / 皱纹）

### 与 MagicFace `Analyseer.cpp`（doMAnalyse2 路径）

| 分析项 | nMin | nMax |
|--------|------|------|
| 毛孔 / 色斑 / 痤疮 | 75 | 125 |
| 皱纹 | 75 | 125 |
| 均匀度 | 75 | 125（算法内忽略） |

### 与 MagicFace 产品默认（doAnalyse_bylocal + analysis.ini）

| 分析项 | 女性默认 nMin | nMax |
|--------|---------------|------|
| RGB 毛孔 | 105 | 115 |
| RGB 色斑 | 60 | 90 |
| RGB 皱纹 | 89 | 100 |
| UV 痤疮 | 165 | - |

集成时可按产品 ini 传入不同阈值。均匀度可视化使用 MagicFace `RGBEvennessConst` 绿→黄→橙 24 级配色。

---

## 完整调用流程

```text
1. initFaceDetector()
2. autoMark*FaceByFile()  →  T_CONTOUR（logical 坐标）
3. logical → full 像素  →  pxl[]
4. setExtraInfo(age, gender, source_type)   // 可选
5. analyse*ByFile(in, out, pxl, n, nMin, nMax)
6. freeContour(&c)
```

---

## 目录结构

```text
LibFA64/
  include/LibFA.h       对外 API
  src/
    libfa_api.cpp       导出层
    face_detect.cpp     人脸检测（dlib + Haar 后备）
    analysis_bridge.cpp 分析桥接 + 结果叠加
    extra_info.cpp      setExtraInfo 存储
  vendor/lib.h          原分析算法（已 patch 边界问题）
  CMakeLists.txt
```

---

## 验证

参见 [`../LibFA64_demo/README.md`](../LibFA64_demo/README.md) 中的命令行 demo。

---

## 已知限制

1. `setExtraInfo` 当前仅保存静态变量，分析算法尚未读取
2. `generate*PictureByFile` 未实现
3. 侧脸大角度、UV/偏振图检测精度依赖原算法，可能需后续调参或换 CNN 检测器
4. 32 位 DLL 若传入 logical 坐标做分析，64 位版需转为 full 坐标（见上文）
