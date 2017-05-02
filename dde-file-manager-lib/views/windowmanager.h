#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>
#include <QHash>

class DFileManagerWindow;
class DUrl;
class DFMEvent;
class FMStateManager;
class FMState;
class QTimer;


class WindowManager : public QObject
{
    Q_OBJECT

public:
    static WindowManager *instance();

    ~WindowManager();
    void initConnect();

    void loadWindowState(DFileManagerWindow* window);
    void saveWindowState(DFileManagerWindow* window);
    inline QHash<const QWidget*, quint64> getWindows(){return m_windows;}

    static DUrl getUrlByWindowId(quint64 windowId);

    static bool tabAddableByWinId(const quint64& winId);

signals:
    void start(const QString &src);

public slots:
    void showNewWindow(const DUrl &url, const bool &isNewWindow=false);
    static quint64 getWindowId(const QWidget *window);
    static QWidget* getWindowById(quint64 winId);
    void reastartAppProcess();

private slots:
    void onWindowClosed();
    void onLastActivedWindowClosed(quint64 winId);
    void quit();

protected:
    explicit WindowManager(QObject *parent = 0);

private:
    static QHash<const QWidget*, quint64> m_windows;
    static int m_count;

     FMStateManager *m_fmStateManager = NULL;

     QTimer* m_restartProcessTimer = NULL;
     bool m_isAppInDaemonStatus = true;
};

#endif // WINDOWMANAGER_H
