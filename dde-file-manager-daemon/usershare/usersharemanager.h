#ifndef USERSHAREDAEMONMANAGER_H
#define USERSHAREDAEMONMANAGER_H

#include <QObject>

class UserShareAdaptor;

class UserShareManager : public QObject
{
    Q_OBJECT
public:
    explicit UserShareManager(QObject *parent = 0);
    ~UserShareManager();

    static QString ObjectPath;

signals:

public slots:
    bool addGroup(const QString &groupName);
    bool addUserToGroup(const QString &userName, const QString &groupName);
    bool setUserSharePassword(const QString &username, const QString &passward);
    bool restartSambaService();
private:
    UserShareAdaptor* m_userShareAdaptor = NULL;
};

#endif // USERSHAREDAEMONMANAGER_H
