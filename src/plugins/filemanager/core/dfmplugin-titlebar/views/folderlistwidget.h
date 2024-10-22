// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FOLDERLISTWIDGET_H
#define FOLDERLISTWIDGET_H

#include "dfmplugin_titlebar_global.h"

#include <DBlurEffectWidget>
#include <QScopedPointer>

namespace dfmplugin_titlebar {
class FolderListWidgetPrivate;
class FolderListWidget : public Dtk::Widget::DBlurEffectWidget
{
    Q_OBJECT
    QScopedPointer<FolderListWidgetPrivate> d;

public:
    explicit FolderListWidget(QWidget *parent = nullptr);
    ~FolderListWidget() override;

    void setFolderList(const QList<CrumbData> &datas);

Q_SIGNALS:
    void urlButtonActivated(const QUrl &url);
    void hidden();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void hideEvent(QHideEvent *event) override;
};

}   // namespace dfmplugin_titlebar

#endif   // FOLDERLISTWIDGET_H
