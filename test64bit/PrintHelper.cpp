#include "PrintHelper.h"

#include <QDialog>
#include <QFileInfo>
#include <QImage>
#include <QPageLayout>
#include <QPageSize>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>

PrintHelper::PrintHelper(QObject *parent)
    : QObject(parent)
{
}

void PrintHelper::printImage(const QString &filePath)
{
    const QImage img(filePath);
    if (img.isNull())
        return;

    QPrinter printer(QPrinter::HighResolution);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageOrientation(QPageLayout::Portrait);

    QPrintDialog dialog(&printer);
    if (dialog.exec() != QDialog::Accepted)
        return;

    QPainter painter(&printer);
    const QRect page = printer.pageLayout().paintRectPixels(printer.resolution());
    const QImage scaled = img.scaled(page.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    const int x = page.x() + (page.width() - scaled.width()) / 2;
    const int y = page.y() + (page.height() - scaled.height()) / 2;
    painter.drawImage(QPoint(x, y), scaled);
    painter.end();
}
