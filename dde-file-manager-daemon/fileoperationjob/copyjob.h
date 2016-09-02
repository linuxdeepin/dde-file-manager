#ifndef COPYJOB_H
#define COPYJOB_H

#include <QObject>
#include "basejob.h"

class CopyJobAdaptor;

class CopyJob : public BaseJob
{
    Q_OBJECT
public:
    explicit CopyJob(const QStringList &filelist, const QString &targetDir, QObject *parent = 0);
    ~CopyJob();

    static QString BaseObjectPath;
    static QString PolicyKitActionId;
    static int JobId;

signals:
    void Done(const QString& message);

public slots:
    void Execute();

private:
    QStringList m_filelist;
    QString m_targetDir;
    int m_jobId = 0;

    CopyJobAdaptor* m_adaptor;
};

#endif // COPYJOB_H
