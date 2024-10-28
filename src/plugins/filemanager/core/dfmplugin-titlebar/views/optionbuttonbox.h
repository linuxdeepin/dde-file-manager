// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OptionButtonBox_H
#define OptionButtonBox_H

#include "dfmplugin_titlebar_global.h"

#include <DToolButton>

#include <QWidget>

namespace dfmplugin_titlebar {
class ActionButton : public QToolButton
{
    Q_OBJECT

public:
    explicit ActionButton(QWidget *parent = nullptr);
    void setAction(QAction *action);
    QAction *action() const;
};

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

public slots:
    void onUrlChanged(const QUrl &url);

private:
    void initializeUi();
    void initConnect();
    void initUiForSizeMode();
};
}

#endif   // OptionButtonBox_H
