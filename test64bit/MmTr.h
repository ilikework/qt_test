#pragma once

#include <QCoreApplication>
#include <QString>

/** 与 QML qsTr 同源：空 context，对应 translations/gen_qm.py TABLE */
inline QString mmTr(const char *sourceText, const char *disambiguation = nullptr, int n = -1)
{
    return QCoreApplication::translate("", sourceText, disambiguation, n);
}
