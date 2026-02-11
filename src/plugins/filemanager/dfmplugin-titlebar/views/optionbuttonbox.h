// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OptionButtonBox_H
#define OptionButtonBox_H

#include "dfmplugin_titlebar_global.h"

#include <DToolButton>

#include <QWidget>

namespace dfmplugin_titlebar {

class ViewOptionsButton;
class OptionButtonBoxPrivate;
class OptionButtonBox : public QWidget
{
    Q_OBJECT
    friend class OptionButtonBoxPrivate;
    OptionButtonBoxPrivate *const d;

public:
    explicit OptionButtonBox(QWidget *parent = nullptr);
    DTK_WIDGET_NAMESPACE::DToolButton *iconViewButton() const;
    DTK_WIDGET_NAMESPACE::DToolButton *listViewButton() const;
    ViewOptionsButton *viewOptionsButton() const;
    void setIconViewButton(DTK_WIDGET_NAMESPACE::DToolButton *iconViewButton);
    void setListViewButton(DTK_WIDGET_NAMESPACE::DToolButton *listViewButton);
    void setViewOptionsButton(ViewOptionsButton *button);

    void setViewMode(int mode);
    DFMBASE_NAMESPACE::Global::ViewMode viewMode() const;
    void updateOptionButtonBox(int parentWidth);

public slots:
    void onUrlChanged(const QUrl &url);

private:
    void initializeUi();
    void initConnect();
    void initUiForSizeMode();
    void updateFixedWidth();

    void switchToCompactMode();
    void switchToNormalMode();

    static constexpr int kCompactModeThreshold = 600;
};
}

#endif   // OptionButtonBox_H
