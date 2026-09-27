#!/usr/bin/env python3
"""Generate mmface_*.qm from embedded translation table (no lupdate required)."""
from __future__ import annotations

import os
import re
import subprocess
import sys
import xml.etree.ElementTree as ET
from pathlib import Path
from xml.dom import minidom

ROOT = os.path.dirname(os.path.abspath(__file__))
OUT_DIR = os.path.join(ROOT, "i18n")
QML_ROOT = Path(ROOT).resolve().parent / "QMLContent"
QSTR_PAT = re.compile(r'qsTr\("((?:\\.|[^"\\])*)"\)')
os.makedirs(OUT_DIR, exist_ok=True)

# source (Simplified Chinese) -> locale translations
TABLE: dict[str, dict[str, str]] = {
    '   生日: ': {
        "zh_CN": '   生日: ',
        "zh_TW": '   生日: ',
        "en": '   Birthday: ',
        "ja": '   誕生日: ',
    },
    ' 的用户吗？': {
        "zh_CN": ' 的用户吗？',
        "zh_TW": ' 的用戶吗？',
        "en": '?',
        "ja": ' のユーザーを削除しますか？',
    },
    '%1 %2 未找到照片': {
        "zh_CN": '%1 %2 未找到照片',
        "zh_TW": '%1 %2 未找到照片',
        "en": '%1 %2 photo not found',
        "ja": '%1 %2 の写真が見つかりません',
    },
    '%1 ROI 无效': {
        "zh_CN": '%1 ROI 无效',
        "zh_TW": '%1 ROI 无效',
        "en": '%1 invalid ROI',
        "ja": '%1 の ROI が無効です',
    },
    '%1 图片不存在：%2': {
        "zh_CN": '%1 图片不存在：%2',
        "zh_TW": '%1 圖片不存在：%2',
        "en": '%1 image missing: %2',
        "ja": '%1 画像が存在しません：%2',
    },
    '%1年 %2月': {
        "zh_CN": '%1年 %2月',
        "zh_TW": '%1年 %2月',
        "en": '%1 / %2',
        "ja": '%1年%2月',
    },
    '%1：定位失败': {
        "zh_CN": '%1：定位失败',
        "zh_TW": '%1：定位失敗',
        "en": '%1: marking failed',
        "ja": '%1：マーキングに失敗しました',
    },
    '%1：已锁定默认轮廓（跳过）': {
        "zh_CN": '%1：已锁定默认轮廓（跳过）',
        "zh_TW": '%1：已锁定默认轮廓（跳过）',
        "en": '%1: default contour locked (skipped)',
        "ja": '%1：デフォルト輪郭をロック済み（スキップ）',
    },
    '%1：自动定位失败（LibFA 未检测到人脸，0 个点），已用默认轮廓': {
        "zh_CN": '%1：自动定位失败（LibFA 未检测到人脸，0 个点），已用默认轮廓',
        "zh_TW": '%1：自动定位失敗（LibFA 未检測到人臉，0 個点），已用默认轮廓',
        "en": '%1: auto mark failed (0 points), using default contour',
        "ja": '%1：自動マーキング失敗（顔未検出・0点）、デフォルト輪郭を使用',
    },
    '%1：自动定位失败，已用默认轮廓': {
        "zh_CN": '%1：自动定位失败，已用默认轮廓',
        "zh_TW": '%1：自动定位失敗，已用默认轮廓',
        "en": '%1: auto mark failed, using default contour',
        "ja": '%1：自動マーキング失敗、デフォルト輪郭を使用',
    },
    '%1：自动定位成功': {
        "zh_CN": '%1：自动定位成功',
        "zh_TW": '%1：自动定位成功',
        "en": '%1: auto mark succeeded',
        "ja": '%1：自動マーキング成功',
    },
    '%1：锚点图片未找到': {
        "zh_CN": '%1：锚点图片未找到',
        "zh_TW": '%1：锚点圖片未找到',
        "en": '%1: anchor image not found',
        "ja": '%1：アンカー画像が見つかりません',
    },
    '1. 产品/服务预录': {
        "zh_CN": '1. 产品/服务预录',
        "zh_TW": '1. 產品/服務預錄',
        "en": '1. Products / Services',
        "ja": '1. 製品・サービス事前登録',
    },
    '2. 报告预录': {
        "zh_CN": '2. 报告预录',
        "zh_TW": '2. 報告預錄',
        "en": '2. Report Templates',
        "ja": '2. レポート事前登録',
    },
    '3D 光源': {
        "zh_CN": '3D 光源',
        "zh_TW": '3D 光源',
        "en": '3D lighting',
        "ja": '3D 照明',
    },
    '3D人脸': {
        "zh_CN": '3D人脸',
        "zh_TW": '3D人臉',
        "en": '3D Face',
        "ja": '3D顔',
    },
    '3D模型': {
        "zh_CN": '3D模型',
        "zh_TW": '3D模型',
        "en": '3D Model',
        "ja": '3Dモデル',
    },
    '3D生成失败': {
        "zh_CN": '3D生成失败',
        "zh_TW": '3D生成失敗',
        "en": '3D generation failed',
        "ja": '3D 生成に失敗しました',
    },
    '< 上一页': {
        "zh_CN": '< 上一页',
        "zh_TW": '< 上一頁',
        "en": '< Previous',
        "ja": '< 前へ',
    },
    'A4打印': {
        "zh_CN": 'A4打印',
        "zh_TW": 'A4打印',
        "en": 'A4 Print',
        "ja": 'A4印刷',
    },
    'Acne': {
        "zh_CN": '粉刺',
        "zh_TW": '粉刺',
        "en": 'Acne',
        "ja": 'にきび',
    },
    'Brown Spots': {
        "zh_CN": '褐色斑',
        "zh_TW": '褐色斑',
        "en": 'Brown Spots',
        "ja": 'シミ',
    },
    'Cancel': {
        "zh_CN": '取消',
        "zh_TW": '取消',
        "en": 'Cancel',
        "ja": 'キャンセル',
    },
    'Deep Spots': {
        "zh_CN": '深层色斑',
        "zh_TW": '深層色斑',
        "en": 'Deep Spots',
        "ja": '深層シミ',
    },
    'Female': {
        "zh_CN": '女',
        "zh_TW": '女',
        "en": 'Female',
        "ja": '女性',
    },
    'Good': {
        "zh_CN": '好',
        "zh_TW": '好',
        "en": 'Good',
        "ja": '良',
    },
    'Male': {
        "zh_CN": '男',
        "zh_TW": '男',
        "en": 'Male',
        "ja": '男性',
    },
    'Medium': {
        "zh_CN": '中',
        "zh_TW": '中',
        "en": 'Medium',
        "ja": '中',
    },
    'Mixed Spots': {
        "zh_CN": '混合彩斑',
        "zh_TW": '混合彩斑',
        "en": 'Mixed Spots',
        "ja": '混合シミ',
    },
    'NPL 光圈': {
        "zh_CN": 'NPL 光圈',
        "zh_TW": 'NPL 光圈',
        "en": 'NPL aperture',
        "ja": 'NPL 絞り',
    },
    'NPL 快门': {
        "zh_CN": 'NPL 快门',
        "zh_TW": 'NPL 快门',
        "en": 'NPL shutter',
        "ja": 'NPL シャッター',
    },
    'NPL 白平衡': {
        "zh_CN": 'NPL 白平衡',
        "zh_TW": 'NPL 白平衡',
        "en": 'NPL white balance',
        "ja": 'NPL ホワイトバランス',
    },
    'OK': {
        "zh_CN": '确定',
        "zh_TW": '確定',
        "en": 'OK',
        "ja": 'OK',
    },
    'PL 光圈': {
        "zh_CN": 'PL 光圈',
        "zh_TW": 'PL 光圈',
        "en": 'PL aperture',
        "ja": 'PL 絞り',
    },
    'PL 快门': {
        "zh_CN": 'PL 快门',
        "zh_TW": 'PL 快门',
        "en": 'PL shutter',
        "ja": 'PL シャッター',
    },
    'PL 白平衡': {
        "zh_CN": 'PL 白平衡',
        "zh_TW": 'PL 白平衡',
        "en": 'PL white balance',
        "ja": 'PL ホワイトバランス',
    },
    'Poor': {
        "zh_CN": '差',
        "zh_TW": '差',
        "en": 'Poor',
        "ja": '不良',
    },
    'Pores': {
        "zh_CN": '毛 孔',
        "zh_TW": '毛 孔',
        "en": 'Pores',
        "ja": '毛穴',
    },
    'RGB 光圈': {
        "zh_CN": 'RGB 光圈',
        "zh_TW": 'RGB 光圈',
        "en": 'RGB aperture',
        "ja": 'RGB 絞り',
    },
    'RGB 快门': {
        "zh_CN": 'RGB 快门',
        "zh_TW": 'RGB 快门',
        "en": 'RGB shutter',
        "ja": 'RGB シャッター',
    },
    'RGB 白平衡': {
        "zh_CN": 'RGB 白平衡',
        "zh_TW": 'RGB 白平衡',
        "en": 'RGB white balance',
        "ja": 'RGB ホワイトバランス',
    },
    'Save': {
        "zh_CN": '保存',
        "zh_TW": '保存',
        "en": 'Save',
        "ja": '保存',
    },
    'Sensitivity': {
        "zh_CN": '敏感度',
        "zh_TW": '敏感度',
        "en": 'Sensitivity',
        "ja": '敏感',
    },
    'Summary Report': {
        "zh_CN": '综合报告',
        "zh_TW": '綜合報告',
        "en": 'Summary',
        "ja": '総合レポート',
    },
    'Surface Spots': {
        "zh_CN": '浅层色斑',
        "zh_TW": '淺層色斑',
        "en": 'Surface Spots',
        "ja": '表層シミ',
    },
    'T_FacePhoto_Map 中没有可执行的 LibFA64 分析项': {
        "zh_CN": 'T_FacePhoto_Map 中没有可执行的 LibFA64 分析项',
        "zh_TW": 'T_FacePhoto_Map 中没有可执行的 LibFA64 分析项',
        "en": 'No LibFA64 analyse items in T_FacePhoto_Map',
        "ja": 'T_FacePhoto_Map に実行可能な LibFA64 分析項目がありません',
    },
    'T_FacePhoto_Map 未配置分析映射': {
        "zh_CN": 'T_FacePhoto_Map 未配置分析映射',
        "zh_TW": 'T_FacePhoto_Map 未配置分析映射',
        "en": 'T_FacePhoto_Map has no analyse mapping',
        "ja": 'T_FacePhoto_Map に分析マッピングがありません',
    },
    'UV 光圈': {
        "zh_CN": 'UV 光圈',
        "zh_TW": 'UV 光圈',
        "en": 'UV aperture',
        "ja": 'UV 絞り',
    },
    'UV 快门': {
        "zh_CN": 'UV 快门',
        "zh_TW": 'UV 快门',
        "en": 'UV shutter',
        "ja": 'UV シャッター',
    },
    'UV 白平衡': {
        "zh_CN": 'UV 白平衡',
        "zh_TW": 'UV 白平衡',
        "en": 'UV white balance',
        "ja": 'UV ホワイトバランス',
    },
    'Unknown': {
        "zh_CN": '未知',
        "zh_TW": '未知',
        "en": 'Unknown',
        "ja": '不明',
    },
    'Wrinkles': {
        "zh_CN": '皱 纹',
        "zh_TW": '皺 紋',
        "en": 'Wrinkles',
        "ja": 'しわ',
    },
    'XY区域变脸（点按→X轴左右）': {
        "zh_CN": 'XY区域变脸（点按→X轴左右）',
        "zh_TW": 'XY区域变臉（点按→X轴左右）',
        "en": 'XY region morph (tap for X-axis)',
        "ja": 'XY領域モーフ（タップでX軸左右）',
    },
    'X轴左右变脸（点按→Y轴上下）': {
        "zh_CN": 'X轴左右变脸（点按→Y轴上下）',
        "zh_TW": 'X轴左右变臉（点按→Y轴上下）',
        "en": 'X-axis morph (tap for Y-axis)',
        "ja": 'X軸左右モーフ（タップでY軸上下）',
    },
    'Y轴上下变脸（点按→XY区域）': {
        "zh_CN": 'Y轴上下变脸（点按→XY区域）',
        "zh_TW": 'Y轴上下变臉（点按→XY区域）',
        "en": 'Y-axis morph (tap for XY region)',
        "ja": 'Y軸上下モーフ（タップでXY領域）',
    },
    'initFaceDetector 失败，请确认 shape_predictor_68_face_landmarks.dat 在程序目录': {
        "zh_CN": 'initFaceDetector 失败，请确认 shape_predictor_68_face_landmarks.dat 在程序目录',
        "zh_TW": 'initFaceDetector 失敗，請確認 shape_predictor_68_face_landmarks.dat 在程序目錄',
        "en": 'initFaceDetector failed. Ensure shape_predictor_68_face_landmarks.dat is beside the exe.',
        "ja": 'initFaceDetector 失敗。shape_predictor_68_face_landmarks.dat が exe 横にあるか確認してください',
    },
    '° · 仰角 ': {
        "zh_CN": '° · 仰角 ',
        "zh_TW": '° · 仰角 ',
        "en": '° · elevation ',
        "ja": '° · 仰角 ',
    },
    '⚠️ 姓名不能为空': {
        "zh_CN": '⚠️ 姓名不能为空',
        "zh_TW": '⚠️ 姓名不能為空',
        "en": 'Name is required',
        "ja": '⚠️ 氏名は必須です',
    },
    '⚠️ 手机号格式错误': {
        "zh_CN": '⚠️ 手机号格式错误',
        "zh_TW": '⚠️ 手機號格式错誤',
        "en": 'Invalid mobile number',
        "ja": '⚠️ 携帯番号の形式が正しくありません',
    },
    '⚠️ 请选择性别': {
        "zh_CN": '⚠️ 请选择性别',
        "zh_TW": '⚠️ 請選擇性別',
        "en": 'Please select gender',
        "ja": '⚠️ 性別を選択してください',
    },
    '⚠️ 邮箱格式不正确！': {
        "zh_CN": '⚠️ 邮箱格式不正确！',
        "zh_TW": '⚠️ 邮箱格式不正确！',
        "en": 'Invalid email',
        "ja": '⚠️ メール形式が正しくありません',
    },
    '\u3000指数：': {
        "zh_CN": '\u3000指数：',
        "zh_TW": '\u3000指数：',
        "en": '  Score: ',
        "ja": '\u3000指数：',
    },
    '\u3000综合指数：': {
        "zh_CN": '\u3000综合指数：',
        "zh_TW": '\u3000综合指数：',
        "en": '  Overall score: ',
        "ja": '\u3000総合指数：',
    },
    '。': {
        "zh_CN": '。',
        "zh_TW": '。',
        "en": '.',
        "ja": '。',
    },
    '一': {
        "zh_CN": '一',
        "zh_TW": '一',
        "en": 'Mon',
        "ja": '月',
    },
    '三': {
        "zh_CN": '三',
        "zh_TW": '三',
        "en": 'Wed',
        "ja": '水',
    },
    '下一页 >': {
        "zh_CN": '下一页 >',
        "zh_TW": '下一頁 >',
        "en": 'Next >',
        "ja": '次へ >',
    },
    '不保存': {
        "zh_CN": '不保存',
        "zh_TW": '不保存',
        "en": "Don't save",
        "ja": '保存しない',
    },
    '中': {
        "zh_CN": '中',
        "zh_TW": '中',
        "en": 'Medium',
        "ja": '中',
    },
    '主画面': {
        "zh_CN": '主画面',
        "zh_TW": '主畫面',
        "en": 'Main View',
        "ja": 'メイン画面',
    },
    '二': {
        "zh_CN": '二',
        "zh_TW": '二',
        "en": 'Tue',
        "ja": '火',
    },
    '二维码': {
        "zh_CN": '二维码',
        "zh_TW": '二维码',
        "en": 'QR code',
        "ja": 'QRコード',
    },
    '五': {
        "zh_CN": '五',
        "zh_TW": '五',
        "en": 'Fri',
        "ja": '金',
    },
    '产品与报告信息': {
        "zh_CN": '产品与报告信息',
        "zh_TW": '產品與報告信息',
        "en": 'Products & reports',
        "ja": '製品・レポート情報',
    },
    '产品名称不能重复：「%1」': {
        "zh_CN": '产品名称不能重复：「%1」',
        "zh_TW": '產品名称不能重復：「%1」',
        "en": 'Duplicate product name: "%1"',
        "ja": '製品名が重複しています：「%1」',
    },
    '亮度': {
        "zh_CN": '亮度',
        "zh_TW": '亮度',
        "en": 'Brightness',
        "ja": '明るさ',
    },
    '人脸检测初始化失败': {
        "zh_CN": '人脸检测初始化失败',
        "zh_TW": '人臉检測初始化失敗',
        "en": 'Face detector init failed',
        "ja": '顔検出の初期化に失敗しました',
    },
    '价格未填': {
        "zh_CN": '价格未填',
        "zh_TW": '价格未填',
        "en": 'Price not set',
        "ja": '価格未入力',
    },
    '价格（必填，数字≥0，可为0）': {
        "zh_CN": '价格（必填，数字≥0，可为0）',
        "zh_TW": '价格（必填，数字≥0，可為0）',
        "en": 'Price (required, number ≥ 0)',
        "ja": '価格（必須、数値≥0、0可）',
    },
    '使用原定位结果': {
        "zh_CN": '使用原定位结果',
        "zh_TW": '使用原定位結果',
        "en": 'Use previous contour',
        "ja": '前回のマーキング結果を使用',
    },
    '使用默认模板': {
        "zh_CN": '使用默认模板',
        "zh_TW": '使用默认模板',
        "en": 'Use default template',
        "ja": 'デフォルトテンプレートを使用',
    },
    '保存': {
        "zh_CN": '保存',
        "zh_TW": '保存',
        "en": 'Save',
        "ja": '保存',
    },
    '保存分析结果失败：%1': {
        "zh_CN": '保存分析结果失败：%1',
        "zh_TW": '保存分析结果失敗：%1',
        "en": 'Failed to save analyse result: %1',
        "ja": '分析結果の保存に失敗：%1',
    },
    '保存到数据库': {
        "zh_CN": '保存到数据库',
        "zh_TW": '保存到数据库',
        "en": 'Save to database',
        "ja": 'データベースに保存',
    },
    '保存后退出': {
        "zh_CN": '保存后退出',
        "zh_TW": '保存後退出',
        "en": 'Save and exit',
        "ja": '保存して終了',
    },
    '保存失败': {
        "zh_CN": '保存失败',
        "zh_TW": '保存失敗',
        "en": 'Save failed',
        "ja": '保存に失敗しました',
    },
    '保存成功': {
        "zh_CN": '保存成功',
        "zh_TW": '保存成功',
        "en": 'Saved',
        "ja": '保存しました',
    },
    '保留新定位结果': {
        "zh_CN": '保留新定位结果',
        "zh_TW": '保留新定位結果',
        "en": 'Keep New Result',
        "ja": '新しい結果を保持',
    },
    '做分析': {
        "zh_CN": '做分析',
        "zh_TW": '做分析',
        "en": 'Analyze',
        "ja": '分析する',
    },
    '全选': {
        "zh_CN": '全选',
        "zh_TW": '全選',
        "en": 'Select all',
        "ja": 'すべて選択',
    },
    '六': {
        "zh_CN": '六',
        "zh_TW": '六',
        "en": 'Sat',
        "ja": '土',
    },
    '共 %1 条，已选 %2 条': {
        "zh_CN": '共 %1 条，已选 %2 条',
        "zh_TW": '共 %1 条，已選 %2 条',
        "en": '%1 items, %2 selected',
        "ja": '全 %1 件、%2 件選択',
    },
    '关放大镜': {
        "zh_CN": '关放大镜',
        "zh_TW": '關放大鏡',
        "en": 'Magnifier off',
        "ja": '拡大鏡オフ',
    },
    '关灯': {
        "zh_CN": '关灯',
        "zh_TW": '關燈',
        "en": 'Lights off',
        "ja": '照明オフ',
    },
    '准备备份...': {
        "zh_CN": '准备备份...',
        "zh_TW": '准備備份...',
        "en": 'Preparing backup...',
        "ja": 'バックアップ準備中...',
    },
    '准备恢复...': {
        "zh_CN": '准备恢复...',
        "zh_TW": '准備恢復...',
        "en": 'Preparing restore...',
        "ja": '復元準備中...',
    },
    '分析': {
        "zh_CN": '分析',
        "zh_TW": '分析',
        "en": 'Analysis',
        "ja": '分析',
    },
    '分析失败': {
        "zh_CN": '分析失败',
        "zh_TW": '分析失敗',
        "en": 'Analysis Failed',
        "ja": '分析失敗',
    },
    '分析完成': {
        "zh_CN": '分析完成',
        "zh_TW": '分析完成',
        "en": 'Analysis Complete',
        "ja": '分析完了',
    },
    '分析贴图': {
        "zh_CN": '分析贴图',
        "zh_TW": '分析貼圖',
        "en": 'Analysis Texture',
        "ja": '分析テクスチャ',
    },
    '删除': {
        "zh_CN": '删除',
        "zh_TW": '刪除',
        "en": 'Delete',
        "ja": '削除',
    },
    '删除用户': {
        "zh_CN": '删除用户',
        "zh_TW": '刪除用戶',
        "en": 'Delete',
        "ja": '削除',
    },
    '功能说明': {
        "zh_CN": '功能说明',
        "zh_TW": '功能说明',
        "en": 'Description',
        "ja": '機能説明',
    },
    '压缩备份失败': {
        "zh_CN": '压缩备份失败',
        "zh_TW": '压缩備份失敗',
        "en": 'Backup compression failed',
        "ja": 'バックアップ圧縮に失敗しました',
    },
    '原图': {
        "zh_CN": '原图',
        "zh_TW": '原圖',
        "en": 'Original',
        "ja": '原画',
    },
    '原贴图': {
        "zh_CN": '原贴图',
        "zh_TW": '原貼圖',
        "en": 'Original texture',
        "ja": '元テクスチャ',
    },
    '取消': {
        "zh_CN": '取消',
        "zh_TW": '取消',
        "en": 'Cancel',
        "ja": 'キャンセル',
    },
    '取消全选': {
        "zh_CN": '取消全选',
        "zh_TW": '取消全選',
        "en": 'Deselect all',
        "ja": '選択解除',
    },
    '变脸': {
        "zh_CN": '变脸',
        "zh_TW": '變臉',
        "en": 'Morph',
        "ja": 'モーフ',
    },
    '右脸': {
        "zh_CN": '右脸',
        "zh_TW": '右臉',
        "en": 'Right',
        "ja": '右顔',
    },
    '右脸定位结果': {
        "zh_CN": '右脸定位结果',
        "zh_TW": '右臉定位结果',
        "en": 'Right face result',
        "ja": '右顔マーキング結果',
    },
    '名称': {
        "zh_CN": '名称',
        "zh_TW": '名称',
        "en": 'Name',
        "ja": '名称',
    },
    '四': {
        "zh_CN": '四',
        "zh_TW": '四',
        "en": 'Thu',
        "ja": '木',
    },
    '回到 用户一览': {
        "zh_CN": '回到 用户一览',
        "zh_TW": '回到 用戶一覽',
        "en": 'Customers',
        "ja": '顧客一覧へ',
    },
    '回到Home': {
        "zh_CN": '回到Home',
        "zh_TW": '回到Home',
        "en": 'Home',
        "ja": 'ホームへ',
    },
    '图片 (*.png *.jpg *.jpeg *.bmp)': {
        "zh_CN": '图片 (*.png *.jpg *.jpeg *.bmp)',
        "zh_TW": '圖片 (*.png *.jpg *.jpeg *.bmp)',
        "en": 'Images (*.png *.jpg *.jpeg *.bmp)',
        "ja": '画像 (*.png *.jpg *.jpeg *.bmp)',
    },
    '图片尺寸': {
        "zh_CN": '图片尺寸',
        "zh_TW": '圖片尺寸',
        "en": 'Image size',
        "ja": '画像サイズ',
    },
    '图片质量': {
        "zh_CN": '图片质量',
        "zh_TW": '圖片質量',
        "en": 'Image quality',
        "ja": '画質',
    },
    '均匀度': {
        "zh_CN": '均匀度',
        "zh_TW": '均匀度',
        "en": 'Evenness',
        "ja": '均一性',
    },
    '处理中: %1%': {
        "zh_CN": '处理中: %1%',
        "zh_TW": '处理中: %1%',
        "en": 'Processing: %1%',
        "ja": '処理中: %1%',
    },
    '备份': {
        "zh_CN": '备份',
        "zh_TW": '備份',
        "en": 'Backup',
        "ja": 'バックアップ',
    },
    '备份与恢复': {
        "zh_CN": '备份与恢复',
        "zh_TW": '備份與恢復',
        "en": 'Backup & Restore',
        "ja": 'バックアップと復元',
    },
    '备份成功！': {
        "zh_CN": '备份成功！',
        "zh_TW": '備份成功！',
        "en": 'Backup successful!',
        "ja": 'バックアップ成功！',
    },
    '备份文件损坏或解压失败': {
        "zh_CN": '备份文件损坏或解压失败',
        "zh_TW": '備份文件损坏或解压失敗',
        "en": 'Backup archive corrupted or extract failed',
        "ja": 'バックアップファイル破損または展開失敗',
    },
    '女': {
        "zh_CN": '女',
        "zh_TW": '女',
        "en": 'Female',
        "ja": '女性',
    },
    '好': {
        "zh_CN": '好',
        "zh_TW": '好',
        "en": 'Good',
        "ja": '良',
    },
    '定位失败': {
        "zh_CN": '定位失败',
        "zh_TW": '定位失敗',
        "en": 'Marking Failed',
        "ja": 'マーキング失敗',
    },
    '定位结果': {
        "zh_CN": '定位结果',
        "zh_TW": '定位結果',
        "en": 'Marking Result',
        "ja": 'マーキング結果',
    },
    '客户信息': {
        "zh_CN": '客户信息',
        "zh_TW": '客戶信息',
        "en": 'Customer data',
        "ja": '顧客情報',
    },
    '客户信息填写': {
        "zh_CN": '客户信息填写',
        "zh_TW": '客戶信息填寫',
        "en": 'Customer information',
        "ja": '顧客情報入力',
    },
    '客户姓名': {
        "zh_CN": '客户姓名',
        "zh_TW": '客戶姓名',
        "en": 'Name',
        "ja": '顧客名',
    },
    '客户姓名: ': {
        "zh_CN": '客户姓名: ',
        "zh_TW": '客戶姓名: ',
        "en": 'Name: ',
        "ja": '顧客名: ',
    },
    '客户报告': {
        "zh_CN": '客户报告',
        "zh_TW": '客戶報告',
        "en": 'Customer Report',
        "ja": '顧客レポート',
    },
    '客户电话': {
        "zh_CN": '客户电话',
        "zh_TW": '客戶電話',
        "en": 'Phone',
        "ja": '電話番号',
    },
    '客户管理': {
        "zh_CN": '客户管理',
        "zh_TW": '客戶管理',
        "en": 'Customer Management',
        "ja": '顧客管理',
    },
    '客户编号': {
        "zh_CN": '客户编号',
        "zh_TW": '客戶編號',
        "en": 'Customer ID',
        "ja": '顧客番号',
    },
    '客户编号: ': {
        "zh_CN": '客户编号: ',
        "zh_TW": '客戶編號: ',
        "en": 'ID: ',
        "ja": '顧客番号: ',
    },
    '对比': {
        "zh_CN": '对比',
        "zh_TW": '對比',
        "en": 'Compare',
        "ja": '比較',
    },
    '导入功能开发中...': {
        "zh_CN": '导入功能开发中...',
        "zh_TW": '导入功能开发中...',
        "en": 'Import feature coming soon...',
        "ja": 'インポート機能は開発中です...',
    },
    '导入用': {
        "zh_CN": '导入用',
        "zh_TW": '导入用',
        "en": 'For import',
        "ja": 'インポート用',
    },
    '左右脸轮廓已就绪，请选择下一步：': {
        "zh_CN": '左右脸轮廓已就绪，请选择下一步：',
        "zh_TW": '左右臉轮廓已就绪，請選擇下一步：',
        "en": 'Left/right contours are ready. Choose next step:',
        "ja": '左右の輪郭が準備できました。次の操作を選択してください：',
    },
    '左脸': {
        "zh_CN": '左脸',
        "zh_TW": '左臉',
        "en": 'Left',
        "ja": '左顔',
    },
    '左脸定位结果': {
        "zh_CN": '左脸定位结果',
        "zh_TW": '左臉定位结果',
        "en": 'Left face result',
        "ja": '左顔マーキング結果',
    },
    '差': {
        "zh_CN": '差',
        "zh_TW": '差',
        "en": 'Poor',
        "ja": '差',
    },
    '已选产品': {
        "zh_CN": '已选产品',
        "zh_TW": '已選產品',
        "en": 'Selected products',
        "ja": '選択済み製品',
    },
    '平移': {
        "zh_CN": '平移',
        "zh_TW": '平移',
        "en": 'Pan',
        "ja": '移動',
    },
    '建议（约可显示 300 字）': {
        "zh_CN": '建议（约可显示 300 字）',
        "zh_TW": '建议（约可顯示 300 字）',
        "en": 'Suggestion (~300 chars)',
        "ja": '提案（約300文字）',
    },
    '开始': {
        "zh_CN": '开始',
        "zh_TW": '开始',
        "en": 'Start',
        "ja": '開始',
    },
    '开灯': {
        "zh_CN": '开灯',
        "zh_TW": '開燈',
        "en": 'Lights on',
        "ja": '照明オン',
    },
    '彩点显示': {
        "zh_CN": '彩点显示',
        "zh_TW": '彩点顯示',
        "en": 'Overlay view',
        "ja": 'オーバーレイ表示',
    },
    '性别': {
        "zh_CN": '性别',
        "zh_TW": '性別',
        "en": 'Gender',
        "ja": '性別',
    },
    '性别: ': {
        "zh_CN": '性别: ',
        "zh_TW": '性別: ',
        "en": 'Gender: ',
        "ja": '性別: ',
    },
    '恢复': {
        "zh_CN": '恢复',
        "zh_TW": '恢復',
        "en": 'Restore',
        "ja": '復元',
    },
    '恢复原定位或模板失败，请重试。': {
        "zh_CN": '恢复原定位或模板失败，请重试。',
        "zh_TW": '恢復原定位或模板失敗，請重试。',
        "en": 'Failed to restore previous contour. Retry.',
        "ja": '前回のマーキングまたはテンプレートの復元に失敗しました。再試行してください。',
    },
    '恢复成功！': {
        "zh_CN": '恢复成功！',
        "zh_TW": '恢復成功！',
        "en": 'Restore successful!',
        "ja": '復元成功！',
    },
    '您确定要删除 ID 为 ': {
        "zh_CN": '您确定要删除 ID 为 ',
        "zh_TW": '您确定要刪除 ID 為 ',
        "en": 'Delete customer ID ',
        "ja": 'ID ',
    },
    '手动精修轮廓': {
        "zh_CN": '手动精修轮廓',
        "zh_TW": '手動精修輪廓',
        "en": 'Refine Manually',
        "ja": '手動で輪郭調整',
    },
    '关闭': {
        "zh_CN": '关闭',
        "zh_TW": '關閉',
        "en": 'Close',
        "ja": '閉じる',
    },
    '自动定位未成功，可进入手动精修轮廓。': {
        "zh_CN": '自动定位未成功，可进入手动精修轮廓。',
        "zh_TW": '自動定位未成功，可進入手動精修輪廓。',
        "en": 'Auto mark failed. You can refine the contour manually.',
        "ja": '自動マーキングに失敗しました。手動で輪郭を調整できます。',
    },
    '手机号': {
        "zh_CN": '手机号',
        "zh_TW": '手機號',
        "en": 'Mobile',
        "ja": '携帯番号',
    },
    '报告': {
        "zh_CN": '报告',
        "zh_TW": '報告',
        "en": 'Report',
        "ja": 'レポート',
    },
    '报告摘要: ': {
        "zh_CN": '报告摘要: ',
        "zh_TW": '報告摘要: ',
        "en": 'Summary: ',
        "ja": '概要: ',
    },
    '报告日期: ': {
        "zh_CN": '报告日期: ',
        "zh_TW": '報告日期: ',
        "en": 'Report date: ',
        "ja": 'レポート日: ',
    },
    '拍摄': {
        "zh_CN": '拍摄',
        "zh_TW": '拍攝',
        "en": 'Capture',
        "ja": '撮影',
    },
    '指数 ': {
        "zh_CN": '指数 ',
        "zh_TW": '指数 ',
        "en": 'Score ',
        "ja": '指数 ',
    },
    '推荐产品/服务': {
        "zh_CN": '推荐产品/服务',
        "zh_TW": '推荐產品/服務',
        "en": 'Recommended products',
        "ja": 'おすすめ製品・サービス',
    },
    '推荐产品/服务（5–10项）': {
        "zh_CN": '推荐产品/服务（5–10项）',
        "zh_TW": '推荐產品/服務（5–10项）',
        "en": 'Recommended products (5–10)',
        "ja": 'おすすめ製品・サービス（5〜10件）',
    },
    '提示': {
        "zh_CN": '提示',
        "zh_TW": '提示',
        "en": 'Notice',
        "ja": 'お知らせ',
    },
    '搜索方式:': {
        "zh_CN": '搜索方式:',
        "zh_TW": '搜尋方式:',
        "en": 'Search by:',
        "ja": '検索方法:',
    },
    '操作完成': {
        "zh_CN": '操作完成',
        "zh_TW": '操作完成',
        "en": 'Done',
        "ja": '操作完了',
    },
    '放大镜': {
        "zh_CN": '放大镜',
        "zh_TW": '放大鏡',
        "en": 'Magnifier',
        "ja": '拡大鏡',
    },
    '新增用户': {
        "zh_CN": '新增用户',
        "zh_TW": '新增用戶',
        "en": 'Add Customer',
        "ja": '新規顧客',
    },
    '方位角 ': {
        "zh_CN": '方位角 ',
        "zh_TW": '方位角 ',
        "en": 'Azimuth ',
        "ja": '方位角 ',
    },
    '无可用左右图数据，无法生成 3D': {
        "zh_CN": '无可用左右图数据，无法生成 3D',
        "zh_TW": '无可用左右圖数据，无法生成 3D',
        "en": 'No left/right images available for 3D',
        "ja": '左右画像がないため 3D を生成できません',
    },
    '无图': {
        "zh_CN": '无图',
        "zh_TW": '无圖',
        "en": 'No image',
        "ja": '画像なし',
    },
    '无效的客户或组号': {
        "zh_CN": '无效的客户或组号',
        "zh_TW": '无效的客戶或组號',
        "en": 'Invalid customer or group',
        "ja": '無効な顧客またはグループ番号',
    },
    '无法复制数据库文件': {
        "zh_CN": '无法复制数据库文件',
        "zh_TW": '无法復制数据库文件',
        "en": 'Cannot copy database file',
        "ja": 'データベースファイルをコピーできません',
    },
    '无法暂存旧数据': {
        "zh_CN": '无法暂存旧数据',
        "zh_TW": '无法暂存旧数据',
        "en": 'Cannot stage old data',
        "ja": '旧データを一時保存できません',
    },
    '日': {
        "zh_CN": '日',
        "zh_TW": '日',
        "en": 'Sun',
        "ja": '日',
    },
    '是否保存当前修改到数据库？': {
        "zh_CN": '是否保存当前修改到数据库？',
        "zh_TW": '是否保存当前修改到数据库？',
        "en": 'Save changes to database?',
        "ja": '変更をデータベースに保存しますか？',
    },
    '是否现在对左右脸做自动轮廓定位？': {
        "zh_CN": '是否现在对左右脸做自动轮廓定位？',
        "zh_TW": '是否現在對左右臉做自動輪廓定位？',
        "en": 'Run auto contour on left and right now?',
        "ja": '左右の輪郭を自動マーキングしますか？',
    },
    '显示轮廓': {
        "zh_CN": '显示轮廓',
        "zh_TW": '顯示轮廓',
        "en": 'Show contour',
        "ja": '輪郭表示',
    },
    '显示：分析图': {
        "zh_CN": '显示：分析图',
        "zh_TW": '顯示：分析圖',
        "en": 'View: Analysis',
        "ja": '表示：分析画像',
    },
    '显示：原图': {
        "zh_CN": '显示：原图',
        "zh_TW": '顯示：原圖',
        "en": 'View: Original',
        "ja": '表示：原画',
    },
    '显示：对比': {
        "zh_CN": '显示：对比',
        "zh_TW": '顯示：對比',
        "en": 'View: Compare',
        "ja": '表示：比較',
    },
    '未命名': {
        "zh_CN": '未命名',
        "zh_TW": '未命名',
        "en": 'Unnamed',
        "ja": '名称未設定',
    },
    '未找到分析配置': {
        "zh_CN": '未找到分析配置',
        "zh_TW": '未找到分析配置',
        "en": 'Analyse config not found',
        "ja": '分析設定が見つかりません',
    },
    '未找到客户信息': {
        "zh_CN": '未找到客户信息',
        "zh_TW": '未找到客戶信息',
        "en": 'Customer not found',
        "ja": '顧客情報が見つかりません',
    },
    '未知': {
        "zh_CN": '未知',
        "zh_TW": '未知',
        "en": 'Unknown',
        "ja": '不明',
    },
    '未能连接相机程序，请联系客服。': {
        "zh_CN": '未能连接相机程序，请联系客服。',
        "zh_TW": '未能連接相機程式，請聯繫客服。',
        "en": 'Cannot connect to camera app. Contact support.',
        "ja": 'カメラプログラムに接続できません。サポートにお問い合わせください。',
    },
    '未选择': {
        "zh_CN": '未选择',
        "zh_TW": '未選擇',
        "en": 'None selected',
        "ja": '未選択',
    },
    '检测报告': {
        "zh_CN": '检测报告',
        "zh_TW": '检測報告',
        "en": 'Analysis Report',
        "ja": '分析レポート',
    },
    '橡皮擦': {
        "zh_CN": '橡皮擦',
        "zh_TW": '橡皮擦',
        "en": 'Eraser',
        "ja": '消しゴム',
    },
    '正在压缩/解压，请稍候…': {
        "zh_CN": '正在压缩/解压，请稍候…',
        "zh_TW": '正在压缩/解压，請稍候…',
        "en": 'Compressing/extracting, please wait…',
        "ja": '圧縮/展開中、お待ちください…',
    },
    '正在处理中，请稍候': {
        "zh_CN": '正在处理中，请稍候',
        "zh_TW": '正在处理中，請稍候',
        "en": 'Busy, please wait',
        "ja": '処理中です。お待ちください',
    },
    '正在安全暂存当前数据...': {
        "zh_CN": '正在安全暂存当前数据...',
        "zh_TW": '正在安全暂存当前数据...',
        "en": 'Safely staging current data...',
        "ja": '現在のデータを安全に退避中...',
    },
    '正在定位中，请稍候': {
        "zh_CN": '正在定位中，请稍候',
        "zh_TW": '正在定位中，請稍候',
        "en": 'Marking in progress, please wait',
        "ja": 'マーキング中です。お待ちください',
    },
    '正在打开相机，请稍候（': {
        "zh_CN": '正在打开相机，请稍候（',
        "zh_TW": '正在打开相机，請稍候（',
        "en": 'Opening camera, please wait (',
        "ja": 'カメラを起動中（',
    },
    '正在校验备份包...': {
        "zh_CN": '正在校验备份包...',
        "zh_TW": '正在校验備份包...',
        "en": 'Verifying backup archive...',
        "ja": 'バックアップを検証中...',
    },
    '正在生成 3D 模型…': {
        "zh_CN": '正在生成 3D 模型…',
        "zh_TW": '正在生成 3D 模型…',
        "en": 'Generating 3D model…',
        "ja": '3D モデル生成中…',
    },
    '正在皮肤分析…': {
        "zh_CN": '正在皮肤分析…',
        "zh_TW": '正在皮膚分析…',
        "en": 'Analyzing skin…',
        "ja": '肌分析中…',
    },
    '正在自动定位轮廓…': {
        "zh_CN": '正在自动定位轮廓…',
        "zh_TW": '正在自動定位輪廓…',
        "en": 'Auto-marking contour…',
        "ja": '輪郭を自動マーキング中…',
    },
    '正在连接相机打开预览': {
        "zh_CN": '正在连接相机打开预览',
        "zh_TW": '正在連接相机打开預覽',
        "en": 'Connecting camera preview',
        "ja": 'カメラに接続してプレビューを開いています',
    },
    '正在部署新文件...': {
        "zh_CN": '正在部署新文件...',
        "zh_TW": '正在部署新文件...',
        "en": 'Deploying new files...',
        "ja": '新しいファイルを展開中...',
    },
    '正在重新连接数据库...': {
        "zh_CN": '正在重新连接数据库...',
        "zh_TW": '正在重新連接数据库...',
        "en": 'Reconnecting database...',
        "ja": 'データベースに再接続中...',
    },
    '每周 2–3 次，敷 15 分钟。': {
        "zh_CN": '每周 2–3 次，敷 15 分钟。',
        "zh_TW": '每周 2–3 次，敷 15 分钟。',
        "en": 'Use 2–3 times per week, 15 minutes each.',
        "ja": '週2〜3回、15分間使用。',
    },
    '每日早晚使用，配合按摩。': {
        "zh_CN": '每日早晚使用，配合按摩。',
        "zh_TW": '每日早晚使用，配合按摩。',
        "en": 'Use morning and evening with massage.',
        "ja": '朝夕使用、マッサージと併用。',
    },
    '每种报告分 好 / 中 / 差 三档，每档填写建议并勾选推荐产品/服务。请切换下方 Tab 选择报告类型。': {
        "zh_CN": '每种报告分 好 / 中 / 差 三档，每档填写建议并勾选推荐产品/服务。请切换下方 Tab 选择报告类型。',
        "zh_TW": '每种報告分 好 / 中 / 差 三档，每档填寫建议并勾選推荐產品/服務。請切换下方 Tab 選擇報告类型。',
        "en": 'Each report has Good/Medium/Poor tiers with suggestions and products.',
        "ja": '各レポートは良/中/差の3段階。提案とおすすめ製品を設定し、下のタブで種類を選択してください。',
    },
    '毛孔': {
        "zh_CN": '毛孔',
        "zh_TW": '毛孔',
        "en": 'Pores',
        "ja": '毛穴',
    },
    '水分': {
        "zh_CN": '水分',
        "zh_TW": '水分',
        "en": 'Moisture',
        "ja": '水分',
    },
    '水分测量': {
        "zh_CN": '水分测量',
        "zh_TW": '水分測量',
        "en": 'Moisture',
        "ja": '水分測定',
    },
    '请输入水分值（0–99）': {
        "zh_CN": '请输入水分值（0–99）',
        "zh_TW": '請輸入水分值（0–99）',
        "en": 'Enter moisture (0–99)',
        "ja": '水分値を入力（0–99）',
    },
    '%1 WHOLE 未找到照片': {
        "zh_CN": '%1 WHOLE 未找到照片',
        "zh_TW": '%1 WHOLE 未找到照片',
        "en": '%1 WHOLE photo not found',
        "ja": '%1 WHOLE の写真が見つかりません',
    },
    '%1 水分保存失败：%2': {
        "zh_CN": '%1 水分保存失败：%2',
        "zh_TW": '%1 水分保存失敗：%2',
        "en": '%1 moisture save failed: %2',
        "ja": '%1 水分の保存に失敗：%2',
    },
    '测量': {
        "zh_CN": '测量',
        "zh_TW": '測量',
        "en": 'Measure',
        "ja": '測定',
    },
    '浏览': {
        "zh_CN": '浏览',
        "zh_TW": '浏覽',
        "en": 'Browse',
        "ja": '参照',
    },
    '添加': {
        "zh_CN": '添加',
        "zh_TW": '添加',
        "en": 'Add',
        "ja": '追加',
    },
    '清除旧分析结果失败': {
        "zh_CN": '清除旧分析结果失败',
        "zh_TW": '清除旧分析结果失敗',
        "en": 'Failed to clear old analyse results',
        "ja": '旧分析結果の削除に失敗しました',
    },
    '点击关闭 · 30 秒内不再提示': {
        "zh_CN": '点击关闭 · 30 秒内不再提示',
        "zh_TW": '点击关闭 · 30 秒内不再提示',
        "en": 'Click to dismiss · no reminder for 30s',
        "ja": 'クリックで閉じる · 30秒間表示しません',
    },
    '点击选图': {
        "zh_CN": '点击选图',
        "zh_TW": '点击選圖',
        "en": 'Click to select image',
        "ja": 'クリックして画像を選択',
    },
    '生日': {
        "zh_CN": '生日',
        "zh_TW": '生日',
        "en": 'Birthday',
        "ja": '誕生日',
    },
    '用户管理': {
        "zh_CN": '用户管理',
        "zh_TW": '用戶管理',
        "en": 'User Management',
        "ja": 'ユーザー管理',
    },
    '电话: ': {
        "zh_CN": '电话: ',
        "zh_TW": '電話: ',
        "en": 'Phone: ',
        "ja": '電話: ',
    },
    '男': {
        "zh_CN": '男',
        "zh_TW": '男',
        "en": 'Male',
        "ja": '男性',
    },
    '画圆': {
        "zh_CN": '画圆',
        "zh_TW": '画圆',
        "en": 'Circle',
        "ja": '円',
    },
    '痤疮': {
        "zh_CN": '痤疮',
        "zh_TW": '痤疮',
        "en": 'Acne',
        "ja": 'にきび',
    },
    '登记日': {
        "zh_CN": '登记日',
        "zh_TW": '登記日',
        "en": 'Registered',
        "ja": '登録日',
    },
    '登记时间: ': {
        "zh_CN": '登记时间: ',
        "zh_TW": '登記時間: ',
        "en": 'Registered: ',
        "ja": '登録日: ',
    },
    '皮肤分析': {
        "zh_CN": '皮肤分析',
        "zh_TW": '皮膚分析',
        "en": 'Skin Analysis',
        "ja": '肌分析',
    },
    '皮肤分析失败。\\n': {
        "zh_CN": '皮肤分析失败。\\n',
        "zh_TW": '皮膚分析失敗。\\n',
        "en": 'Skin analysis failed.\\n',
        "ja": '肌分析に失敗しました。\\n',
    },
    '皮肤分析完成，共 %1 项。': {
        "zh_CN": '皮肤分析完成，共 %1 项。',
        "zh_TW": '皮膚分析完成，共 %1 项。',
        "en": 'Skin analysis complete: %1 items.',
        "ja": '肌分析完了：%1 項目。',
    },
    '皮肤分析部分完成：成功 %1 项，失败 %2 项。\\n%3': {
        "zh_CN": '皮肤分析部分完成：成功 %1 项，失败 %2 项。\\n%3',
        "zh_TW": '皮膚分析部分完成：成功 %1 项，失敗 %2 项。\\n%3',
        "en": 'Skin analysis partially complete: %1 succeeded, %2 failed.\\n%3',
        "ja": '肌分析が一部完了：成功 %1 件、失敗 %2 件。\\n%3',
    },
    '皱纹': {
        "zh_CN": '皱纹',
        "zh_TW": '皺紋',
        "en": 'Wrinkles',
        "ja": 'しわ',
    },
    '相机打开失败，请检查连接': {
        "zh_CN": '相机打开失败，请检查连接',
        "zh_TW": '相机打开失敗，請检查連接',
        "en": 'Failed to open camera. Check connection.',
        "ja": 'カメラを開けません。接続を確認してください',
    },
    '确定': {
        "zh_CN": '确定',
        "zh_TW": '確定',
        "en": 'OK',
        "ja": '確定',
    },
    '确定要删除该客户吗？': {
        "zh_CN": '确定要删除该客户吗？',
        "zh_TW": '確定要刪除該客戶嗎？',
        "en": 'Delete this customer?',
        "ja": 'この顧客を削除しますか？',
    },
    '确认': {
        "zh_CN": '确认',
        "zh_TW": '確認',
        "en": 'Confirm',
        "ja": '確認',
    },
    '确认删除': {
        "zh_CN": '确认删除',
        "zh_TW": '確認刪除',
        "en": 'Confirm Delete',
        "ja": '削除の確認',
    },
    '示例精华液': {
        "zh_CN": '示例精华液',
        "zh_TW": '示例精华液',
        "en": 'Sample Serum',
        "ja": 'サンプル美容液',
    },
    '示例面膜': {
        "zh_CN": '示例面膜',
        "zh_TW": '示例面膜',
        "en": 'Sample Mask',
        "ja": 'サンプルマスク',
    },
    '秒）': {
        "zh_CN": '秒）',
        "zh_TW": '秒）',
        "en": 's)',
        "ja": '秒）',
    },
    '稍后': {
        "zh_CN": '稍后',
        "zh_TW": '稍后',
        "en": 'Later',
        "ja": '後で',
    },
    '精修轮廓': {
        "zh_CN": '精修轮廓',
        "zh_TW": '精修轮廓',
        "en": 'Refine contour',
        "ja": '輪郭を調整',
    },
    '约 %1 秒后自动左右摆动': {
        "zh_CN": '约 %1 秒后自动左右摆动',
        "zh_TW": '约 %1 秒后自动左右摆动',
        "en": 'Auto swing in ~%1 s',
        "ja": '約 %1 秒後に自動スイング',
    },
    '维护产品/服务目录，供报告预录时选用。可上传照片、填写名称、价格、功能说明。': {
        "zh_CN": '维护产品/服务目录，供报告预录时选用。可上传照片、填写名称、价格、功能说明。',
        "zh_TW": '维护產品/服務目錄，供報告預錄時選用。可上传照片、填寫名称、价格、功能说明。',
        "en": 'Maintain product/service catalog for reports.',
        "ja": 'レポート用の製品・サービス一覧を管理。写真・名称・価格・説明を登録できます。',
    },
    '综合指数 ': {
        "zh_CN": '综合指数 ',
        "zh_TW": '综合指数 ',
        "en": 'Overall score ',
        "ja": '総合指数 ',
    },
    '综合皮肤检测报告': {
        "zh_CN": '综合皮肤检测报告',
        "zh_TW": '综合皮膚检測報告',
        "en": 'Comprehensive Skin Report',
        "ja": '総合肌分析レポート',
    },
    '编辑信息': {
        "zh_CN": '编辑信息',
        "zh_TW": '編輯資訊',
        "en": 'Edit',
        "ja": '編集',
    },
    '肌肤皱纹: ': {
        "zh_CN": '肌肤皱纹: ',
        "zh_TW": '肌膚皺紋: ',
        "en": 'Wrinkles: ',
        "ja": 'しわ: ',
    },
    '肌肤粉刺: ': {
        "zh_CN": '肌肤粉刺: ',
        "zh_TW": '肌膚粉刺: ',
        "en": 'Acne: ',
        "ja": 'にきび: ',
    },
    '肌肤色斑: ': {
        "zh_CN": '肌肤色斑: ',
        "zh_TW": '肌膚色斑: ',
        "en": 'Spots: ',
        "ja": 'シミ: ',
    },
    '肌肤血红斑: ': {
        "zh_CN": '肌肤血红斑: ',
        "zh_TW": '肌膚血紅斑: ',
        "en": 'Erythema: ',
        "ja": '赤み: ',
    },
    '自动区域定位': {
        "zh_CN": '自动区域定位',
        "zh_TW": '自動區域定位',
        "en": 'Auto Contour',
        "ja": '自動輪郭マーキング',
    },
    '自动定位': {
        "zh_CN": '自动定位',
        "zh_TW": '自動定位',
        "en": 'Auto Mark',
        "ja": '自動マーキング',
    },
    '自动（跟随系统）': {
        "zh_CN": '自动（跟随系统）',
        "zh_TW": '自动（跟随系统）',
        "en": 'Auto (System)',
        "ja": '自動（システムに従う）',
    },
    '色斑': {
        "zh_CN": '色斑',
        "zh_TW": '色斑',
        "en": 'Spots',
        "ja": 'シミ',
    },
    '设定': {
        "zh_CN": '设定',
        "zh_TW": '設定',
        "en": 'Settings',
        "ja": '設定',
    },
    '诊断建议': {
        "zh_CN": '诊断建议',
        "zh_TW": '诊断建议',
        "en": 'Diagnosis',
        "ja": '診断アドバイス',
    },
    '诊断建议（约100字）': {
        "zh_CN": '诊断建议（约100字）',
        "zh_TW": '诊断建议（约100字）',
        "en": 'Diagnosis notes (~100 chars)',
        "ja": '診断アドバイス（約100文字）',
    },
    '诊断等级': {
        "zh_CN": '诊断等级',
        "zh_TW": '診斷等級',
        "en": 'Diagnosis Tier',
        "ja": '診断ランク',
    },
    '诊断等级：': {
        "zh_CN": '诊断等级：',
        "zh_TW": '诊断等级：',
        "en": 'Diagnosis tier: ',
        "ja": '診断ランク：',
    },
    '语言': {
        "zh_CN": '语言',
        "zh_TW": '語言',
        "en": 'Language',
        "ja": '言語',
    },
    '请': {
        "zh_CN": '请',
        "zh_TW": '請',
        "en": 'Please ',
        "ja": 'お願い：',
    },
    '请使用右侧「取消」或「保存」\\n退出拍摄': {
        "zh_CN": '请使用右侧「取消」或「保存」\\n退出拍摄',
        "zh_TW": '請使用右侧「取消」或「保存」\\n退出拍攝',
        "en": 'Use Cancel or Save on the right\\nto exit capture',
        "ja": '右側の「キャンセル」または「保存」で\\n撮影を終了してください',
    },
    '请先完成左右脸区域定位': {
        "zh_CN": '请先完成左右脸区域定位',
        "zh_TW": '請先完成左右臉区域定位',
        "en": 'Complete left/right contour marking first',
        "ja": '先に左右の輪郭マーキングを完了してください',
    },
    '请先完成自动区域定位。': {
        "zh_CN": '请先完成自动区域定位。',
        "zh_TW": '請先完成自動區域定位。',
        "en": 'Complete auto contour first.',
        "ja": '先に自動輪郭マーキングを完了してください。',
    },
    '请先完成皮肤分析后再查看报告。': {
        "zh_CN": '请先完成皮肤分析后再查看报告。',
        "zh_TW": '請先完成皮膚分析後再查看報告。',
        "en": 'Complete skin analysis before opening the report.',
        "ja": 'レポートを開く前に肌分析を完了してください。',
    },
    '请先连接相机': {
        "zh_CN": '请先连接相机',
        "zh_TW": '請先連接相机',
        "en": 'Connect camera first',
        "ja": '先にカメラを接続してください',
    },
    '请先选择要定位的照片组。': {
        "zh_CN": '请先选择要定位的照片组。',
        "zh_TW": '請先選擇要定位的照片組。',
        "en": 'Select a photo group first.',
        "ja": '先にマーキングする写真グループを選択してください。',
    },
    '请填写产品名称（第 %1 条名称为空）': {
        "zh_CN": '请填写产品名称（第 %1 条名称为空）',
        "zh_TW": '請填寫產品名称（第 %1 条名称為空）',
        "en": 'Please enter product name (row %1 is empty)',
        "ja": '製品名を入力してください（%1 行目が空です）',
    },
    '请填写价格（第 %1 条价格为空，0 为有效值）': {
        "zh_CN": '请填写价格（第 %1 条价格为空，0 为有效值）',
        "zh_TW": '請填寫价格（第 %1 条价格為空，0 為有效值）',
        "en": 'Please enter price (row %1 is empty; 0 is valid)',
        "ja": '価格を入力してください（%1 行目が空です。0は有効）',
    },
    '请确认是否保留本次自动定位结果。': {
        "zh_CN": '请确认是否保留本次自动定位结果。',
        "zh_TW": '請確認是否保留本次自动定位结果。',
        "en": 'Keep this auto-mark result?',
        "ja": '今回の自動マーキング結果を保持しますか？',
    },
    '请输入Email': {
        "zh_CN": '请输入Email',
        "zh_TW": '請輸入Email',
        "en": 'Enter email',
        "ja": 'メールを入力',
    },
    '请输入客户姓名': {
        "zh_CN": '请输入客户姓名',
        "zh_TW": '請輸入客戶姓名',
        "en": 'Enter customer name',
        "ja": '顧客名を入力',
    },
    '请输入手机号': {
        "zh_CN": '请输入手机号',
        "zh_TW": '請輸入手機號',
        "en": 'Enter mobile number',
        "ja": '携帯番号を入力',
    },
    '请输入搜索内容': {
        "zh_CN": '请输入搜索内容',
        "zh_TW": '請輸入搜尋內容',
        "en": 'Enter search text',
        "ja": '検索内容を入力',
    },
    '请选择': {
        "zh_CN": '请选择',
        "zh_TW": '請選擇',
        "en": 'Please select',
        "ja": '選択してください',
    },
    '请选择下一步操作：': {
        "zh_CN": '请选择下一步操作：',
        "zh_TW": '請選擇下一步操作：',
        "en": 'Choose next step:',
        "ja": '次の操作を選択してください：',
    },
    '请选择下一步：': {
        "zh_CN": '请选择下一步：',
        "zh_TW": '請選擇下一步：',
        "en": 'Choose next step:',
        "ja": '次の操作を選択してください：',
    },
    '请选择文件保存路径': {
        "zh_CN": '请选择文件保存路径',
        "zh_TW": '請選擇文件保存路径',
        "en": 'Choose save path',
        "ja": '保存先パスを選択してください',
    },
    '读取左右脸 ROI 失败，请先完成区域定位': {
        "zh_CN": '读取左右脸 ROI 失败，请先完成区域定位',
        "zh_TW": '读取左右臉 ROI 失敗，請先完成区域定位',
        "en": 'Failed to read ROI. Complete contour marking first.',
        "ja": '左右 ROI の読み取りに失敗。先に輪郭マーキングを完了してください',
    },
    '资料备份': {
        "zh_CN": '资料备份',
        "zh_TW": '資料備份',
        "en": 'Backup & Restore',
        "ja": 'バックアップと復元',
    },
    '轮廓定位结果': {
        "zh_CN": '轮廓定位结果',
        "zh_TW": '輪廓定位結果',
        "en": 'Contour Result',
        "ja": '輪郭マーキング結果',
    },
    '输入名称或功能说明筛选': {
        "zh_CN": '输入名称或功能说明筛选',
        "zh_TW": '輸入名称或功能说明筛選',
        "en": 'Filter by name or description',
        "ja": '名称または説明で絞り込み',
    },
    '返回': {
        "zh_CN": '返回',
        "zh_TW": '返回',
        "en": 'Back',
        "ja": '戻る',
    },
    '进入分析': {
        "zh_CN": '进入分析',
        "zh_TW": '進入分析',
        "en": 'Start Analysis',
        "ja": '分析へ',
    },
    '退出变脸': {
        "zh_CN": '退出变脸',
        "zh_TW": '退出變臉',
        "en": 'Exit Morph',
        "ja": 'モーフ終了',
    },
    '退出模型': {
        "zh_CN": '退出模型',
        "zh_TW": '退出模型',
        "en": 'Exit Model',
        "ja": 'モデル終了',
    },
    '退出程序': {
        "zh_CN": '退出程序',
        "zh_TW": '退出程式',
        "en": 'Exit',
        "ja": '終了',
    },
    '选择产品/服务': {
        "zh_CN": '选择产品/服务',
        "zh_TW": '選擇產品/服務',
        "en": 'Select products/services',
        "ja": '製品・サービスを選択',
    },
    '选择产品/服务图片': {
        "zh_CN": '选择产品/服务图片',
        "zh_TW": '選擇產品/服務圖片',
        "en": 'Select product image',
        "ja": '製品画像を選択',
    },
    '选择备份保存位置': {
        "zh_CN": '选择备份保存位置',
        "zh_TW": '選擇備份保存位置',
        "en": 'Choose backup save location',
        "ja": 'バックアップ保存先を選択',
    },
    '选择备份文件进行恢复': {
        "zh_CN": '选择备份文件进行恢复',
        "zh_TW": '選擇備份文件进行恢復',
        "en": 'Choose backup file to restore',
        "ja": '復元するバックアップファイルを選択',
    },
    '选择照片': {
        "zh_CN": '选择照片',
        "zh_TW": '選擇照片',
        "en": 'Select photo',
        "ja": '写真を選択',
    },
    '邮件': {
        "zh_CN": '邮件',
        "zh_TW": '邮件',
        "en": 'Email',
        "ja": 'メール',
    },
    '部署失败，已自动回滚至原始数据。': {
        "zh_CN": '部署失败，已自动回滚至原始数据。',
        "zh_TW": '部署失敗，已自动回滚至原始数据。',
        "en": 'Deploy failed; rolled back to original data.',
        "ja": '展開失敗。元のデータに自動ロールバックしました。',
    },
    '重新拍摄': {
        "zh_CN": '重新拍摄',
        "zh_TW": '重新拍攝',
        "en": 'Retake',
        "ja": '再撮影',
    },
    '重置光位': {
        "zh_CN": '重置光位',
        "zh_TW": '重置光位',
        "en": 'Reset light',
        "ja": '照明位置をリセット',
    },
    '错误': {
        "zh_CN": '错误',
        "zh_TW": '错誤',
        "en": 'Error',
        "ja": 'エラー',
    },
    '隐藏': {
        "zh_CN": '隐藏',
        "zh_TW": '隐藏',
        "en": 'Hide',
        "ja": '非表示',
    },
    '非法备份包：缺少数据库文件': {
        "zh_CN": '非法备份包：缺少数据库文件',
        "zh_TW": '非法備份包：缺少数据库文件',
        "en": 'Invalid backup: missing database',
        "ja": '無効なバックアップ：データベースがありません',
    },
    '预录': {
        "zh_CN": '预录',
        "zh_TW": '預錄',
        "en": 'Pre-record',
        "ja": '事前登録',
    },
    '预录建议自动填入，可修改': {
        "zh_CN": '预录建议自动填入，可修改',
        "zh_TW": '預錄建议自动填入，可修改',
        "en": 'Pre-filled suggestion, editable',
        "ja": '事前登録の提案が自動入力されます（編集可）',
    },
    '预录设置': {
        "zh_CN": '预录设置',
        "zh_TW": '預錄設置',
        "en": 'Pre-record Settings',
        "ja": '事前登録設定',
    },
    '预览': {
        "zh_CN": '预览',
        "zh_TW": '預覽',
        "en": 'Preview',
        "ja": 'プレビュー',
    },
    '预览打开失败': {
        "zh_CN": '预览打开失败',
        "zh_TW": '預覽打开失敗',
        "en": 'Failed to open preview',
        "ja": 'プレビューを開けませんでした',
    },
    '高级备份': {
        "zh_CN": '高级备份',
        "zh_TW": '高级備份',
        "en": 'Advanced backup',
        "ja": '高度なバックアップ',
    },
    '（无功能说明）': {
        "zh_CN": '（无功能说明）',
        "zh_TW": '（无功能说明）',
        "en": '(No description)',
        "ja": '（説明なし）',
    },
    '；保存轮廓失败': {
        "zh_CN": '；保存轮廓失败',
        "zh_TW": '；保存轮廓失敗',
        "en": '; failed to save contour',
        "ja": '；輪郭の保存に失敗',
    },
}

LOCALES = ["zh_CN", "zh_TW", "en", "ja"]


def collect_qml_contexts() -> dict[str, set[str]]:
    """QML qsTr() looks up by file context name, not empty context (used by C++ mmTr)."""
    contexts: dict[str, set[str]] = {}
    if not QML_ROOT.is_dir():
        return contexts
    for path in QML_ROOT.rglob("*.qml"):
        if "del" in path.parts:
            continue
        text = path.read_text(encoding="utf-8", errors="ignore")
        strings = set(QSTR_PAT.findall(text))
        if not strings:
            continue
        rel = path.relative_to(QML_ROOT).as_posix()
        for name in {rel, path.name}:
            contexts.setdefault(name, set()).update(strings)
    return contexts


def _add_context(ts: ET.Element, context_name: str, sources: set[str], locale: str) -> None:
    ctx = ET.SubElement(ts, "context")
    ET.SubElement(ctx, "name").text = context_name
    for source in sorted(sources):
        row = TABLE.get(source)
        if not row:
            continue
        msg = ET.SubElement(ctx, "message")
        ET.SubElement(msg, "source").text = source
        tr = ET.SubElement(msg, "translation")
        tr.text = row.get(locale, source)
        if tr.text == source and locale != "zh_CN":
            tr.set("type", "unfinished")
        else:
            tr.set("type", "finished")


def write_ts(locale: str, path: str) -> None:
    ts = ET.Element("TS", version="2.1", language=locale)
    # C++ mmTr / MmI18n (empty context)
    _add_context(ts, "", set(TABLE.keys()), locale)
    # QML qsTr (per-file context)
    for ctx_name, sources in sorted(collect_qml_contexts().items()):
        _add_context(ts, ctx_name, sources, locale)
    xml = minidom.parseString(ET.tostring(ts, encoding="unicode")).toprettyxml(indent="  ")
    with open(path, "w", encoding="utf-8") as f:
        f.write(xml)


def find_lrelease() -> str | None:
    for base in [
        os.environ.get("QTDIR", ""),
        r"C:\Qt\6.10.2\msvc2022_64",
        r"C:\Qt\6.9.3\msvc2022_64",
    ]:
        if not base:
            continue
        exe = os.path.join(base, "bin", "lrelease.exe")
        if os.path.isfile(exe):
            return exe
    return None


def main() -> int:
    ts_dir = ROOT
    for loc in LOCALES:
        ts_path = os.path.join(ts_dir, f"mmface_{loc}.ts")
        write_ts(loc, ts_path)
        print("wrote", ts_path)

    lrelease = find_lrelease()
    if not lrelease:
        print("WARN: lrelease not found; .qm not generated. Install Qt Linguist tools.", file=sys.stderr)
        return 1

    for loc in LOCALES:
        ts_path = os.path.join(ts_dir, f"mmface_{loc}.ts")
        qm_path = os.path.join(OUT_DIR, f"mmface_{loc}.qm")
        subprocess.check_call([lrelease, ts_path, "-qm", qm_path])
        print("wrote", qm_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
