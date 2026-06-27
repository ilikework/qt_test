#ifndef FACEANALYSEMANAGER_H
#define FACEANALYSEMANAGER_H

#include <QObject>
#include <QString>

class FaceAnalyseManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
    explicit FaceAnalyseManager(QObject *parent = nullptr);

    bool busy() const { return busy_; }

    Q_INVOKABLE bool ensureDetector();
    Q_INVOKABLE bool deleteCustomerGroup(const QString &customerId, int groupId);
    /// Returns: none | auto | manual | template
    Q_INVOKABLE QString contourState(const QString &customerId, int groupId, const QString &dirType) const;
    Q_INVOKABLE bool groupNeedsAutoMark(const QString &customerId, int groupId) const;
    Q_INVOKABLE void autoMarkGroup(const QString &customerId, int groupId);

signals:
    void busyChanged();
    void autoMarkFinished(bool success, const QString &message);
    void errorMessage(const QString &msg);

private:
    void setBusy(bool on);

    bool detectorReady_ = false;
    bool busy_ = false;
};

#endif
