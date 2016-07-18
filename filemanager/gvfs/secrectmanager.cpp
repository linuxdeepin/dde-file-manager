#include "secrectmanager.h"
#include "../app/global.h"
#include "../app/filesignalmanager.h"
#include "../shutil/standardpath.h"
#include <QDebug>

SecrectManager::SecrectManager(QObject *parent) : QObject(parent)
{
    initData();
    initConnect();
}

SecrectManager::~SecrectManager()
{

}

void SecrectManager::initData()
{
    loadCache();
}

void SecrectManager::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::requsetCacheLoginData, this, &SecrectManager::cacheSambaLoginData);
}

const SecretSchema *SecrectManager::SMBSecretSchema()
{
    static const SecretSchema the_schema = {
        "org.gnome.keyring.NetworkPassword", SECRET_SCHEMA_DONT_MATCH_NAME,
        {
            {"user", SECRET_SCHEMA_ATTRIBUTE_STRING },
            {"domain", SECRET_SCHEMA_ATTRIBUTE_STRING },
            {"server", SECRET_SCHEMA_ATTRIBUTE_STRING },
            {"protocol", SECRET_SCHEMA_ATTRIBUTE_STRING }
        }
    };

    return &the_schema;
}

void SecrectManager::on_password_cleared(GObject *source, GAsyncResult *result, gpointer unused)
{
    qDebug() << "on_password_cleared";
    GError *error = NULL;

    gboolean removed = secret_password_clear_finish (result, &error);

    qDebug() << removed;

    if (error != NULL) {
        /* ... handle the failure here */
        g_error_free (error);

    } else {
        /* removed will be TRUE if a password was removed */
        qDebug() << "password was removed";
    }
}

void SecrectManager::clearPasswordByLoginObj(const QJsonObject &obj)
{
    secret_password_clear(SMBSecretSchema(), NULL, on_password_cleared, NULL,
                          "user", obj.value("user").toString().toStdString().c_str(),
                          "domain", obj.value("domain").toString().toStdString().c_str(),
                          "server", obj.value("server").toString().toStdString().c_str(),
                          "protocol", obj.value("protocol").toString().toStdString().c_str(),
                          NULL);
}

QJsonObject SecrectManager::getLoginData(const QString &id)
{
    return m_smbLoginObjs.value(id).toObject();
}

QJsonObject SecrectManager::getLoginDatas()
{
    return m_smbLoginObjs;
}

QString SecrectManager::cachePath()
{
    return QString("%1/samba.json").arg(StandardPath::getCachePath());
}

void SecrectManager::cacheSambaLoginData(const QJsonObject &obj)
{
    QJsonValue v(obj);
    m_smbLoginObjs.insert(obj.value("id").toString(), v);
    saveCache();
}

void SecrectManager::loadCache()
{
    QFile file(cachePath());
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Couldn't write samba file!";
        return;
    }
    QByteArray data = file.readAll();
    QJsonDocument jsonDoc(QJsonDocument::fromJson(data));
    m_smbLoginObjs = jsonDoc.object();
    file.close();
    qDebug() << m_smbLoginObjs;
}

void SecrectManager::saveCache()
{
    QFile file(cachePath());
    if (!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "Couldn't write samba file!";
        return;
    }
    QJsonDocument jsonDoc(m_smbLoginObjs);
    file.write(jsonDoc.toJson());
    file.close();
}

