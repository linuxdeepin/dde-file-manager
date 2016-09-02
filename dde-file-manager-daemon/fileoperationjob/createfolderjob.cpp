#include "createfolderjob.h"
#include "dbusadaptor/createfolderjob_adaptor.h"

QString CreateFolderJob::BaseObjectPath = "/com/deepin/filemanager/daemon/CreateFolderJob";
QString CreateFolderJob::PolicyKitActionId = "com.deepin.filemanager.daemon.NewCreateFolderJob";
int CreateFolderJob::JobId = 0;

CreateFolderJob::CreateFolderJob(const QString &fabspath, QObject *parent) :
    BaseJob(parent),
    m_fabspath(fabspath)
{
    JobId += 1;
    m_jobId = JobId;
    setObjectPath(QString("%1%2").arg(BaseObjectPath, QString::number(m_jobId)));
    m_adaptor = new CreateFolderJobAdaptor(this);
}

CreateFolderJob::~CreateFolderJob()
{

}

void CreateFolderJob::Execute()
{
    qDebug() << "CreateFolderJob execute";
    qDebug() << PolicyKitActionId;
    bool isAuthenticationSucceeded = checkAuthorization(PolicyKitActionId, getClientPid());
    if (isAuthenticationSucceeded){
        qDebug() << "CreateFolderJob executing";
    }
    deleteLater();
}

