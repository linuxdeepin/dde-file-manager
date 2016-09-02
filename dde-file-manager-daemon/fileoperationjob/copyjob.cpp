#include "copyjob.h"
#include "dbusadaptor/copyjob_adaptor.h"

QString CopyJob::BaseObjectPath = "/com/deepin/filemanager/daemon/CreateCopyJob";
QString CopyJob::PolicyKitActionId = "com.deepin.filemanager.daemon.NewCopyJob";
int CopyJob::JobId = 0;

CopyJob::CopyJob(const QStringList &filelist, const QString &targetDir, QObject *parent) :
    BaseJob(parent),
    m_filelist(filelist),
    m_targetDir(targetDir)
{
    JobId += 1;
    m_jobId = JobId;
    setObjectPath(QString("%1%2").arg(BaseObjectPath, QString::number(m_jobId)));
    m_adaptor = new CopyJobAdaptor(this);
}

CopyJob::~CopyJob()
{

}

void CopyJob::Execute()
{
    qDebug() << "CreateFolderJob execute";
    qDebug() << PolicyKitActionId;
    bool isAuthenticationSucceeded = checkAuthorization(PolicyKitActionId, getClientPid());
    if (isAuthenticationSucceeded){
        qDebug() << "CreateFolderJob executing";
    }
    deleteLater();
}
