#ifndef FILEMANAGERAPP_H
#define FILEMANAGERAPP_H

#include <QObject>

class WindowManager;
class FileManagerWindow;
class DFileManagerWindow;
class AppController;
class DialogManager;
class QLocalServer;

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
    void show();

private:
    WindowManager* m_windowManager = NULL;
    FileManagerWindow* m_fileManagerWindow = NULL;
    DFileManagerWindow * m_dFileMangerWindow = NULL;
    AppController* m_appController = NULL;
    DialogManager* m_dialogManager = NULL;
};

#endif // FILEMANAGERAPP_H
