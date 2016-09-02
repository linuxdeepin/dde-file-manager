#include "deletejob.h"
#include "dbusadaptor/deletejob_adaptor.h"

QString DeleteJob::BaseObjectPath = "/com/deepin/filemanager/daemon/DeleteJob";
QString DeleteJob::PolicyKitActionId = "com.deepin.filemanager.daemon.NewDeleteJob";
int DeleteJob::JobId = 0;


DeleteJob::DeleteJob(const QStringList &filelist, QObject *parent) :
    BaseJob(parent),
    m_filelist(filelist)
{
    JobId += 1;
    m_jobId = JobId;
    setObjectPath(QString("%1%2").arg(BaseObjectPath, QString::number(m_jobId)));
    m_adaptor = new DeleteJobAdaptor(this);
}

DeleteJob::~DeleteJob()
{

}

void DeleteJob::Execute()
{
    qDebug() << "DeleteJob execute";
    qDebug() << PolicyKitActionId;
    bool isAuthenticationSucceeded = checkAuthorization(PolicyKitActionId, getClientPid());
    if (isAuthenticationSucceeded){
        qDebug() << "DeleteJob executing";
    }
    deleteLater();
}

