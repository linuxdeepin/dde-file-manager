// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PANEL_H
#define PANEL_H

#include <dfm-gui/containment.h>
#include <dfm-gui/quickutils.h>

#include <QObject>

class QQuickWindow;

DFMGUI_BEGIN_NAMESPACE

class AppletItem;

// TODO:此部分是否和 Applet 分离，而非继承关系
class PanelPrivate;
class Panel : public Containment
{
    Q_OBJECT

    Q_PROPERTY(bool showSidebar READ showSidebar WRITE setShowSidebar NOTIFY showSidebarChanged FINAL)

public:
    explicit Panel(QObject *parent = nullptr);

    QQuickWindow *window() const;
    quint64 windId() const;

    bool showSidebar() const;
    void setShowSidebar(bool b);
    Q_SIGNAL void showSidebarChanged(bool show);

    // TODO: 是否沿用之前框架的注册方式
    Q_INVOKABLE void installTitleBar(AppletItem *titlebar);
    Q_INVOKABLE void installSideBar(AppletItem *sidebar);
    Q_INVOKABLE void installWorkSpace(AppletItem *workspace);
    Q_INVOKABLE void installDetailView(AppletItem *detailview);

    AppletItem *titleBar() const;
    AppletItem *sideBar() const;
    AppletItem *workSpace() const;
    AppletItem *detailView() const;

    virtual void loadState();
    virtual void saveState();

    Q_SLOT void setSidebarState(int width, bool manualHide, bool autoHide);
    Q_SIGNAL void sidebarStateChanged(int width, bool manualHide, bool autoHide);

Q_SIGNALS:
    void aboutToOpen();
    void aboutToClose();

    void shortcutTriggered(QuickUtils::ShortcutType type, const QKeyCombination &combin);

    void titleBarInstallFinished();
    void sideBarInstallFinished();
    void workspaceInstallFinished();
    void detailViewInstallFinished();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    Q_DECLARE_PRIVATE_D(dptr, Panel)
    Q_DISABLE_COPY(Panel)
};

DFMGUI_END_NAMESPACE

#endif   // PANEL_H
