// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OptionButtonBox_H
#define OptionButtonBox_H

#include "dfmplugin_titlebar_global.h"

#include <QWidget>
#include <QToolButton>

namespace dfmplugin_titlebar {
class ActionButton : public QToolButton
{
    Q_OBJECT

public:
    explicit ActionButton(QWidget *parent = nullptr);
    void setAction(QAction *action);
    QAction *action() const;
};

class OptionButtonBoxPrivate;
class OptionButtonBox : public QWidget
{
    Q_OBJECT
    friend class OptionButtonBoxPrivate;
    OptionButtonBoxPrivate *const d;

public:
    explicit OptionButtonBox(QWidget *parent = nullptr);
    QToolButton *iconViewButton() const;
    QToolButton *listViewButton() const;
    QToolButton *detailButton() const;
    void setIconViewButton(QToolButton *iconViewButton);
    void setListViewButton(QToolButton *listViewButton);
    void setDetailButton(QToolButton *detailButton);

    void setViewMode(int mode);

public slots:
    void onUrlChanged(const QUrl &url);

private:
    void initializeUi();
    void initConnect();
};
}

#endif   // OptionButtonBox_H
