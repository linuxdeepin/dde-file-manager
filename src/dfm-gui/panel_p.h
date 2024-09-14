// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PANEL_P_H
#define PANEL_P_H

#include <dfm-gui/panel.h>
#include <dfm-gui/appletitem.h>
#include <dfm-gui/shortcutmap.h>

#include "containment_p.h"

DFMGUI_BEGIN_NAMESPACE

class PanelPrivate : public ContainmentPrivate
{
    Q_DECLARE_PUBLIC(Panel)

public:
    explicit PanelPrivate(Panel *q);

    void setRootObject(QObject *item) override;

    void loadWindowState();
    void saveWindowState() const;

    void loadSidebarState();
    void saveSidebarState() const;

    bool handleKeyPressed(QKeyEvent *keyEvent);

    static QSharedPointer<ShortcutMap> windowShortcut();

    QPointer<QQuickWindow> window;
    AppletItem *sidebar { nullptr };
    AppletItem *titlebar { nullptr };
    AppletItem *workspace { nullptr };
    AppletItem *detailview { nullptr };

    bool showSidebar { true };
    int leftWidth { 200 };
    bool autoHideSidebar { false };
    bool manualHideSidebar { false };
    static constexpr int kDefaultLeftWidth { 200 };

    static ShortcutMapPtr shortcutPtr;
};

DFMGUI_END_NAMESPACE

#endif   // PANEL_P_H
