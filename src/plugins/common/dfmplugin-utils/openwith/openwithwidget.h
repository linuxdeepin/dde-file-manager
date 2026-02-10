// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPENWITHWIDGET_H
#define OPENWITHWIDGET_H
#include "dfmplugin_utils_global.h"

#include <DArrowLineDrawer>

#include <QListWidget>
#include <QButtonGroup>
#include <QUrl>

namespace dfmplugin_utils {
class OpenWithWidget : public DTK_WIDGET_NAMESPACE::DArrowLineDrawer
{
    Q_OBJECT
public:
    explicit OpenWithWidget(QWidget *parent = nullptr);

public:
    void selectFileUrl(const QUrl &url);

private:
    void initUI();

private slots:
    void openWithBtnChecked(QAbstractButton *btn);

    void slotExpandChange(bool state);

private:
    QListWidget *openWithListWidget { nullptr };
    QButtonGroup *openWithBtnGroup { nullptr };
    QUrl currentFileUrl;
};
}
#endif   // OPENWITHWIDGET_H
