# 组级皮肤分析 — 设计规格

> 关联：[LibFA64_customerAnalyse_integration.md](./LibFA64_customerAnalyse_integration.md)、[group_contour_storage.md](./group_contour_storage.md)  
> 外部：[LibFA64/README.md](../../LibFA64/README.md) — API、CapType 映射、分析参数  
> 状态：**实施中**

---

## 1. 目标

对 **一个 Group** 内左右两侧、各 **8 张** 分析用照片，按产品标准映射调用 LibFA64 `analyse*ByFile`，将数值结果写入 `T_FacePhoto_AnalyseInfo`，叠加图写入组目录。

触发入口：`customerAnalyse` 主画面 **「皮肤分析」** 按钮（须已完成左右脸区域定位）。

---

## 2. 范围

| 包含 | 不包含 |
|------|--------|
| L/R × 8 CapType 中 **7 项 LibFA64 分析** | WHOLE 水分（`Analyse_Function=21`，老 OCX 算法） |
| ROI 来自组级锚点 `smooth_curve`（13 点像素多边形） | 报告页 UI、等级判定、叠加图在编辑器内预览 |
| 结果 upsert 到 `T_FacePhoto_AnalyseInfo` | `generate*PictureByFile` 衍生图生成 |
| 叠加图输出到 `customers/{custId}/{group}/analyse/` | 正脸 36 点 / M 侧照片 |

---

## 3. 照片与分析项映射（T_FacePhoto_Map）

**配置源**：`T_FacePhoto_Map` 表 — 每张 **CapType** 对应一个 **Analyse_Function**（及 Report_Type）。  
组分析启动时 `AppDb::getFacePhotoAnalyseMap()` 读全表，**不再在 C++ 硬编码 CapType 列表**。

| Photo_CapType | Analyse_Function | LibFA API（代码注册） | Report_Type | 默认 |
|---------------|------------------|----------------------|-------------|------|
| RGB | 2 | `analysePoresByFile` | 1 | ✓ |
| UV | 5 | `analyseAcnesByFile` | 2 | ✓ |
| PL | 1 | `analyseSpotsByFile` | 3 | ✓ |
| NPL | 1 | `analyseSpotsByFile` | 4 | ✓ |
| GRAY | 4 | `analyseWrinkleByFile` | 5 | ✓ |
| RED | 3 | `analyseEvennessByFile` | 6 | ✓ |
| BROWN | 1 | `analyseSpotsByFile` | 7 | ✓ |
| WHOLE | 21 | （非 LibFA，跳过） | 8 | ✓ |

**两层分工**：

| 层 | 可配置性 | 说明 |
|----|----------|------|
| `Photo_CapType` → `Analyse_Function` | **DB 可改** | 例如把 RGB 从毛孔(2) 改成色斑(1)，改 `T_FacePhoto_Map` 即可 |
| `Analyse_Function` → LibFA DLL | **代码固定** | 1~5 映射到五个 `analyse*ByFile`；21=水分待 OCX |

**查找照片**：组内按 `Cust_ID + Group_ID + Photo_DirType + Photo_CapType` 定位 `T_Customers_FacePhoto` 行（`findPhotoInGroupByCapType`），不依赖 Photo_ID 硬编码。

**任务数**：`(T_FacePhoto_Map 中 Analyse_Function 为 1~5 的行数) × 2 侧`。默认 7×2=14。

**初始化**：`init.sql` 插入 8 行默认映射；已有库若表为空，`AppDb::ensureFacePhotoAnalyseMapDefaults()` 补种。
---

## 4. ROI（pxl[]）

1. 左/右分别读锚点轮廓：`getGroupContourDrawInfo(custId, groupId, L/R)`。  
2. JSON `points` 为 **锚点 RGB 图像素**（`coordSpace=pixel`，见 group_contour_storage）。  
3. 转为 LibFA 所需 `pxl[]`：`[x0,y0,x1,y1,…]`，闭合多边形，**原图 OpenCV 像素坐标**。  
4. **同侧** 8 张图共用该侧 ROI（组内分辨率一致；若将来尺寸不一致，需按锚点→目标图比例缩放，本期不实现）。

前置条件：`groupNeedsAutoMark == false`（与 UI「皮肤分析」可用条件一致）。

---

## 5. LibFA 调用参数

| 项 | 约定 |
|----|------|
| `initFaceDetector` | 分析前调用一次（与自动定位共用进程内状态） |
| `setExtraInfo(age, gender, source_type)` | age 由 `Cust_Birthday` 推算；gender=`Cust_Gender`；source_type 见下表 |
| nMin / nMax | 毛孔/色斑/痤疮/均匀度/皱纹：**75 / 125**（与 README doMAnalyse2 一致）；均匀度算法内忽略阈值 |
| 输出叠加图 | `{groupDir}/analyse/{Photo_Name 去扩展名}_overlay.jpg` |

`source_type`（`LibFA.h`）：

| CapType | SOURCE_* |
|---------|----------|
| RGB | SOURCE_RGB (0) |
| UV | SOURCE_UV365 (1) |
| PL | SOURCE_PL_POSITIVE (3) |
| NPL | SOURCE_PL_NEGATIVE (4) |
| GRAY / RED / BROWN | SOURCE_RGB (0)（LibFA 未单独定义，沿用 RGB） |

---

## 6. 数据库写入

表：`T_FacePhoto_AnalyseInfo`

| 列 | 来源 |
|----|------|
| FacePhoto_IX | `T_Customers_FacePhoto.IX`（该 CapType + DirType + Photo_ID） |
| Analyse_Function | 上表 Function 列 |
| Analyse_Result | `T_ANA_RESULT.value`（检出数量） |
| Analyse_Precent | `T_ANA_RESULT.percent`（×10000，原样入库） |
| EditTime | 本地时间 `datetime('now','localtime')` |

**策略**：组分析开始前 `DELETE` 该组全部 `AnalyseInfo`（仅 LibFA 项，即关联 14 张照片的行）；再逐条 `INSERT`/`UPDATE`（`upsertAnalyseInfo` 按 `FacePhoto_IX + Analyse_Function` 唯一）。

---

## 7. C++ 接口（FaceAnalyseManager）

```cpp
Q_INVOKABLE void analyseGroup(const QString &customerId, int groupId);
signals:
    void groupAnalyseProgress(int done, int total, const QString &label);
    void groupAnalyseFinished(bool success, const QString &message);
```

- 与 `autoMarkGroup` 共用 `busy`；进行中互斥。  
- 后台 `QtConcurrent` 执行 14 步，每步 emit progress。  
- 任一步输入图缺失、ROI 无效、DLL 失败 → 记录日志，该步写入 Result=0/Percent=0 或跳过（本期：**失败步写 0 并继续**，最后汇总 message）。

---

## 8. AppDb 扩展

```cpp
bool findCustomerByCustId(const QString &custId, Customer *out) const;
bool findPhotoInGroup(..., FacePhoto *out) const;
QString groupFolderPath(const QString &custId, int groupId) const;
QString photoFilePath(const FacePhoto &photo) const;
bool upsertAnalyseInfo(int facePhotoIx, int analyseFunction, int result, int percent);
bool deleteGroupAnalyseInfo(const QString &custId, int groupId);
QVector<FacePhotoAnalyseMapEntry> getFacePhotoAnalyseMap() const;
```

---

## 9. QML（customerAnalyse）

- `startSkinAnalyse()` → `faceAnalyseManager.analyseGroup(customerID, currentGroupID)`。  
- `busy` 遮罩文案：皮肤分析时显示「正在皮肤分析…」。  
- `groupAnalyseFinished`：成功 MessageBox 汇总；失败显示原因。  
- 拍摄后工作流选「皮肤分析」同样走 `analyseGroup`。

---

## 10. 流程

```text
皮肤分析按钮
  ├─ groupRegionReady? 否 → 提示先定位
  ├─ ensureDetector()
  ├─ deleteGroupAnalyseInfo(group)
  ├─ 读 L/R 锚点轮廓 → pxlL[], pxlR[]
  ├─ 读客户 age/gender
  └─ for side in {L,R}:
        for row in getFacePhotoAnalyseMap() (Analyse_Function 1~5):
          findPhotoByCapType → inPath
          resolve LibFA API from Analyse_Function
          setExtraInfo(age, gender, source_type from CapType)
          analyse*ByFile(...)
          upsertAnalyseInfo(IX, row.Analyse_Function, ...)
  → groupAnalyseFinished
```

---

## 11. 后续（非本期）

- WHOLE 水分（Function 21）  
- 报告页读取 `AnalyseInfo` + 叠加图展示  
- RED/BROWN 衍生图 `generate*PictureByFile`  
- 按 `T_Report_Template` 计算 Report_LEVEL  

---

## 修订记录

| 日期 | 说明 |
|------|------|
| 2026-06-27 | 初版：14 项 LibFA 组分析 + DB 写入 |
| 2026-07-04 | 分析映射改为读取 `T_FacePhoto_Map`，CapType→Function 可 DB 配置 |
