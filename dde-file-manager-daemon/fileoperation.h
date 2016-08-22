#ifndef FILEOPERATION_H
#define FILEOPERATION_H

#include <QObject>
#include <QDBusVariant>


class FileOperationAdaptor;

class FileOperation : public QObject
{
    Q_OBJECT

public:
    explicit FileOperation(const QString& servicePath, const QString& objectPath, QObject *parent = 0);
    ~FileOperation();

signals:

public slots:
    QDBusVariant NewRenameJob(const QString& oldFile, const QString& newFile);
    QString test(const QString &oldFile, const QString &newFile, QDBusObjectPath &result2, bool &result3);

private:
    QString m_servicePath;
    QString m_objectPath;
    FileOperationAdaptor* m_fileOperationAdaptor;
    int m_renameJobCount = 0;
    QString m_renameJobBasePath;
};

#endif // FILEOPERATION_H
