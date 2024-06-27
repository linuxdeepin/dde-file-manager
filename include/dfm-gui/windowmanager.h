// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <dfm-gui/dfm_gui_global.h>
#include <dfm-gui/panel.h>

#include <QObject>

class QQuickWindow;
class QQmlEngine;

DFMGUI_BEGIN_NAMESPACE

class Applet;
class WindowManagerPrivate;
class WindowManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(WindowManager)

public:
    using Handle = QPointer<Panel>;

    static WindowManager *instance();
    void initialize();
    QSharedPointer<QQmlEngine> engine() const;

    Handle createWindow(const QUrl &url, const QString &pluginName,
                        const QString &quickId, const QVariantMap &var = {});
    void showWindow(const Handle &handle) const;
    bool activeExistsWindowByUrl(const QUrl &url) const;

    quint64 findWindowId(const QObject *itemObject) const;
    quint64 findWindowIdFromApplet(Applet *applet) const;
    Handle findWindowByUrl(const QUrl &url) const;
    Handle findWindowById(quint64 windId) const;
    QList<quint64> windowIdList() const;

    void resetPreviousActivedWindowId();
    quint64 previousActivedWindowId() const;
    bool containsCurrentUrl(const QUrl &url, const QQuickWindow *window = nullptr) const;

    QString lastError() const;

Q_SIGNALS:
    void windowCreated(quint64 windId);
    void windowOpened(quint64 windId);
    void windowClosed(quint64 windId);
    void lastWindowClosed(quint64 windId);
    void currentUrlChanged(quint64 windId, const QUrl &url);

private:
    explicit WindowManager();
    ~WindowManager() override;

private:
    QScopedPointer<WindowManagerPrivate> dptr;

    Q_DECLARE_PRIVATE_D(dptr, WindowManager)
};

using WindowHandle = WindowManager::Handle;

DFMGUI_END_NAMESPACE

#define FMQuickWindowIns (DFMGUI_NAMESPACE::WindowManager::instance())

#endif   // WINDOWMANAGER_H
