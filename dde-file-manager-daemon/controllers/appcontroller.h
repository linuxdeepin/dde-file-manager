#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include "../partman/partitionmanager.h"


class FileOperation;
class UserShareManager;
class UsbFormatter;

class AppController : public QObject
{
    Q_OBJECT
public:
    explicit AppController(QObject *parent = 0);
    ~AppController();

    void initControllers();
    void initConnect();

signals:

public slots:

private:
    FileOperation* m_fileOperationController = NULL;
    UserShareManager* m_userShareManager = NULL;
    UsbFormatter* m_usbFormatter = NULL;
};

#endif // APPCONTROLLER_H
