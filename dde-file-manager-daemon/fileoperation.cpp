#include "fileoperation.h"
#include "fileoperation_adaptor.h"
#include "renamejob_adaptor.h"
#include "renamejob.h"
#include <QDBusConnection>
#include <QDBusVariant>
#include <QDebug>

FileOperation::FileOperation(const QString &servicePath, const QString &objectPath, QObject *parent) :
    QObject(parent)
{
    m_servicePath = servicePath;
    m_objectPath = objectPath;
    QDBusConnection connection = QDBusConnection::systemBus();
    connection.registerObject(objectPath, this);
    m_fileOperationAdaptor = new FileOperationAdaptor(this);
    m_renameJobBasePath = "/com/deepin/filemanager/daemon/RenameJob";
}

FileOperation::~FileOperation()
{

}

QDBusVariant FileOperation::NewRenameJob(const QString &oldFile, const QString &newFile)
{
    qDebug() << oldFile << newFile;
    m_renameJobCount += 1;
    QString objectPath = QString("%1%2").arg(m_renameJobBasePath, QString::number(m_renameJobCount));

    QMap<QString, QVariant> result;
    result.insert("service", m_servicePath);
    result.insert("objectPath", objectPath);

    RenameJob* job = new RenameJob;
    QDBusConnection connection = QDBusConnection::systemBus();
    qDebug() << connection.registerObject(objectPath, job);
    RenameJobAdaptor* jobAdaptor = new RenameJobAdaptor(job);


    QDBusVariant dbusvariant;
    dbusvariant.setVariant(QVariant(result));
    qDebug() << result;
    return dbusvariant;
}

QString FileOperation::test(const QString &oldFile, const QString &newFile, QDBusObjectPath &result2, bool &result3)
{
    result2.setPath(m_renameJobBasePath);
    result3 = true;

    qDebug() << result2.path() << result3;

    return "1111111111111";
}
