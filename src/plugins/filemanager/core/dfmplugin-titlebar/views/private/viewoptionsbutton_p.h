// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIEWOPTIONSBUTTON_P_H
#define VIEWOPTIONSBUTTON_P_H

#include "dfmplugin_titlebar_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <DPushButton>
#include <QScopedPointer>

class QUrl;
class QEvent;
class QListView;
class QMenu;

namespace dfmplugin_titlebar {
class ViewOptionsWidget;
class ViewOptionsButton;
class ViewOptionsButtonPrivate : public QObject
{
    Q_OBJECT
    friend class ViewOptionsButton;
    ViewOptionsButton *q { nullptr };

    DFMBASE_NAMESPACE::Global::ViewMode viewMode { DFMBASE_NAMESPACE::Global::ViewMode::kNoneMode };
    QUrl fileUrl;
    bool hoverFlag { false };   // 鼠标是否悬停在按钮上
    ViewOptionsWidget *viewOptionsWidget { nullptr };

public:
    explicit ViewOptionsButtonPrivate(ViewOptionsButton *qq);
    virtual ~ViewOptionsButtonPrivate();

private:
    void initConnect();
    bool popupVisible() const;
};

}   // namespace dfmplugin_titlebar

#endif   // VIEWOPTIONSBUTTON_P_H
