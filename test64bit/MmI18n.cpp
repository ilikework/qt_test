#include "MmI18n.h"

#include <QCoreApplication>

namespace {

QString t(const char *key)
{
    return QCoreApplication::translate("", key);
}

} // namespace

MmI18n &MmI18n::instance()
{
    static MmI18n inst;
    return inst;
}

MmI18n::MmI18n(QObject *parent)
    : QObject(parent)
{
}

void MmI18n::retranslate()
{
    emit stringsChanged();
}

QVariantList MmI18n::reportLabels() const
{
    QVariantList list;
    for (int i = 0; i < 9; ++i)
        list.append(reportLabel(i));
    return list;
}

QString MmI18n::reportLabel(int index) const
{
    switch (index) {
    case 0:
        return t("Pores");
    case 1:
        return t("Acne");
    case 2:
        return t("Deep Spots");
    case 3:
        return t("Surface Spots");
    case 4:
        return t("Wrinkles");
    case 5:
        return t("Sensitivity");
    case 6:
        return t("Brown Spots");
    case 7:
        return t("Mixed Spots");
    case 8:
        return t("Summary Report");
    default:
        return QString();
    }
}

QString MmI18n::genderLabel(int gender) const
{
    switch (gender) {
    case 1:
        return t("Male");
    case 2:
        return t("Female");
    default:
        return t("Unknown");
    }
}

QString MmI18n::tierLabel(int tier) const
{
    switch (tier) {
    case 0:
        return t("Good");
    case 1:
        return t("Medium");
    case 2:
        return t("Poor");
    default:
        return QString();
    }
}

QString MmI18n::okText() const
{
    return t("OK");
}

QString MmI18n::cancelText() const
{
    return t("Cancel");
}

QString MmI18n::saveText() const
{
    return t("Save");
}
