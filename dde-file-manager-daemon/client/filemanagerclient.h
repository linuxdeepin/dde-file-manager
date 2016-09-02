#ifndef FILEMANAGERCLIENT_H
#define FILEMANAGERCLIENT_H

#include <QObject>

#include "dbusservice/dbusinterface/fileoperation_interface.h"

class FileManagerClient : public QObject
{
    Q_OBJECT
public:
    explicit FileManagerClient(QObject *parent = 0);
    ~FileManagerClient();

signals:

public slots:
    void testNewNameJob(const QString &oldFile, const QString &newFile);

private:
    FileOperationInterface* m_fileOperationInterface = NULL;

};

#endif // FILEMANAGERCLIENT_H
