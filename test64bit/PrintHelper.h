#pragma once

#include <QObject>

class PrintHelper : public QObject
{
    Q_OBJECT
public:
    explicit PrintHelper(QObject *parent = nullptr);

    /// 打印本地图片文件，按 A4 等比居中（弹出系统打印对话框）
    Q_INVOKABLE void printImage(const QString &filePath);
};
