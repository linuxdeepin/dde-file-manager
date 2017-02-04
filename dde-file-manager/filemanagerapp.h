#ifndef FILEMANAGERAPP_H
#define FILEMANAGERAPP_H

#include <QObject>

class WindowManager;
class AppController;
class DUrl;
class QFileSystemWatcher;

QT_BEGIN_NAMESPACE
class QLocalServer;
QT_END_NAMESPACE

class FileManagerApp : public QObject
{
    Q_OBJECT

public:
    static FileManagerApp *instance();

    ~FileManagerApp();

    void initApp();
    void initView();
    void initManager();
    void initTranslation();
    void lazyRunInitServiceTask();
    void initSysPathWatcher();
    void initConnect();

    static void initService();

//    QString getFileJobConfigPath();

public slots:
    void show(const DUrl& url);
    void showPropertyDialog(const QStringList paths);

protected:
    explicit FileManagerApp(QObject *parent = 0);

private:
    WindowManager* m_windowManager = NULL;
    QFileSystemWatcher* m_sysPathWatcher;
};

#endif // FILEMANAGERAPP_H
