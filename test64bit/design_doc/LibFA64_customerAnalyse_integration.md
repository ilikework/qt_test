# LibFA64 接入 customerAnalyse — 设计规格（确认版）

> 状态：**设计已确认，待编码**  
> 范围：拍摄后流程、左右 RGB 自动轮廓定位、手动精修；皮肤分析（毛孔/色斑等）留 Phase2。

---

## 1. 目标与范围

### 1.1 本期实现

| 做 | 不做 |
|----|------|
| 将 LibFA64 链接进 `test64bit`，C++ 桥接 `FaceAnalyseManager` | `analysePoresByFile` 等分析调用与结果 UI |
| 拍摄保存后三选一：做分析 / 重新拍摄 / 退出 | UV/PL 等非 RGB 光型的自动定位 |
| 切主画面并选中**最新拍摄组**后走分析工作流 | 正脸 36 点 `autoMarkFaceByFile`（可预留 API） |
| 左右 RGB 分别调用 `autoMarkLeftFaceByFile` / `autoMarkRightFaceByFile` | 报告页联动 |
| 点阵转 `smooth_curve` 入库，`MMImageEditor` 显示/精修轮廓 | |

### 1.2 产品决策（已确认）

| 问题 | 决策 |
|------|------|
| 「重新拍摄」是否删刚保存的一组？ | **是**：删除 DB + 磁盘目录 |
| 平时是否用 `T_FacePhoto_DrawInfo_Template`？ | **否**；仅自动定位**失败**时兜底 |
| 自动定位失败后？ | 用模板兜底，并**标记** `autoMarkFailed`，**不再提示/不再做自动定位** |
| 保存后「退出」？ | **保留数据**，回主画面，**不**进入分析工作流 |
| 保存后「做分析」？ | 切主画面时**必须选中最新拍摄组**，再自动定位与分析准备 |
| 轮廓作用范围？ | **整组**：左廓用于组内所有左图，右廓用于所有右图（见 [group_contour_storage.md](./group_contour_storage.md)） |
| 组廓存哪？ | **方案 1 已确认**：不改表；只写在锚点 `FacePhoto_IX`（RGB + Photo_ID=1）的 `DrawInfo` 上 |

---

## 2. 依赖与参考

### 2.1 LibFA64 API（侧脸）

| 拍摄文件 / 方向 | 函数 | 点数 |
|-----------------|------|------|
| `*_L.jpg` | `autoMarkLeftFaceByFile` | 13 |
| `*_R.jpg` | `autoMarkRightFaceByFile` | 13 |

进程内调用一次：`initFaceDetector()`（成功返回 `true`）。

### 2.2 运行时部署（exe 同目录）

1. `LibFA64.dll`
2. `opencv_world343.dll`
3. `shape_predictor_68_face_landmarks.dat`

### 2.3 坐标转换（必须使用 LibFA64 公式）

DLL 返回 **768 宽逻辑坐标**，Y 轴**自下而上**（MagicFace 风格）。

映射到原图像素（OpenCV 左上角原点，分析 ROI 也用此坐标）：

```text
fullX = round(logicalX * imageWidth / 768)
fullY = round(imageHeight - 1 - logicalY * imageWidth / 768)
```

**禁止**使用模板灌入时的公式（`x * width/768`, `y * height/1024`，且无 Y 翻转）。  
参考：`LibFA64_demo/main.cpp`、`LibFA64/README.md`。

### 2.4 现有代码冲突点

`ImageEditor::loadFromDb` 当前在「无 `smooth_curve`」时会**静默**从 `getTemplateInfo("_L"/"_R")` 灌模板并写库 —— 与本设计冲突，**必须改造**（见 §5）。

---

## 3. 架构

```text
customerAnalyse.qml
    ├── CameraView          保存后弹窗三选一
    ├── MMImageEditor ×2    leftMain / rightMain
    └── analyseWorkflow     状态机

FaceAnalyseManager (新 QObject，参考 MM3DManager)
    ├── initFaceDetector / autoMarkLeft / autoMarkRight
    ├── logical → full 像素转换
    ├── deleteGroup(customerId, groupId)
    ├── busy 信号 → InputFunnelBlocker（App.qml 或专用属性）
    └── 工作线程执行 DLL，避免阻塞 UI

ImageEditor (改造)
    ├── init(ix, dirType)：组廓经 resolveAnchorIx 读锚点 DrawInfo
    ├── applyContour / reloadDrawings
    └── 精修/自动定位只 upsert 锚点上的 smooth_curve（scope=group）

AppDb
    ├── resolveAnchorIx / upsertGroupContourOnAnchor
    └── deleteGroup（整组删除，见 implementation_checklist.md）
```

---

## 4. 用户流程

### 4.1 拍摄保存后弹窗

```text
[保存成功：DB + 磁盘已写入]
        │
        ├─ 做分析 ──→ requestShowMain + selectGroup(savedGroupID)
        │              analyseWorkflowActive = true
        │
        ├─ 重新拍摄 ──→ deleteGroup(customerId, savedGroupID)
        │              留在拍摄页，重新 preview
        │
        └─ 退出 ──→ requestShowMain，数据保留
                    analyseWorkflowActive = false
```

**实现要点**

- `camClient.save()` 后会把内部 `GroupID_` 置 0；保存时需向 QML 暴露 **`lastSavedGroupId`**（属性或信号参数）。
- 「重新拍摄」需实现 **`deleteGroup`**：删除 `T_Customers_FacePhoto`、`T_FacePhoto_DrawInfo`、`T_FacePhoto_AnalyseInfo` 及 `customers/{custId}/{groupId}/` 目录。
- `CameraView`：保存后**先弹窗**，再根据选择决定是否 `requestShowMain`（不再保存后无条件回主画面）。

### 4.2 选中最新组

「做分析」切主画面后：

1. `analyseModule.init(customerID)`
2. 在 `mainphotoes` 中查找 `GROUPID === savedGroupID` → 设 `curIndex`
3. `loadsubphotoes(curIndex)`，加载左右主图

**禁止**固定 `curIndex = 0`。

### 4.3 分析工作流（`analyseWorkflowActive`）

仅用户选择「做分析」且已选中正确组时进入。

```text
对 RGB、Photo_ID=1 的左右图（IXL / IXR）分别判断轮廓状态
        │
        ├─ 已有有效轮廓（见 §4.4）→ 跳过自动定位
        │
        └─ 无轮廓且未 autoMarkFailed
                → 弹窗：「是否现在做自动定位？」
                   [开始] / [稍后]（稍后则 analyseWorkflowActive=false）
        │
        ▼
[开始] 后台线程（可串行或并行）：
  左：autoMarkLeftFaceByFile(本地 *_L 路径)
  右：autoMarkRightFaceByFile(本地 *_R 路径)
        │
        ├─ count > 0：写锚点 DrawInfo（smooth_curve, source=auto, scope=group）
        │              提示「自动定位完成」
        │              MMImageEditor.currentMode = ShowSmooth
        │
        └─ count == 0：从 Template 表取 _L/_R 模板
                       写锚点 DrawInfo（source=template, autoMarkFailed=true）
                       提示「自动定位失败，已使用默认轮廓」
                       ShowSmooth；此后不再提供自动定位
        │
        ▼
弹窗：「手动精修轮廓」 / 「图片分析」（Phase2 占位）
        │
        ├─ 手动精修 → currentMode: EditSmooth
        │              endSmoothEdit → 更新锚点组廓，source=manual
        │              显示「分析」按钮（本期占位）
        │
        └─ 图片分析 → Phase2（本期仅校验轮廓就绪 + 提示开发中）
```

**左右独立**：一侧成功、一侧失败分别处理；两侧都处理完后再出「手动 / 分析」选择。

**定位进行中**：`faceAnalyseManager.busy` + 全窗 `InputFunnelBlocker`，禁止切组/误操作。

---

## 5. 轮廓数据规则

> **存储**：方案 1 已确认 — 组廓在**锚点** `DrawInfo` 上，经 `resolveAnchorIx` 读写。详见 [group_contour_storage.md](./group_contour_storage.md)。

### 5.1 `ImageEditor::loadFromDb` 新逻辑

```text
① resolveAnchorIx → 锚点 DrawInfo 中有 scope=group 的 smooth_curve → 加载（logical768 → 当前图像素）
② 锚点无组廓且未 autoMarkFailed → 不画脸廓，不读 Template
③ 已 autoMarkFailed 且锚点无廓（异常）→ Template 写入锚点一次
④ 当前 facePhotoIx 的 DrawInfo → 只加载线/圆，忽略非锚点上的 smooth_curve
```

**禁止**：无轮廓时静默灌模板；禁止对非锚点 IX 写组廓。

### 5.2 轮廓状态（锚点 DrawInfo 内 JSON）

| 字段 | 说明 |
|------|------|
| `scope` | 固定 `"group"` |
| `source` | `"auto"` \| `"manual"` \| `"template"` |
| `autoMarkFailed` | 失败兜底后为 `true`，该侧不再自动定位 |
| `coordSpace` | `"logical768"` |
| `pointCount` | 可选，13（侧脸） |

**自动定位成功示例：**

```json
{
  "type": "smooth_curve",
  "version": "V1",
  "scope": "group",
  "source": "auto",
  "autoMarkFailed": false,
  "coordSpace": "logical768",
  "pointCount": 13,
  "color": "#ff0000",
  "weight": 3,
  "points": [{"x": 328, "y": 445}, ...]
}
```

**失败兜底示例：**

```json
{
  "type": "smooth_curve",
  "scope": "group",
  "source": "template",
  "autoMarkFailed": true,
  "coordSpace": "logical768",
  "points": [...]
}
```

### 5.3 状态与 UI 对照

| 状态 | 条件（查锚点） | 分析流程中的行为 |
|------|----------------|------------------|
| `none` | 锚点无组廓、未失败 | 提示「是否自动定位」 |
| `auto` | 自动成功 | ShowSmooth，可精修 |
| `manual` | 用户精修过 | 同上 |
| `template` + `autoMarkFailed` | 失败兜底 | ShowSmooth，不再自动定位 |

### 5.4 历史数据

- 锚点（或任意 IX）上旧 `smooth_curve` 无 `scope`：视为组廓；优先迁移到锚点 IX。
- 非锚点 IX 上的 `smooth_curve`：读时忽略。

---

## 6. ImageEditor / MMImageEditor 改造

| API / 行为 | 说明 |
|------------|------|
| `init(ix, dirType)` | 组廓经 `resolveAnchorIx`；无廓不灌 Template |
| `reloadDrawings()` | 自动定位 / 精修后刷新 |
| `contourState()` | 查锚点 JSON，供 QML |
| `getContourPixelPolygon(ix)` | Phase2：logical → 该图 `pxl[]` |

**MMImageEditor.qml**

- `enterShowContour()` / `enterEditContour()`
- 精修结束写**锚点** DrawInfo

**精修**：`endSmoothEdit` → `upsertGroupContourOnAnchor`，`source=manual`。

---

## 7. FaceAnalyseManager（C++ 草案）

```cpp
// Q_PROPERTY bool busy READ busy NOTIFY busyChanged

Q_INVOKABLE bool ensureDetector();
Q_INVOKABLE QVariantMap autoMarkGroupSide(const QString &custId, int groupId, bool isLeft);
// 内部：resolveAnchorIx + RGB Photo_ID=1 路径 + DLL + 写锚点 DrawInfo

Q_INVOKABLE bool deleteGroup(const QString &customerId, int groupId);  // 委托 AppDb
Q_INVOKABLE int contourState(const QString &custId, int groupId, const QString &dirType);
```

- `autoMarkSide` 内：DLL 调用 → 坐标转换 → 写 DB 或失败走 `getTemplateInfo` + `autoMarkFailed`。
- 使用 `QtConcurrent` / 专用线程，`busy` 时 UI 漏斗拦截。

---

## 8. UI 组件

| 场景 | 组件 |
|------|------|
| 保存后三选一 | 新 `AnalyseChoiceDialog`（3 按钮） |
| 是否自动定位 | 双按钮 Dialog |
| 定位完成 → 手动/分析 | 双按钮 Dialog |
| 定位中 | `InputFunnelBlocker`（`faceAnalyseManager.busy`） |
| 错误提示 | 现有 `MessageBox` |

---

## 9. CMake / 部署

```cmake
target_include_directories(apptest64bit PRIVATE path/to/LibFA64/include)
target_link_libraries(apptest64bit PRIVATE path/to/LibFA64.lib)
# POST_BUILD: LibFA64.dll, opencv_world343.dll, shape_predictor_68_face_landmarks.dat
```

`main.cpp`：`engine.rootContext()->setContextProperty("faceAnalyseManager", &faceAnalyseManager);`

---

## 10. 边界与风险

| # | 严重度 | 问题 | 对策 |
|---|--------|------|------|
| 1 | 高 | 坐标公式混用 | 仅 LibFA64 full 公式；单测对照 demo `contour_*.txt` |
| 2 | 高 | 静默灌模板 | 锚点无廓时不灌；见 §5.1 |
| 3 | 高 | 保存后 `GroupID` 丢失 | `lastSavedGroupId` |
| 4 | 中 | 删组后 `curIndex` 越界 | 刷新 thumbs 后 clamp |
| 5 | 中 | 左右一侧失败 | 分侧提示与写锚点 |
| 6 | 中 | DLL 阻塞 UI | 工作线程 + `busy` |
| 7 | 中 | 误写非锚点 IX | 强制 `upsertGroupContourOnAnchor` |
| 8 | 低 | 仅 RGB Photo_ID=1 | 本期写死；Phase2 扩展 |
| 9 | 低 | API 名与文件后缀 | 以 `Photo_DirType` L/R 选函数 |
| 10 | 低 | 用户选「稍后」 | `analyseWorkflowActive=false` |

---

## 11. 实施顺序（编码）

详见 **[implementation_checklist.md](./implementation_checklist.md)**（含 `deleteGroup`、`resolveAnchorIx`）。

概要：

1. P0：`deleteGroup`、`resolveAnchorIx`、LibFA64 链接  
2. P1：ImageEditor 锚点读写、logical768  
3. P2：QML 拍摄后流程与 `customerAnalyse` 状态机  
4. P3：demo 对比与 Phase2 ROI  

---

## 12. Phase2 预留（分析）

```text
1. 从 smooth_curve 导出 pxl[]（full 像素，闭合多边形）
2. setExtraInfo(age, gender, SOURCE_RGB)
3. analysePoresByFile / analyseSpotsByFile / ...
4. 结果写入 T_FacePhoto_AnalyseInfo + 叠加图展示
```

参数参考：`LibFA64/README.md`、`LibFA64_demo`（毛孔/色斑/痤疮 nMin=75, nMax=125）。

---

## 修订记录

| 日期 | 说明 |
|------|------|
| 2026-06-20 | 初版确认 |
| 2026-06-20 | 组廓改为方案 1 锚点 + DrawInfo；实施清单独立文件 |
