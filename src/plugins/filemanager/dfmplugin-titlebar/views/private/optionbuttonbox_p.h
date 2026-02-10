// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPTIONBUTTONBOX_P_H
#define OPTIONBUTTONBOX_P_H

#include "dfmplugin_titlebar_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <DToolButton>

#include <QPushButton>
#include <QHBoxLayout>
#include <QButtonGroup>

class QHBoxLayout;

namespace dfmplugin_titlebar {

using DFMBASE_NAMESPACE::Global::ViewMode;
class SortByButton;
class ViewOptionsButton;
class OptionButtonBox;
class OptionButtonBoxPrivate : public QObject
{
    Q_OBJECT
    friend class OptionButtonBox;

public:
    explicit OptionButtonBoxPrivate(OptionButtonBox *parent);
    void updateCompactButton();

public slots:
    void setViewMode(ViewMode mode);
    void onViewModeChanged(int mode);

private:
    void loadViewMode(const QUrl &url);
    void switchMode(ViewMode mode);

private:
    OptionButtonBox *const q;

    QButtonGroup *buttonGroup { nullptr };
    DTK_WIDGET_NAMESPACE::DToolButton *iconViewButton { nullptr };
    DTK_WIDGET_NAMESPACE::DToolButton *listViewButton { nullptr };
    DTK_WIDGET_NAMESPACE::DToolButton *treeViewButton { nullptr };
    SortByButton *sortByButton { nullptr };
    ViewOptionsButton *viewOptionsButton { nullptr };
    QHBoxLayout *hBoxLayout { nullptr };
    DTK_WIDGET_NAMESPACE::DToolButton *compactButton { nullptr };
    bool isCompactMode { false };

    ViewMode currentMode { ViewMode::kIconMode };
    QUrl currentUrl;

    bool iconViewEnabled { true };
    bool listViewEnabled { true };
    bool treeViewEnabled { true };
    bool sortByEnabled { true };
    bool viewOptionsEnabled { true };
};
}   // namespace dfmplugin_titlebar

#endif   // OPTIONBUTTONBOX_P_H
