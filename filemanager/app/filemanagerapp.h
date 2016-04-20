#ifndef FILEMANAGERAPP_H
#define FILEMANAGERAPP_H

#include <QObject>

class WindowManager;
class FileManagerWindow;
class DFileManagerWindow;
class AppController;
class DialogManager;
class DUrl;
class QTimer;

QT_BEGIN_NAMESPACE
class QLocalServer;
QT_END_NAMESPACE

class FileManagerApp : public QObject
{
    Q_OBJECT
public:
    explicit FileManagerApp(QObject *parent = 0);
    ~FileManagerApp();

    void initGtk();
    void initApp();
    void initView();
    void initController();
    void initManager();
    void initTranslation();
    void initConnect();

    AppController *getAppController() const;

public slots:
    void show(const DUrl& url);
    void lazyRunCacheTask();

private:
    WindowManager* m_windowManager = NULL;
    FileManagerWindow* m_fileManagerWindow = NULL;
    DFileManagerWindow * m_dFileMangerWindow = NULL;
    AppController* m_appController = NULL;
    DialogManager* m_dialogManager = NULL;
    QTimer* m_taskTimer = NULL;
};

#endif // FILEMANAGERAPP_H
