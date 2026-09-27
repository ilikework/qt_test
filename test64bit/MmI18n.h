#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

/** C++ / QML 共用的可翻译字符串（报告名、性别等） */
class MmI18n : public QObject
{
    Q_OBJECT

public:
    static MmI18n &instance();

    Q_INVOKABLE QVariantList reportLabels() const;
    Q_INVOKABLE QString reportLabel(int index) const;
    Q_INVOKABLE QString genderLabel(int gender) const;
    Q_INVOKABLE QString tierLabel(int tier) const;
    Q_INVOKABLE QString okText() const;
    Q_INVOKABLE QString cancelText() const;
    Q_INVOKABLE QString saveText() const;

public slots:
    void retranslate();

signals:
    void stringsChanged();

private:
    explicit MmI18n(QObject *parent = nullptr);
};
