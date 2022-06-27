/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
