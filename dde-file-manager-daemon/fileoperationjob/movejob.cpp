#include "movejob.h"
#include "dbusadaptor/movejob_adaptor.h"

QString MoveJob::BaseObjectPath = "/com/deepin/filemanager/daemon/MoveJob";
QString MoveJob::PolicyKitActionId = "com.deepin.filemanager.daemon.NewMoveJob";
int MoveJob::JobId = 0;

MoveJob::MoveJob(const QStringList &filelist, const QString &targetDir, QObject *parent) :
    BaseJob(parent),
    m_filelist(filelist),
    m_targetDir(targetDir)
{
    JobId += 1;
    m_jobId = JobId;
    setObjectPath(QString("%1%2").arg(BaseObjectPath, QString::number(m_jobId)));
    m_adaptor = new MoveJobAdaptor(this);
}

MoveJob::~MoveJob()
{

}

void MoveJob::Execute()
{
    qDebug() << "MoveJob execute";
    qDebug() << PolicyKitActionId;
    bool isAuthenticationSucceeded = checkAuthorization(PolicyKitActionId, getClientPid());
    if (isAuthenticationSucceeded){
        qDebug() << "MoveJob executing";
    }
    deleteLater();
}

