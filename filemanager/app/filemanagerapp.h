#ifndef FILEMANAGERAPP_H
#define FILEMANAGERAPP_H

#include <QObject>

class FileManagerWindow;
class AppController;

class FileManagerApp : public QObject
{
    Q_OBJECT
public:
    explicit FileManagerApp(QObject *parent = 0);
    ~FileManagerApp();

    void initApp();
    void initView();
    void initController();
    void initLogger();
    void initCommandline();
    void initGtk();
    void initConnect();

signals:

public slots:
    void show();

private:
    FileManagerWindow* m_fileManagerWindow=NULL;
    AppController* m_appController = NULL;
};

#endif // FILEMANAGERAPP_H
