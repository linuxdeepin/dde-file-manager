#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <QtGlobal>

#include <DApplication>
#include <durl.h>

QT_BEGIN_NAMESPACE
class QLocalServer;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE;

class SingleApplication : public DApplication
{
    Q_OBJECT
public:
    explicit SingleApplication(int &argc, char **argv, int = ApplicationFlags);
    ~SingleApplication();
    void initConnect();

    static void initSources();
    static void newClientProcess(const QString& key, const DUrlList& urllist);
    static QString userServerName(const QString& key);
    static QString userId();

public slots:
    bool setSingleInstance(const QString& key);
    void handleConnection();
    void readData();
    void closeServer();

private:
    static QString getUserID();

    static QString UserID;
    QLocalServer* m_localServer;
};

#endif // SINGLEAPPLICATION_H
