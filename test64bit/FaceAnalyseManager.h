#ifndef FACEANALYSEMANAGER_H
#define FACEANALYSEMANAGER_H

#include <QObject>
#include <QString>
#include <QUrl>

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
    /// @param fromWorkflow true=拍摄后工作流（失败模板锁定、不弹保留/恢复选择）
    Q_INVOKABLE void autoMarkGroup(const QString &customerId, int groupId, bool fromWorkflow = false);
    /// isLeft=true 左脸，false 右脸
    Q_INVOKABLE bool pendingAutoMarkSideSucceeded(bool isLeft) const;
    Q_INVOKABLE QString pendingAutoMarkSideSummary(bool isLeft) const;
    Q_INVOKABLE QString pendingAutoMarkRevertLabel(bool isLeft) const;
    Q_INVOKABLE bool confirmAutoMarkSideChoice(bool isLeft, bool keepNew);
    Q_INVOKABLE void finalizeAutoMarkChoice();
    Q_INVOKABLE void analyseGroup(const QString &customerId, int groupId);
    Q_INVOKABLE void notifyGroupAnalyseProgress(int done, int total, const QString &label);
    Q_INVOKABLE bool photoHasAnalyseOverlay(int facePhotoIx) const;
    Q_INVOKABLE QUrl photoAnalyseOverlayUrl(int facePhotoIx) const;

signals:
    void busyChanged();
    void autoMarkFinished(bool success, const QString &message, bool needsResultChoice);
    void groupAnalyseProgress(int done, int total, const QString &label);
    void groupAnalyseFinished(bool success, const QString &message);
    void errorMessage(const QString &msg);

private:
    void setBusy(bool on);
    void clearPendingAutoMarkChoice();

    bool detectorReady_ = false;
    bool busy_ = false;
    bool pendingChoiceActive_ = false;
    QString pendingCustomerId_;
    int pendingGroupId_ = 0;
    QString pendingPreviousL_;
    QString pendingPreviousR_;
    bool pendingHadPreviousL_ = false;
    bool pendingHadPreviousR_ = false;
    QString pendingTemplateL_;
    QString pendingTemplateR_;
    bool pendingNewOkL_ = false;
    bool pendingNewOkR_ = false;
    QString pendingSummaryL_;
    QString pendingSummaryR_;
};

#endif
