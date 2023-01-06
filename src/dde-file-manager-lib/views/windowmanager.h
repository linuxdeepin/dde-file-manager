// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QObject>
#include <QHash>
#include <QMutex>

class DFileManagerWindow;
class DUrl;
class DFMEvent;
class QTimer;


class WindowManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enableAutoQuit READ enableAutoQuit WRITE setEnableAutoQuit)

public:
    static WindowManager *instance();

    ~WindowManager();
    void initConnect();

    void loadWindowState(DFileManagerWindow* window);
    void saveWindowState(DFileManagerWindow* window);
    inline QHash<const QWidget*, quint64> getWindows(){return m_windows;}

    static DUrl getUrlByWindowId(quint64 windowId);

    static bool tabAddableByWinId(const quint64& winId);

    bool enableAutoQuit() const;

    void clearWindowActions();

signals:
    void start(const QString &src);

public slots:
    void showNewWindow(const DUrl &url, const bool &isNewWindow=false);
    static quint64 getWindowId(const QWidget *window);
    static QWidget* getWindowById(quint64 winId);
    void reastartAppProcess();

    void setEnableAutoQuit(bool enableAutoQuit);

private slots:
    void onWindowClosed();
    void onLastActivedWindowClosed(quint64 winId);
    void onShowNewWindow();
    void onRemoveNeedShowSmbUrl(const DUrl &url);

protected:
    explicit WindowManager(QObject *parent = nullptr);

private:
    static QHash<const QWidget*, quint64> m_windows;
    static int m_count;
    DFileManagerWindow *m_lastWindow = nullptr;
    QMultiHash<DUrl, DUrl> m_smbPointUrl;
    QMutex m_smbPointUrlMutex;

#ifdef AUTO_RESTART_DEAMON
    QTimer* m_restartProcessTimer = NULL;
    bool m_enableAutoQuit = false;
#endif
};

#endif // WINDOWMANAGER_H
