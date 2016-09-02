#ifndef DELETEJOB_H
#define DELETEJOB_H

#include <QObject>
#include "basejob.h"

class DeleteJobAdaptor;

class DeleteJob : public BaseJob
{
    Q_OBJECT
public:
    explicit DeleteJob(const QStringList& filelist, QObject *parent = 0);
    ~DeleteJob();

    static QString BaseObjectPath;
    static QString PolicyKitActionId;
    static int JobId;

signals:
    void Done(const QString& message);

public slots:
    void Execute();

private:
    QStringList m_filelist;
    int m_jobId = 0;

    DeleteJobAdaptor* m_adaptor;

};

#endif // DELETEJOB_H
