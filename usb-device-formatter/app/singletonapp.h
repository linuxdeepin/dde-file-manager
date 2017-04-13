#ifndef SINGLETONAPP_H
#define SINGLETONAPP_H

#include <QObject>
#include <QLocalServer>

class SingletonApp : public QObject
{
    Q_OBJECT
public:
    void initConnections();
    QString getServerPathByKey(const QString& key);
    QString getUserId();
    QString getServerRootPath();
    static SingletonApp* instance();

signals:

public slots:
    bool setSingletonApplication(const QString& key);
    void readData();
    void handleConnection();

private:
    QLocalServer m_server;
    QString m_key;
    explicit SingletonApp(QObject *parent = 0);
};

#endif // SINGLETONAPP_H
