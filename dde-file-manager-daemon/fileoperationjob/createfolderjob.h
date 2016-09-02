#ifndef CREATEFOLDERJOB_H
#define CREATEFOLDERJOB_H

#include <QObject>
#include "basejob.h"

class CreateFolderJobAdaptor;

class CreateFolderJob : public BaseJob
{
    Q_OBJECT
public:
    explicit CreateFolderJob(const QString& fabspath, QObject *parent = 0);
    ~CreateFolderJob();

    static QString BaseObjectPath;
    static QString PolicyKitActionId;
    static int JobId;

signals:
    void Done(const QString& message);

public slots:
    void Execute();

private:
    QString m_fabspath;
    int m_jobId = 0;

    CreateFolderJobAdaptor* m_adaptor;
};

#endif // CREATEFOLDERJOB_H
