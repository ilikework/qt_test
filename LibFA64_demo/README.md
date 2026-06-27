# LibFA64_demo

LibFA64 的命令行验证程序：自动人脸轮廓定位 + 五项皮肤分析。

---

## 前置条件

1. 已按 [`../LibFA64/README.md`](../LibFA64/README.md) 构建 **LibFA64 Release**
2. 运行目录需有以下文件（构建 demo 时会自动复制 DLL；模型需手动放置）：

| 文件 | 说明 |
|------|------|
| `LibFA64.dll` | POST_BUILD 从 LibFA64 复制 |
| `opencv_world343.dll` | POST_BUILD 从 `test64bit/libs` 复制 |
| `shape_predictor_68_face_landmarks.dat` | dlib 模型，**需手动复制到 exe 目录** |

模型可从 DreamMirror 工程或备份目录获取，例如：

`D:\backup\MagicMirror\03_liang\DreamMirror\DreamMirror\shape_predictor_68_face_landmarks.dat`

---

## 构建

```bat
cmake -S D:\MagicMirror\git\qt_test\LibFA64_demo -B D:\MagicMirror\git\qt_test\LibFA64_demo\build ^
  -G "Visual Studio 17 2022" -A x64

cmake --build D:\MagicMirror\git\qt_test\LibFA64_demo\build --config Release
```

或使用一键脚本（需本目录有 `sample.jpg` 才会自动跑图）：

```bat
run_demo.bat
```

可执行文件：

`build/bin/Release/libfa64_demo.exe`

---

## 命令行用法

```text
libfa64_demo.exe --image <path> [--front|--left|--right] [--analyse|--all]
libfa64_demo.exe --help
```

| 参数 | 说明 |
|------|------|
| `--image <path>` | 输入图片（JPG/BMP 等） |
| `--front` | 正脸定位，36 点（默认） |
| `--left` | 左脸 / `*_L.jpg`，13 点 |
| `--right` | 右脸 / `*_R.jpg`，13 点 |
| `--analyse` / `--all` | 在自动 ROI 内运行全部分析 |
| `--face` | 仅定位（与 `--front` 等配合，不加 `--analyse` 时只出轮廓） |
| `--help` | 打印帮助 |

程序启动后会 `chdir` 到 exe 所在目录，输出写入 `demo_output/`。

---

## 使用示例

```bat
cd D:\MagicMirror\git\qt_test\LibFA64_demo\build\bin\Release

:: 复制模型（首次）
copy D:\path\to\shape_predictor_68_face_landmarks.dat .

:: 仅正脸轮廓
libfa64_demo.exe --front --image 0000001_12.jpg

:: 右脸轮廓（01_R.jpg 用 --right）
libfa64_demo.exe --right --image 01_R.jpg

:: 左脸轮廓
libfa64_demo.exe --left --image 01_L.jpg

:: 定位 + 全部分析（推荐）
libfa64_demo.exe --right --image 01_R.jpg --analyse
libfa64_demo.exe --front --image 0000001_12.jpg --analyse
```

### 文件后缀与侧脸模式

| 拍摄文件 | 使用参数 | 说明 |
|----------|----------|------|
| `*_R.jpg` | `--right` | 原图方向检测 |
| `*_L.jpg` | `--left` | 镜像检测（勿用 `--right`） |
| 正脸 / `*_M.jpg` 等 | `--front` | 36 点 |

---

## 输出文件

### 轮廓定位（`--front` / `--left` / `--right`）

| 模式 | 叠加图 | 坐标文本 |
|------|--------|----------|
| front | `demo_output/face_contour.jpg` | `demo_output/contour.txt` |
| left | `demo_output/face_contour_left.jpg` | `demo_output/contour_left.txt` |
| right | `demo_output/face_contour_right.jpg` | `demo_output/contour_right.txt` |

坐标 txt 列：`index logical_x logical_y full_x full_y`

### 皮肤分析（加 `--analyse`）

| 文件 | 分析项 |
|------|--------|
| `demo_output/analyse_pores.jpg` | 毛孔 |
| `demo_output/analyse_spots.jpg` | 色斑 |
| `demo_output/analyse_wrinkle.jpg` | 皱纹 |
| `demo_output/analyse_acnes.jpg` | 痤疮 |
| `demo_output/analyse_evenness.jpg` | 均匀度（绿→黄→橙分级，同 MagicFace） |

控制台会打印 `value` / `percent`：

- 毛孔、色斑、痤疮：`percent / 10000` ≈ 占比
- 皱纹：`percent / 100` 为展示用换算
- 均匀度：`percent = value × 100`

---

## 分析参数（demo 内置）

与 MagicFace `Analyseer.cpp` 接近，供联调参考：

| 分析项 | nMin | nMax |
|--------|------|------|
| 毛孔 | 75 | 125 |
| 色斑 | 75 | 125 |
| 皱纹 | 40 | 500 |
| 痤疮 | 75 | 125 |
| 均匀度 | 0 | 0（算法内不使用） |

`--analyse` 时会先自动定位，将轮廓转为 **原图 full 像素坐标** 再调用 DLL；与仅传 logical 坐标的旧 32 位调用方式不同，结果更贴近全分辨率原图。

---

## 常见问题

**`FAIL: shape_predictor_68_face_landmarks.dat not found`**  
将模型放到 exe 同目录。

**`01_L.jpg` 轮廓偏到对侧**  
必须用 `--left`，不要用 `--right`。

**侧脸 `count=0`**  
确认 `--right` / `--left` 与文件后缀一致；大图侧脸可依赖 Haar 后备，仍可能失败。

**`FAIL: no face contour detected` on 正脸图**  
使用 `--front`；不要用 `--right` 测正脸图。

**均匀度整片着色**  
正常；均匀度对 ROI 内每个像素按等级着色，不是只标斑点。

---

## 相关文档

- DLL API 与集成：[`../LibFA64/README.md`](../LibFA64/README.md)
- 原 MagicFace 宿主：`107_MagicFace/MagicFace/Analyseer.cpp`
