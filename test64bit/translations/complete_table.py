#!/usr/bin/env python3
"""Collect qsTr/mmTr strings and append missing entries to gen_qm.py TABLE."""
from __future__ import annotations

import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
GEN = ROOT / "translations" / "gen_qm.py"

PATTERNS = [
    re.compile(r'qsTr\("((?:\\.|[^"\\])*)"\)'),
    re.compile(r'mmTr\("((?:\\.|[^"\\])*)"\)'),
]

# Manual EN/JA for strings discovered in codebase (extend as needed)
EN: dict[str, str] = {
    "未命名": "Unnamed",
    "请填写产品名称（第 %1 条名称为空）": "Please enter product name (row %1 is empty)",
    "产品名称不能重复：「%1」": "Duplicate product name: \"%1\"",
    "请填写价格（第 %1 条价格为空，0 为有效值）": "Please enter price (row %1 is empty; 0 is valid)",
    "示例精华液": "Sample Serum",
    "示例面膜": "Sample Mask",
    "每日早晚使用，配合按摩。": "Use morning and evening with massage.",
    "每周 2–3 次，敷 15 分钟。": "Use 2–3 times per week, 15 minutes each.",
    "预录设置": "Pre-record Settings",
    "1. 产品/服务预录": "1. Products / Services",
    "2. 报告预录": "2. Report Templates",
    "维护产品/服务目录，供报告预录时选用。可上传照片、填写名称、价格、功能说明。": "Maintain product/service catalog for reports.",
    "添加": "Add",
    "点击选图": "Click to select image",
    "名称": "Name",
    "价格（必填，数字≥0，可为0）": "Price (required, number ≥ 0)",
    "功能说明": "Description",
    "删除": "Delete",
    "每种报告分 好 / 中 / 差 三档，每档填写建议并勾选推荐产品/服务。请切换下方 Tab 选择报告类型。": "Each report has Good/Medium/Poor tiers with suggestions and products.",
    "已选产品": "Selected products",
    "选择产品/服务图片": "Select product image",
    "保存失败": "Save failed",
    "保存到数据库": "Save to database",
    "是否保存当前修改到数据库？": "Save changes to database?",
    "不保存": "Don't save",
    "输入名称或功能说明筛选": "Filter by name or description",
    "全选": "Select all",
    "取消全选": "Deselect all",
    "无图": "No image",
    "价格未填": "Price not set",
    "（无功能说明）": "(No description)",
    "平移": "Pan",
    "测量": "Measure",
    "画圆": "Circle",
    "橡皮擦": "Eraser",
    "显示轮廓": "Show contour",
    "精修轮廓": "Refine contour",
    "隐藏": "Hide",
    "无可用左右图数据，无法生成 3D": "No left/right images available for 3D",
    "3D生成失败": "3D generation failed",
    "3D 光源": "3D lighting",
    "方位角 ": "Azimuth ",
    "重置光位": "Reset light",
    "亮度": "Brightness",
    "点击关闭 · 30 秒内不再提示": "Click to dismiss · no reminder for 30s",
    "检测报告": "Analysis Report",
    "检测报告": "Analysis Report",
    "%1检测报告": "%1 Analysis Report",
    "二维码": "QR code",
    "邮件": "Email",
    "彩点显示": "Overlay view",
    "A4打印": "A4 print",
    "原图": "Original",
    "对比": "Compare",
    "综合指数 ": "Overall score ",
    "指数 ": "Score ",
    "选择产品/服务": "Select products/services",
    "诊断建议（约100字）": "Diagnosis notes (~100 chars)",
    "预录建议自动填入，可修改": "Pre-filled suggestion, editable",
    "推荐产品/服务（5–10项）": "Recommended products (5–10)",
    "未选择": "None selected",
    "综合皮肤检测报告": "Comprehensive Skin Report",
    "诊断建议": "Diagnosis",
    "推荐产品/服务": "Recommended products",
    "诊断等级：": "Diagnosis tier: ",
    "　综合指数：": "  Overall score: ",
    "　指数：": "  Score: ",
    "备份": "Backup",
    "恢复": "Restore",
    "导入用": "For import",
    "客户信息": "Customer data",
    "产品与报告信息": "Products & reports",
    "高级备份": "Advanced backup",
    "浏览": "Browse",
    "开始": "Start",
    "操作完成": "Done",
    "请选择文件保存路径": "Choose save path",
    "预览": "Preview",
    "设定": "Settings",
    "正在打开相机，请稍候（": "Opening camera, please wait (",
    "秒）": "s)",
    "正在连接相机打开预览": "Connecting camera preview",
    "请先连接相机": "Connect camera first",
    "请选择下一步操作：": "Choose next step:",
    "提示": "Notice",
    "错误": "Error",
    "开始": "Start",
    "稍后": "Later",
    "左脸定位结果": "Left face result",
    "右脸定位结果": "Right face result",
    "请选择下一步：": "Choose next step:",
    "请确认是否保留本次自动定位结果。": "Keep this auto-mark result?",
    "请": "Please ",
    "恢复原定位或模板失败，请重试。": "Failed to restore previous contour. Retry.",
    "请使用右侧「取消」或「保存」\n退出拍摄": "Use Cancel or Save on the right to exit capture",
    "确认": "Confirm",
    "您确定要删除 ID 为 ": "Delete customer ID ",
    " 的用户吗？": "?",
    "电话: ": "Phone: ",
    "客户信息填写": "Customer information",
    "选择照片": "Select photo",
    "登记日": "Registered",
    "性别": "Gender",
    "生日": "Birthday",
    "手机号": "Mobile",
    "请输入客户姓名": "Enter customer name",
    "请输入Email": "Enter email",
    "请输入手机号": "Enter mobile number",
    "请选择": "Please select",
    "⚠️ 姓名不能为空": "Name is required",
    "⚠️ 请选择性别": "Please select gender",
    "⚠️ 手机号格式错误": "Invalid mobile number",
    "⚠️ 邮箱格式不正确！": "Invalid email",
    "建议（约可显示 300 字）": "Suggestion (~300 chars)",
    "共 %1 条，已选 %2 条": "%1 items, %2 selected",
    "X轴左右变脸（点按→Y轴上下）": "X-axis morph (tap for Y-axis)",
    "Y轴上下变脸（点按→XY区域）": "Y-axis morph (tap for XY region)",
    "XY区域变脸（点按→X轴左右）": "XY region morph (tap for X-axis)",
    "约 %1 秒后自动左右摆动": "Auto swing in ~%1 s",
    "毛孔": "Pores",
    "粉刺": "Acne",
    "深层色斑": "Deep spots",
    "浅层色斑": "Surface spots",
    "皱纹": "Wrinkles",
    "敏感": "Sensitivity",
    "褐色斑": "Brown spots",
    "混合彩斑": "Mixed spots",
    "initFaceDetector 失败，请确认 shape_predictor_68_face_landmarks.dat 在程序目录": "initFaceDetector failed. Ensure shape_predictor_68_face_landmarks.dat is beside the exe.",
    "使用原定位结果": "Use previous contour",
    "使用默认模板": "Use default template",
    "正在定位中，请稍候": "Marking in progress, please wait",
    "无效的客户或组号": "Invalid customer or group",
    "%1：已锁定默认轮廓（跳过）": "%1: default contour locked (skipped)",
    "%1：锚点图片未找到": "%1: anchor image not found",
    "%1：定位失败": "%1: marking failed",
    "%1：自动定位成功": "%1: auto mark succeeded",
    "%1：自动定位失败（LibFA 未检测到人脸，0 个点），已用默认轮廓": "%1: auto mark failed (0 points), using default contour",
    "%1：自动定位失败，已用默认轮廓": "%1: auto mark failed, using default contour",
    "；保存轮廓失败": "; failed to save contour",
    "人脸检测初始化失败": "Face detector init failed",
    "色斑": "Spots",
    "毛孔": "Pores",
    "均匀度": "Evenness",
    "皱纹": "Wrinkles",
    "痤疮": "Acne",
    "水分": "Moisture",
    "分析": "Analysis",
    "T_FacePhoto_Map 未配置分析映射": "T_FacePhoto_Map has no analyse mapping",
    "T_FacePhoto_Map 中没有可执行的 LibFA64 分析项": "No LibFA64 analyse items in T_FacePhoto_Map",
    "左脸": "Left",
    "右脸": "Right",
    "%1 %2 未找到照片": "%1 %2 photo not found",
    "%1 图片不存在：%2": "%1 image missing: %2",
    "%1 ROI 无效": "%1 invalid ROI",
    "保存分析结果失败：%1": "Failed to save analyse result: %1",
    "未找到分析配置": "Analyse config not found",
    "未找到客户信息": "Customer not found",
    "读取左右脸 ROI 失败，请先完成区域定位": "Failed to read ROI. Complete contour marking first.",
    "清除旧分析结果失败": "Failed to clear old analyse results",
    "皮肤分析完成，共 %1 项。": "Skin analysis complete: %1 items.",
    "皮肤分析部分完成：成功 %1 项，失败 %2 项。\n%3": "Partial analysis: %1 ok, %2 failed.\n%3",
    "皮肤分析失败。\n": "Skin analysis failed.\n",
    "正在处理中，请稍候": "Busy, please wait",
    "请先完成左右脸区域定位": "Complete left/right contour marking first",
    "相机打开失败，请检查连接": "Failed to open camera. Check connection.",
    "预览打开失败": "Failed to open preview",
    "RGB 快门": "RGB shutter",
    "RGB 光圈": "RGB aperture",
    "RGB 白平衡": "RGB white balance",
    "UV 快门": "UV shutter",
    "UV 光圈": "UV aperture",
    "UV 白平衡": "UV white balance",
    "PL 快门": "PL shutter",
    "PL 光圈": "PL aperture",
    "PL 白平衡": "PL white balance",
    "NPL 快门": "NPL shutter",
    "NPL 光圈": "NPL aperture",
    "NPL 白平衡": "NPL white balance",
    "图片尺寸": "Image size",
    "图片质量": "Image quality",
    "备份文件损坏或解压失败": "Backup archive corrupted or extract failed",
    "压缩备份失败": "Backup compression failed",
    "备份成功！": "Backup successful!",
    "非法备份包：缺少数据库文件": "Invalid backup: missing database",
    "（数据库与顾客目录可能被占用）": " (database and customer folder may be in use)",
    "（数据库文件被占用或权限不足）": " (database file locked or permission denied)",
    "（顾客目录被占用或权限不足，请关闭可能占用该目录的程序）": " (customer folder locked; close programs using it)",
    "无法暂存旧数据": "Cannot stage old data",
    "，恢复终止": ", restore aborted",
    "恢复成功！": "Restore successful!",
    "部署失败，已自动回滚至原始数据。": "Deploy failed; rolled back to original data.",
    "无法复制数据库文件": "Cannot copy database file",
    "自动（跟随系统）": "Auto (System)",
    "左右脸轮廓已就绪，请选择下一步：": "Left/right contours are ready. Choose next step:",
    "° · 仰角 ": "° · elevation ",
    "处理中: %1%": "Processing: %1%",
    "正在校验备份包...": "Verifying backup archive...",
    "正在安全暂存当前数据...": "Safely staging current data...",
    "正在部署新文件...": "Deploying new files...",
    "正在重新连接数据库...": "Reconnecting database...",
    "正在压缩/解压，请稍候…": "Compressing/extracting, please wait…",
    "选择备份保存位置": "Choose backup save location",
    "选择备份文件进行恢复": "Choose backup file to restore",
    "准备备份...": "Preparing backup...",
    "准备恢复...": "Preparing restore...",
    "导入功能开发中...": "Import feature coming soon...",
    "%1年 %2月": "%1 / %2",
    "日": "Sun",
    "一": "Mon",
    "二": "Tue",
    "三": "Wed",
    "四": "Thu",
    "五": "Fri",
    "六": "Sat",
    "图片 (*.png *.jpg *.jpeg *.bmp)": "Images (*.png *.jpg *.jpeg *.bmp)",
    "请确认是否保留本次自动定位结果。": "Keep this auto-mark result?",
    "。": ".",
    "男": "Male",
    "女": "Female",
    "备份与恢复": "Backup & Restore",
    "客户报告": "Customer Report",
    "A4打印": "A4 Print",
    "保存": "Save",
    "返回": "Back",
    "好": "Good",
    "中": "Medium",
    "差": "Poor",
    "取消": "Cancel",
    "确定": "OK",
    "原贴图": "Original texture",
    "关灯": "Lights off",
    "开灯": "Lights on",
    "关放大镜": "Magnifier off",
    "放大镜": "Magnifier",
    "正在生成 3D 模型…": "Generating 3D model…",
    "做分析": "Analyze",
    "重新拍摄": "Retake",
    "保存后退出": "Save and exit",
    "保存成功": "Saved",
    "未能连接相机程序，请联系客服。": "Cannot connect to camera app. Contact support.",
    "秒）": "s)",
}

try:
    from fill_ja import JA as _JA_FULL
except ImportError:
    _JA_FULL = {}
JA: dict[str, str] = {k: v for k, v in EN.items()}
JA.update(_JA_FULL)


def collect() -> set[str]:
    found: set[str] = set()
    paths = list((ROOT / "QMLContent").rglob("*.qml")) + list(ROOT.glob("*.cpp"))
    for path in paths:
        if "del" in path.parts:
            continue
        text = path.read_text(encoding="utf-8", errors="ignore")
        for pat in PATTERNS:
            found.update(pat.findall(text))
        for m in re.finditer(r'mmTr\("((?:\\.|[^"\\])*)"\)', text):
            found.add(m.group(1))
    return found


def tw(s: str) -> str:
    table = {
        "户": "戶", "个": "個", "为": "為", "录": "錄", "备": "備", "复": "復",
        "与": "與", "产": "產", "务": "務", "报": "報", "设": "設", "删": "刪",
        "选": "選", "择": "擇", "请": "請", "输": "輸", "号": "號", "时": "時",
        "间": "間", "别": "別", "肤": "膚", "皱": "皺", "纹": "紋", "测": "測",
        "显": "顯", "对": "對", "图": "圖", "脸": "臉", "摄": "攝", "览": "覽",
        "连": "連", "预": "預", "览": "覽", "败": "敗", "误": "誤", "脸": "臉",
        "须": "須", "确认": "確認", "电话": "電話", "登记": "登記", "性别": "性別",
        "手机": "手機", "填写": "填寫", "败": "敗", "须": "須", "质": "質",
    }
    out = s
    for a, b in table.items():
        out = out.replace(a, b)
    return out


def main() -> None:
    text = GEN.read_text(encoding="utf-8")
    ns: dict = {}
    exec(compile(text.replace("__file__", repr(str(GEN))), str(GEN), "exec"), ns)
    table: dict = ns["TABLE"]

    # also add strings still hardcoded in QML assignments - manual list from grep
    extra = [
        "未命名", "检测报告", "原图", "对比",
    ]
    for s in collect() | set(extra):
        if s not in table:
            table[s] = {
                "zh_CN": s,
                "zh_TW": tw(s),
                "en": EN.get(s, s),
                "ja": JA.get(s, EN.get(s, s)),
            }
        elif s in EN:
            table[s]["en"] = EN[s]
            if s in JA:
                table[s]["ja"] = JA[s]
            else:
                table[s]["ja"] = EN[s]

    # rewrite TABLE in gen_qm.py
    lines = ['TABLE: dict[str, dict[str, str]] = {']
    for key in sorted(table.keys()):
        row = table[key]
        lines.append(f'    {key!r}: {{')
        lines.append(f'        "zh_CN": {row["zh_CN"]!r},')
        lines.append(f'        "zh_TW": {row["zh_TW"]!r},')
        lines.append(f'        "en": {row["en"]!r},')
        lines.append(f'        "ja": {row["ja"]!r},')
        lines.append('    },')
    lines.append('}')
    new_table = "\n".join(lines)

    start = text.index("TABLE: dict[str, dict[str, str]] = {")
    end = text.index("\n\nLOCALES", start)
    new_text = text[:start] + new_table + text[end:]
    GEN.write_text(new_text, encoding="utf-8")
    print("TABLE entries:", len(table))


if __name__ == "__main__":
    main()
