# 组级轮廓存储设计（方案 1：锚点 + 现有 DrawInfo）

> 关联：[LibFA64_customerAnalyse_integration.md](./LibFA64_customerAnalyse_integration.md)  
> 状态：**已确认** — **不改表结构**，组廓挂在锚点照片的 `T_FacePhoto_DrawInfo` 上。

---

## 1. 业务需求

一组拍摄（同一 `Cust_ID` + `Group_ID`）包含多对照片：

- 左：`RGB, UV, PL, NPL, GRAY, RED, BROWN, WHOLE` …（`Photo_DirType=L`，`Photo_ID` 1~8）
- 右：同上（`Photo_DirType=R`）

**规则**：无论自动定位还是手动精修，**完成后整组生效**：

- 组内所有**左图**共用同一条左脸轮廓 ROI  
- 组内所有**右图**共用同一条右脸轮廓 ROI  

后续 `analyse*ByFile` 对每张图都要用该 ROI（`pxl[]`）。

---

## 2. 已确认方案：锚点存储（不改表）

### 2.1 核心约定（文档 + 代码必须一致）

> **组廓在锚点上**  
> 每一组、每一侧（左/右）有且仅有一个 **锚点 `FacePhoto_IX`**。  
> 该组的脸廓 `smooth_curve` **只写在锚点的 `T_FacePhoto_DrawInfo` 里**（通常 1 条记录）。  
> 组内同侧其它照片的 `DrawInfo` **不存**脸廓；读取时通过 `resolveAnchorIx` 找到锚点再加载。

**锚点定义（写死，勿改语义）：**

| 字段 | 值 |
|------|-----|
| `Photo_CapType` | `RGB`（与 `MM_RGB` 常量一致） |
| `Photo_ID` | `1` |
| `Photo_DirType` | `L` → 左锚；`R` → 右锚 |
| `Group_ID` / `Cust_ID` | 当前组 |

左锚、右锚各对应 `customerAnalyse` 里 `IXL` / `IXR`（`Photo_ID=1` 那一对）。

### 2.2 为何不用新表

- 删除以**组**为单位，锚点行随组删除，不存在「锚点单独被删、其它图还在」的正常场景。  
- `resolveAnchorIx(custId, groupId, L/R)` 多一步查询，**无法省略**，作为统一入口即可。  
- 零表结构迁移，与现有 `insertDrawInfo` / `updateDrawInfo` 兼容。

### 2.3 未采纳方案（备查）

| 方案 | 说明 |
|------|------|
| 每张照片复制一条 DrawInfo | 冗余、易不一致，不采用 |
| 新表 `T_FacePhoto_GroupContour` | 模型更清晰但需改库，不采用 |
| `FacePhoto_IX = -1` 虚拟行 | 外键语义弱，不采用 |

---

## 3. 表与数据分工（现有结构）

**不新增、不修改表结构。**

| 存储位置 | 内容 |
|----------|------|
| **锚点 `FacePhoto_IX` 的 `T_FacePhoto_DrawInfo`** | 组级脸廓 `smooth_curve`（`scope: "group"`） |
| **非锚点 `FacePhoto_IX` 的 `T_FacePhoto_DrawInfo`** | 仅单张图的线、圆等标注；**禁止**再写 `smooth_curve` |
| **`T_FacePhoto_DrawInfo_Template`** | 全局模板；仅自动定位**失败**时写入锚点 DrawInfo |

`init.sql` 中 `FacePhoto_IX` 注释里的 `-1` 本方案**不使用**；组廓一律挂在真实锚点 IX 上。

---

## 4. JSON 格式（锚点 DrawInfo 内）

```json
{
  "type": "smooth_curve",
  "version": "V1",
  "scope": "group",
  "source": "auto",
  "pointCount": 13,
  "color": "#ff0000",
  "weight": 3,
  "points": [
    { "x": 1520, "y": 980 }
  ]
}
```

| 字段 | 说明 |
|------|------|
| `scope` | 固定 `"group"`，表示本条是组廓而非单图装饰 |
| `source` | 轮廓来源，见下表（**不再使用** `autoMarkFailed`） |
| `points` | **锚点 RGB 照片上的像素坐标**（与照片分辨率一致；组内照片尺寸固定时可共用） |

**不写入组廓 JSON 的字段**（仅模板表或旧数据可能有）：

| 已废弃 / 不用 | 说明 |
|---------------|------|
| `width` / `height` | 照片尺寸固定，点已是真实像素，无需冗余 |
| `coordSpace` | 新数据默认像素；仅旧库 `logical768` 读出时做一次性迁移 |
| `autoMarkFailed` | 语义不清；由 `source` 表达，见下 |

### 4.1 `source` 语义（唯一状态字段）

| `source` | 含义 | 是否再提示自动定位 |
|----------|------|-------------------|
| `auto` | LibFA 自动定位**成功**，点已转为锚点图像素 | 否（已有有效轮廓） |
| `manual` | 用户在锚点图上**精修**过 | 否 |
| `template` | 自动定位**失败**，已用默认模板缩放到锚点像素 | **否**（锁定默认轮廓，不再重试） |
| （无记录） | 该侧尚无组廓 | 是 |

### 4.2 坐标规则

1. **自动定位**：DLL 返回 logical768 → **当场**按锚点 RGB 宽高转为像素 → 写入 `points`。  
2. **精修**：在图上编辑的即为像素 → 原样写回锚点 DrawInfo，`source=manual`。  
3. **打开组内其它图**：读锚点 `points` 直接绘制（照片尺寸固定时无需换算）。  
4. **分析（Phase2）**：从锚点像素轮廓生成 LibFA 所需 `pxl[]`（届时再换算）。

**旧数据**：`coordSpace=logical768` 或 `autoMarkFailed=true` 的条目，下次「分析」时重做并写成上表格式。

---

## 5. API 约定（AppDb / 工具）

### 5.1 `resolveAnchorIx`（必需）

```cpp
/// 组廓锚点：RGB + Photo_ID=1 + 指定侧(L/R)。
/// 见 design_doc/group_contour_storage.md
int resolveAnchorIx(const QString &custId, int groupId, const QString &photoDirType);
// 失败返回 -1（无此组锚点照片）
```

由任意 `FacePhoto_IX` 查组廓时：

```text
facePhotoIx → (custId, groupId, dirType) → resolveAnchorIx → anchorIx
→ getAllDrawInfos(anchorIx) → 取 type==smooth_curve && scope==group
```

**代码中**：凡读写组廓，必须经 `resolveAnchorIx` 或封装它的 `getGroupContourDrawInfo` / `upsertGroupContourOnAnchor`，禁止对非锚点 IX 写 `smooth_curve`。

### 5.2 建议封装（实现时）

```cpp
bool resolveFacePhotoContext(int facePhotoIx, QString *custId, int *groupId, QString *dirType);
int resolveAnchorIx(const QString &custId, int groupId, const QString &dirType);

QString getGroupContourJson(int facePhotoIx);  // 内部 resolveAnchorIx
bool upsertGroupContourOnAnchor(const QString &custId, int groupId,
                                const QString &dirType, const QString &jsonInfo);
ContourMeta getGroupContourMeta(const QString &custId, int groupId, const QString &dirType);
```

### 5.3 代码注释要求

在以下位置加 **相同语义** 的注释（可引用本 MD 路径）：

- `AppDb::resolveAnchorIx` 定义处  
- `ImageEditor::loadFromDb`（加载组廓分支）  
- `ImageEditor::endSmoothEdit` / 保存组廓处  
- `FaceAnalyseManager` 自动定位写库处  

示例：

```cpp
// Group face contour (smooth_curve) is stored ONLY on the anchor FacePhoto_IX
// (RGB, Photo_ID=1, L or R). See test64bit/design_doc/group_contour_storage.md
```

---

## 6. ImageEditor 加载流程

```text
init(facePhotoIx, dirType):
  1. resolveFacePhotoContext(facePhotoIx) → custId, groupId, side
  2. anchorIx = resolveAnchorIx(custId, groupId, side)
  3. 从 anchorIx 的 DrawInfo 加载 scope==group 的 smooth_curve
     → 锚点像素 points 直接显示
  4. 若无组廓且 source≠template：不画脸廓
  5. 从**当前** facePhotoIx 的 DrawInfo 加载线/圆（忽略其中的 smooth_curve）

endSmoothEdit / 自动定位成功:
  → upsertGroupContourOnAnchor（只写锚点，source=manual|auto）

loadFromDb 禁止：对非锚点 IX 静默灌 Template 组廓。
```

---

## 7. 删组

照片按**组**删除，锚点 DrawInfo 随 `FacePhoto_IX` 级联删除，无需单独「恢复锚点」逻辑。

**待实现**：`AppDb::deleteGroup(custId, groupId)`（见 [implementation_checklist.md](./implementation_checklist.md)）。

删除范围应包括：

- `T_Customers_FacePhoto`（该组全部行）  
- `T_FacePhoto_DrawInfo`（含锚点组廓 + 各张标注）  
- `T_FacePhoto_AnalyseInfo`  
- 磁盘目录 `customers/{custId}/{groupId}/`

---

## 8. 边界情况

| 场景 | 处理 |
|------|------|
| 打开组内非锚点左/右图 | `resolveAnchorIx` 读锚点组廓，按当前图尺寸显示 |
| 精修 | 仅在锚点图或任意同侧图上编辑，**写回锚点** |
| 自动定位失败 | 模板缩放到锚点像素写入，`source=template`（不再自动定位） |
| 左右独立 | 左锚、右锚各一条 JSON，状态独立 |
| 历史数据：非锚点 IX 上有 smooth_curve | 读时忽略；可选迁移到锚点 |
| 历史数据：锚点无 scope 字段 | 视为组廓；补写 `scope:group` |

---

## 修订记录

| 日期 | 说明 |
|------|------|
| 2026-06-20 | 初版：曾推荐新表 |
| 2026-06-20 | **确认方案 1**：锚点 + DrawInfo，不改表；resolveAnchorIx；删组待办 |
