#include "renamejob.h"
#include "dbusservice/dbusadaptor/renamejob_adaptor.h"
#include <QDBusConnection>
#include <QDebug>


QString RenameJob::BaseObjectPath = "/com/deepin/filemanager/daemon/RenameJob";
QString RenameJob::PolicyKitActionId = "com.deepin.filemanager.daemon.NewRenameJob";
int RenameJob::JobId = 0;

RenameJob::RenameJob(const QString &oldFile,
                     const QString &newFile,
                     QObject *parent) :

    BaseJob(parent),
    m_oldFile(oldFile),
    m_newFile(newFile)
{
    JobId += 1;
    m_jobId = JobId;
    setObjectPath(QString("%1%2").arg(BaseObjectPath, QString::number(m_jobId)));
    m_adaptor = new RenameJobAdaptor(this);
}

RenameJob::~RenameJob()
{

}

void RenameJob::Execute()
{
    qDebug() << "RenameJob execute";
    qDebug() << PolicyKitActionId;
    bool isAuthenticationSucceeded = checkAuthorization(PolicyKitActionId, getClientPid());
    if (isAuthenticationSucceeded){
        qDebug() << "RenameJob executing";
    }
    deleteLater();
}
