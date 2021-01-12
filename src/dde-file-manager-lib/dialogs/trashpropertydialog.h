/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#ifndef TRASHPROPERTYDIALOG_H
#define TRASHPROPERTYDIALOG_H

#include "durl.h"
#include "propertydialog.h"
#include <QLabel>
#include <ddialog.h>

DWIDGET_USE_NAMESPACE

class TrashPropertyDialog : public DDialog
{
    Q_OBJECT
public:
    explicit TrashPropertyDialog(const DUrl& url, QWidget *parent = nullptr);
    ~TrashPropertyDialog();

    void initUI();
    void startComputerFolderSize(const DUrl& url);

public slots:
    void updateFolderSize(qint64 size);

private:
    DUrl m_url;
    QLabel* m_iconLabel;
    QLabel* m_nameLable;
    QLabel* m_countLabel;
    QLabel* m_sizeLabel;
};

#endif // TRASHPROPERTYDIALOG_H
