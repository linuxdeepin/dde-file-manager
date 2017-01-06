#ifndef FILEMANAGERAPP_H
#define FILEMANAGERAPP_H

#include <QObject>

class WindowManager;
class AppController;
class DUrl;
class QTimer;

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
    void initConnect();

    QString getFileJobConfigPath();

public slots:
    void show(const DUrl& url);

    void loadFileJobConfig();
    void showPropertyDialog(const QStringList paths);

protected:
    explicit FileManagerApp(QObject *parent = 0);

private:
    WindowManager* m_windowManager = NULL;
};

#endif // FILEMANAGERAPP_H
