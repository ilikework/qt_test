# LibFA64 / customerAnalyse 实施清单

> 设计依据：  
> - [LibFA64_customerAnalyse_integration.md](./LibFA64_customerAnalyse_integration.md)  
> - [group_contour_storage.md](./group_contour_storage.md)（**方案 1：锚点 + DrawInfo，不改表**）

状态图例：⬜ 未开始 · 🟨 进行中 · ✅ 完成

---

## P0 — 基础设施

| 状态 | 项 | 说明 |
|------|-----|------|
| ⬜ | CMake 链接 LibFA64 | `include` + `LibFA64.lib`；POST_BUILD 拷贝 `LibFA64.dll`、`opencv_world343.dll`、`shape_predictor_68_face_landmarks.dat` |
| ⬜ | `FaceAnalyseManager` | `initFaceDetector`、`busy`、QML 暴露；进程内初始化一次 |
| ⬜ | **`AppDb::deleteGroup(custId, groupId)`** | 删 DB 该组全部 `FacePhoto` / `DrawInfo` / `AnalyseInfo` + 磁盘 `customers/{id}/{group}/`；供「重新拍摄」等使用（**当前代码尚无整组删除**） |
| ⬜ | `CameraClient::lastSavedGroupId` | 保存后暴露组号，供选中最新组；保存后勿丢失 `Group_ID` |
| ⬜ | **`AppDb::resolveAnchorIx(custId, groupId, dirType)`** | 锚点 = RGB + Photo_ID=1 + L/R；注释引用 `group_contour_storage.md` |
| ⬜ | `resolveFacePhotoContext` / `getGroupContourJson` / `upsertGroupContourOnAnchor` | 组廓读写统一入口，禁止对非锚点 IX 写 `smooth_curve` |

---

## P1 — 轮廓与 ImageEditor

| 状态 | 项 | 说明 |
|------|-----|------|
| ⬜ | `ImageEditor::loadFromDb` 改造 | 组廓经 `resolveAnchorIx` 加载；无廓且不失败时不灌 Template；单图只加载线/圆 |
| ⬜ | logical768 ↔ 像素 | 与 LibFA64 README 一致；精修保存回 logical768 |
| ⬜ | 自动定位写锚点 | `autoMarkLeft/RightFaceByFile` → 锚点 DrawInfo，`source=auto` |
| ⬜ | 失败模板写锚点 | `source=template`，`autoMarkFailed=true`，该侧不再自动定位 |
| ⬜ | `endSmoothEdit` | 更新锚点组廓，`source=manual` |
| ⬜ | 代码注释 | `ImageEditor` / `AppDb` / `FaceAnalyseManager` 锚点约定注释 |

---

## P2 — QML 流程（customerAnalyse）

| 状态 | 项 | 说明 |
|------|-----|------|
| ⬜ | 保存后三选一 Dialog | 做分析 / 重新拍摄（`deleteGroup`）/ 退出 |
| ⬜ | 做分析 → 主画面 + **选中 savedGroupId** | 禁止固定 `curIndex=0` |
| ⬜ | `analyseWorkflowActive` 状态机 | 组级检查左右廓；提示自动定位 |
| ⬜ | 定位中 UI 阻塞 | `faceAnalyseManager.busy` + `InputFunnelBlocker` |
| ⬜ | `MMImageEditor` | `enterShowContour` / `enterEditContour` |
| ⬜ | 手动精修后 / 分析占位 | 双按钮；分析按钮 Phase2 占位 |

---

## P3 — 验证与后续

| 状态 | 项 | 说明 |
|------|-----|------|
| ⬜ | 与 `libfa64_demo` 同图对比轮廓 | 锚点 logical 与 demo `contour_*.txt` |
| ⬜ | 删组联调 | 重拍路径：删组后 DB/文件夹干净 |
| ⬜ | Phase2：`getAnalysisRoiPixels(facePhotoIx)` | logical → 该图 `pxl[]` → `analyse*ByFile` |

---

## 修订记录

| 日期 | 说明 |
|------|------|
| 2026-06-20 | 初版；确认锚点方案；`deleteGroup` 列入 P0 |
