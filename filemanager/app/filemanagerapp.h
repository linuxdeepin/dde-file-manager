#ifndef FILEMANAGERAPP_H
#define FILEMANAGERAPP_H

#include <QObject>

class WindowManager;
#define DEEPIN_UI_ENABLE 1

class FileManagerWindow;
class DFileManagerWindow;
class AppController;
class DTaskDialog;
class QTimer;

class FileManagerApp : public QObject
{
    Q_OBJECT
public:
    explicit FileManagerApp(QObject *parent = 0);
    ~FileManagerApp();

    void initLogger();
    void initGtk();
    void initApp();
    void initView();
    void initTaskDialog();
    void initController();
    void initManager();
    void initCommandline();
    void initTranslation();
    void initConnect();

    AppController *getAppController() const;

public slots:
    void show();
    void handleDataUpdated();

private:
    WindowManager* m_windowManager = NULL;
    FileManagerWindow* m_fileManagerWindow = NULL;
    DFileManagerWindow * m_dFileMangerWindow = NULL;
    AppController* m_appController = NULL;
    DTaskDialog* m_taskDialog = NULL;
    QTimer* m_timer;
};

#endif // FILEMANAGERAPP_H
