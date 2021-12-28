/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#ifndef AddressBar_H
#define AddressBar_H

#include "dfmplugin_titlebar_global.h"

#include <QLineEdit>

DPTITLEBAR_BEGIN_NAMESPACE

class AddressBarPrivate;
class AddressBar : public QLineEdit
{
    Q_OBJECT
    friend class AddressBarPrivate;
    AddressBarPrivate *const d;

public:
    explicit AddressBar(QWidget *parent = nullptr);
    bool completerViewVisible();

Q_SIGNALS:
    void editingFinishedUrl(const QUrl &url);
    void editingFinishedSearch(const QString &url);

public
    Q_SLOT : void startSpinner();
    void stopSpinner();
};

DPTITLEBAR_END_NAMESPACE

#endif   //AddressBar_H
