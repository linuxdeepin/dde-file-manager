#ifndef RENAMEJOB_H
#define RENAMEJOB_H

#include <QObject>
#include "basejob.h"


class RenameJobAdaptor;

class RenameJob : public BaseJob
{
    Q_OBJECT
public:
    explicit RenameJob(const QString&oldFile,
                       const QString &newFile,
                       QObject *parent = 0);
    ~RenameJob();

    static QString BaseObjectPath;
    static QString PolicyKitActionId;
    static int JobId;

signals:
    void Done(const QString& message);

public slots:
    void Execute();

private:
    QString m_oldFile;
    QString m_newFile;
    int m_jobId = 0;
    RenameJobAdaptor* m_adaptor;
};

#endif // RENAMEJOB_H
