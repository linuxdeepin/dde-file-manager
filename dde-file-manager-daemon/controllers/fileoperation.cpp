#include "fileoperation.h"
#include "dbusservice/dbusadaptor/fileoperation_adaptor.h"

#include "fileoperationjob/copyjob.h"
#include "fileoperationjob/movejob.h"
#include "fileoperationjob/createfolderjob.h"
#include "fileoperationjob/createtemplatefilejob.h"
#include "fileoperationjob/renamejob.h"
#include "fileoperationjob/deletejob.h"

#include <QDBusConnection>
#include <QDBusVariant>
#include <QDebug>

QString FileOperation::ObjectPath = "/com/deepin/filemanager/daemon/Operations";

FileOperation::FileOperation(const QString &servicePath, QObject *parent) :
    QObject(parent)
{
    DBusInfoRet::registerMetaType();
    m_servicePath = servicePath;
    QDBusConnection::systemBus().registerObject(ObjectPath, this);
    m_fileOperationAdaptor = new FileOperationAdaptor(this);
}

FileOperation::~FileOperation()
{

}

DBusInfoRet FileOperation::NewCreateFolderJob(const QString &fabspath)
{
    DBusInfoRet result;
    qDebug() << fabspath;
    CreateFolderJob* job = new CreateFolderJob(fabspath);
    if (job->registerObject()){
        result.ServicePath = m_servicePath;
        result.InterfacePath = QDBusObjectPath(job->objectPath());
    }
    qDebug() << result;
    return result;
}

DBusInfoRet FileOperation::NewCreateTemplateFileJob(const QString &templateFile, const QString &targetDir)
{
    DBusInfoRet result;
    qDebug() << templateFile << targetDir;
    CreateTemplateFileJob* job = new CreateTemplateFileJob(templateFile, targetDir);
    if (job->registerObject()){
        result.ServicePath = m_servicePath;
        result.InterfacePath = QDBusObjectPath(job->objectPath());
    }
    qDebug() << result;
    return result;
}

DBusInfoRet FileOperation::NewCopyJob(const QStringList &filelist, const QString &targetDir)
{
    DBusInfoRet result;
    qDebug() << filelist << targetDir;
    CopyJob* job = new CopyJob(filelist, targetDir);
    if (job->registerObject()){
        result.ServicePath = m_servicePath;
        result.InterfacePath = QDBusObjectPath(job->objectPath());
    }
    qDebug() << result;
    return result;
}

DBusInfoRet FileOperation::NewMoveJob(const QStringList &filelist, const QString &targetDir)
{
    DBusInfoRet result;
    qDebug() << filelist << targetDir;
    MoveJob* job = new MoveJob(filelist, targetDir);
    if (job->registerObject()){
        result.ServicePath = m_servicePath;
        result.InterfacePath = QDBusObjectPath(job->objectPath());
    }
    qDebug() << result;
    return result;
}

DBusInfoRet FileOperation::NewRenameJob(const QString &oldFile, const QString &newFile)
{
    DBusInfoRet result;
    qDebug() << oldFile << newFile << QDBusConnection::systemBus().name();
    RenameJob* job = new RenameJob(oldFile, newFile);
    if (job->registerObject()){
        result.ServicePath = m_servicePath;
        result.InterfacePath = QDBusObjectPath(job->objectPath());
    }
    qDebug() << result;
    return result;
}

DBusInfoRet FileOperation::NewDeleteJob(const QStringList &filelist)
{
    DBusInfoRet result;
    qDebug() << filelist;
    DeleteJob* job = new DeleteJob(filelist);
    if (job->registerObject()){
        result.ServicePath = m_servicePath;
        result.InterfacePath = QDBusObjectPath(job->objectPath());
    }
    qDebug() << result;
    return result;
}

QString FileOperation::test(const QString &oldFile, const QString &newFile, QDBusObjectPath &result2, bool &result3)
{
    Q_UNUSED(oldFile)
    Q_UNUSED(newFile)

    result2.setPath("m_renameJobBasePath");
    result3 = true;

    qDebug() << result2.path() << result3;

    return "1111111111111";
}
