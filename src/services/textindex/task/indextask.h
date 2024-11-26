#ifndef INDEXTASK_H
#define INDEXTASK_H

#include "../service_textindex_global.h"

#include <QObject>
#include <QString>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class IndexTask : public QObject
{
    Q_OBJECT
public:
    enum Type {
        Create,
        Update
    };

    enum Status {
        NotStarted,
        Running,
        Finished,
        Failed
    };

    explicit IndexTask(Type type, const QString &path, QObject *parent = nullptr);

    void start();
    void stop();
    bool isRunning() const;
    QString taskPath() const;
    Type taskType() const;
    Status status() const;

Q_SIGNALS:
    void progressChanged(Type type, int count);
    void finished(Type type, bool success);

private:
    void doTask();

    Type m_type;
    QString m_path;
    Status m_status { NotStarted };
    bool m_running { false };
};

SERVICETEXTINDEX_END_NAMESPACE
#endif // INDEXTASK_H
