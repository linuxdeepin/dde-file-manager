#include "createtemplatefilejob.h"
#include "dbusadaptor/createtemplatefilejob_adaptor.h"

QString CreateTemplateFileJob::BaseObjectPath = "/com/deepin/filemanager/daemon/CreateTemplateFileJob";
QString CreateTemplateFileJob::PolicyKitActionId = "com.deepin.filemanager.daemon.NewCreateTempleFileJob";
int CreateTemplateFileJob::JobId = 0;

CreateTemplateFileJob::CreateTemplateFileJob(const QString &templateFile, const QString &targetDir, QObject *parent) :
    BaseJob(parent),
    m_templateFile(templateFile),
    m_targetDir(targetDir)
{
    JobId += 1;
    m_jobId = JobId;
    setObjectPath(QString("%1%2").arg(BaseObjectPath, QString::number(m_jobId)));
    m_adaptor = new CreateTemplateFileAdaptor(this);
}

CreateTemplateFileJob::~CreateTemplateFileJob()
{

}

void CreateTemplateFileJob::Execute()
{
    qDebug() << "CreateTemplateFileJob execute";
    qDebug() << PolicyKitActionId;
    bool isAuthenticationSucceeded = checkAuthorization(PolicyKitActionId, getClientPid());
    if (isAuthenticationSucceeded){
        qDebug() << "CreateTemplateFileJob executing";
    }
    deleteLater();
}

