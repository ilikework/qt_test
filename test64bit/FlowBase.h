#ifndef FLOWBASE_H
#define FLOWBASE_H

#include <QObject>
#include <QJsonObject>

class FlowBase : public QObject {
    Q_OBJECT
public:
    explicit FlowBase(QObject* parent=nullptr) : QObject(parent) {}
    using QObject::QObject;
    virtual void start() = 0;

signals:
    void finished(bool ok, const QString& msg);

protected:
    static bool isOkResult(const QJsonObject& resp) {
        const QString r = resp.value("result").toString().toUpper();
        return (r == "OK");
    }

    bool aborted_ = false;

};


#endif // FLOWBASE_H
