#ifndef SECRETMANAGER_H
#define SECRETMANAGER_H

#include <QObject>
#include <QJsonObject>
#undef signals
extern "C" {
    #include <libsecret/secret.h>
}
#define signals public

class SecretManager : public QObject
{
    Q_OBJECT
public:
    explicit SecretManager(QObject *parent = 0);
    ~SecretManager();

    void initData();
    void initConnect();

    static const SecretSchema * SMBSecretSchema();
    static const SecretSchema * FTPSecretSchema();

    static void on_password_cleared (GObject *source,
                                GAsyncResult *result,
                                gpointer unused);

    void clearPasswordByLoginObj(const QJsonObject& obj);
    QJsonObject getLoginData(const QString& id);
    QJsonObject getLoginDatas();
    static QString cachePath();
signals:

public slots:
    void cacheSambaLoginData(const QJsonObject& obj);
    void loadCache();
    void saveCache();


private:
    QJsonObject m_smbLoginObjs;
};

#endif // SECRETMANAGER_H
