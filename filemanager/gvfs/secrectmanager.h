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

    static const SecretSchema * SMBSecretSchema();

    static void on_password_cleared (GObject *source,
                                GAsyncResult *result,
                                gpointer unused);

    void clearPasswordByLoginObj(const QJsonObject& obj);
signals:

public slots:

private:
    QJsonObject m_smbLoginObj;
};

#endif // SECRECTMANAGER_H
