// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PANEL_H
#define PANEL_H

#include <dfm-gui/containment.h>

#include <QObject>

class QQuickWindow;

DFMGUI_BEGIN_NAMESPACE

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

    void installTitleBar(Applet *titlebar);
    void installSideBar(Applet *sidebar);
    void installWorkSpace(Applet *workspace);
    void installDetailView(Applet *detailview);

    Applet *titleBar() const;
    Applet *sideBar() const;
    Applet *workSpace() const;
    Applet *detailView() const;

    virtual void loadState();
    virtual void saveState();

    Q_SLOT void setSidebarState(int width, bool manualHide, bool autoHide);
    Q_SIGNAL void sidebarStateChanged(int width, bool manualHide, bool autoHide);

Q_SIGNALS:
    void aboutToOpen();
    void aboutToClose();

    void titleBarInstallFinished();
    void sideBarInstallFinished();
    void workspaceInstallFinished();
    void detailViewInstallFinished();

private:
    Q_DECLARE_PRIVATE_D(dptr, Panel);
    Q_DISABLE_COPY(Panel);
};

DFMGUI_END_NAMESPACE

#endif   // PANEL_H
