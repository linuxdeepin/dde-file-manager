#ifndef SECRECTMANAGER_H
#define SECRECTMANAGER_H

#include <QObject>
#include <QJsonObject>
#undef signals
extern "C" {
    #include <libsecret/secret.h>
}
#define signals public

class SecrectManager : public QObject
{
    Q_OBJECT
public:
    explicit SecrectManager(QObject *parent = 0);
    ~SecrectManager();

    void initData();
    void initConnect();

    static const SecretSchema * SMBSecretSchema();

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

#endif // SECRECTMANAGER_H
