#pragma once
#include <QQuickImageProvider>
#include <QImage>
#include <QMutex>

class CameraImageProvider : public QQuickImageProvider {
public:
    CameraImageProvider()
        : QQuickImageProvider(QQuickImageProvider::Image) {}

    void setLatestImage(const QImage& img) {
        QMutexLocker lk(&mtx_);
        latest_ = img; // 深拷贝（QImage 共享数据，通常够用）
    }

    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize) override {
        Q_UNUSED(id);

        QImage img;
        {
            QMutexLocker lk(&mtx_);
            img = latest_;
        }

        if (size) *size = img.size();

        if (!requestedSize.isEmpty() && !img.isNull()) {
            return img.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        return img;
    }

private:
    QMutex mtx_;
    QImage latest_;
};
