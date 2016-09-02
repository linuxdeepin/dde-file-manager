#ifndef FILEOPERATION_H
#define FILEOPERATION_H

#include <QObject>
#include <QtDBus>
#include "dbusservice/dbustype/dbusinforet.h"


class FileOperationAdaptor;

class FileOperation : public QObject ,protected QDBusContext
{
    Q_OBJECT

public:
    explicit FileOperation(const QString& servicePath, QObject *parent = 0);
    ~FileOperation();

    static QString ObjectPath;

signals:

public slots:
    DBusInfoRet NewCreateFolderJob(const QString &fabspath);
    DBusInfoRet NewCreateTemplateFileJob(const QString &templateFile, const QString &targetDir);
    DBusInfoRet NewCopyJob(const QStringList &filelist, const QString &targetDir);
    DBusInfoRet NewMoveJob(const QStringList &filelist, const QString &targetDir);
    DBusInfoRet NewRenameJob(const QString &oldFile, const QString &newFile);
    DBusInfoRet NewDeleteJob(const QStringList &filelist);
    QString test(const QString &oldFile, const QString &newFile, QDBusObjectPath &result2, bool &result3);

private:
    QString m_servicePath;
    FileOperationAdaptor* m_fileOperationAdaptor;
};

typedef QMap<QString, QString> StringMap;

Q_DECLARE_METATYPE(StringMap)

#endif // FILEOPERATION_H
