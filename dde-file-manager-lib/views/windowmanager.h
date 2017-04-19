#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>
#include <QHash>

#include "dfmabstracteventhandler.h"

class DFileManagerWindow;
class DUrl;
class DFMEvent;
class FMStateManager;
class FMState;

DFM_USE_NAMESPACE

class WindowManager : public QObject, public DFMAbstractEventHandler
{
    Q_OBJECT

public:
    explicit WindowManager(QObject *parent = 0);
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

private slots:
    void onWindowClosed();
    void onLastActivedWindowClosed(quint64 winId);
    void quit();

private:
    bool fmEvent(const QSharedPointer<DFMEvent> &event, QVariant *resultData) Q_DECL_OVERRIDE;

    static QHash<const QWidget*, quint64> m_windows;
    static int m_count;

    FMStateManager *m_fmStateManager = NULL;
};

#endif // WINDOWMANAGER_H
