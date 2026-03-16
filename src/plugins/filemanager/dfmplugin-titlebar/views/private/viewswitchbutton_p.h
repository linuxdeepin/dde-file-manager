// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEWSWITCHBUTTON_P_H
#define VIEWSWITCHBUTTON_P_H

#include "dfmplugin_titlebar_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <DToolButton>
#include <DMenu>

namespace dfmplugin_titlebar {

class ViewSwitchButton;

class ViewSwitchButtonPrivate : public QObject
{
    Q_OBJECT
    friend class ViewSwitchButton;
    ViewSwitchButton *const q;

public:
    explicit ViewSwitchButtonPrivate(ViewSwitchButton *parent);
    ~ViewSwitchButtonPrivate() override;

public slots:
    void updateCheckedState();

private:
    void initializeUi();
    void setupMenu();

    bool hoverFlag { false };
    QString currentIconName;           // 当前显示的视图模式图标名
    DTK_WIDGET_NAMESPACE::DMenu *menu { nullptr };   // 内部拥有的弹出菜单
    DFMBASE_NAMESPACE::Global::ViewMode currentMode { DFMBASE_NAMESPACE::Global::ViewMode::kIconMode };   // 当前视图模式，用于菜单勾选状态同步
    QAction *iconAction { nullptr };
    QAction *listAction { nullptr };
    QAction *treeAction { nullptr };
};

}   // namespace dfmplugin_titlebar

#endif   // VIEWSWITCHBUTTON_P_H
