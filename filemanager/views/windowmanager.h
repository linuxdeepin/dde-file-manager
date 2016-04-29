#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include "dfilemanagerwindow.h"

#include <QObject>
#include <QHash>

class DFileManagerWindow;
class DUrl;
class FMEvent;
class FMStateManager;
class FMState;
class D;

class WindowManager : public QObject
{
    Q_OBJECT

public:
    explicit WindowManager(QObject *parent = 0);
    ~WindowManager();
    void initConnect();

    void loadWindowState(DMainWindow* window);
    void saveWindowState(DMainWindow* window);

signals:
    void start(const QString &src);

public slots:
    void showNewWindow(const DUrl &url, bool isAlwaysOpen=false);
    static int getWindowId(const QWidget *window);
    static QWidget* getWindowById(int winId);

private slots:
    void onWindowClosed();
    void onLastActivedWindowClosed(int winId);
    inline QHash<const QWidget*, int> getWindows(){return m_windows;}

private:
     static QHash<const QWidget*, int> m_windows;
     static int m_count;

     FMStateManager *m_fmStateManager = NULL;
};

#endif // WINDOWMANAGER_H
