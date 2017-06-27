#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QtGlobal>

#include <DApplication>
#include <durl.h>

QT_BEGIN_NAMESPACE
class QLocalServer;
class QLocalSocket;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE

class SingleApplication : public DApplication
{
    Q_OBJECT
public:
    explicit SingleApplication(int &argc, char **argv, int = ApplicationFlags);
    ~SingleApplication();
    void initConnect();

    static void initSources();
    static QLocalSocket *newClientProcess(const QString& key, const QByteArray &message);
    static QString userServerName(const QString& key);
    static QString userId();

    bool loadTranslator(QList<QLocale> localeFallback = QList<QLocale>() << QLocale::system());

public slots:
    bool setSingleInstance(const QString& key);
    void handleConnection();
    void readData();
    void closeServer();

private:
    void handleQuitAction() Q_DECL_OVERRIDE;

    static QString getUserID();

    static QString UserID;
    QLocalServer* m_localServer;
};

#endif // SINGLEAPPLICATION_H
