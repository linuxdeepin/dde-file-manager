#include "secrectmanager.h"
#include <QDebug>

SecrectManager::SecrectManager(QObject *parent) : QObject(parent)
{

}

SecrectManager::~SecrectManager()
{

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

