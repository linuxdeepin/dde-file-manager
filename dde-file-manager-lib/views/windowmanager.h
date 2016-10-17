#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>
#include <QHash>

class DFileManagerWindow;
class DUrl;
class DFMEvent;
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

    void loadWindowState(DFileManagerWindow* window);
    void saveWindowState(DFileManagerWindow* window);
    inline QHash<const QWidget*, int> getWindows(){return m_windows;}

    static DUrl getUrlByWindowId(int windowId);

    static bool tabAddableByWinId(const int& winId);

signals:
    void start(const QString &src);

public slots:
    void showNewWindow(const DUrl &url, bool isAlwaysOpen=false);
    static int getWindowId(const QWidget *window);
    static QWidget* getWindowById(int winId);

private slots:
    void onWindowClosed();
    void onLastActivedWindowClosed(int winId);
    void quit();

private:
     static QHash<const QWidget*, int> m_windows;
     static int m_count;

     FMStateManager *m_fmStateManager = NULL;
};

#endif // WINDOWMANAGER_H
