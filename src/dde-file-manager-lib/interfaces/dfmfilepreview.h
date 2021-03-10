/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#ifndef DFMFILEPREVIEWDIALOG_H
#define DFMFILEPREVIEWDIALOG_H

#include <QObject>

#include "dfmglobal.h"

class DUrl;

DFM_BEGIN_NAMESPACE

class DFMFilePreview : public QObject
{
    Q_OBJECT
public:
    explicit DFMFilePreview(QObject *parent = 0);

    virtual void initialize(QWidget *window, QWidget *statusBar);
    virtual bool setFileUrl(const DUrl &url) = 0;
    virtual DUrl fileUrl() const = 0;

    virtual QWidget *contentWidget() const = 0;
    virtual QWidget *statusBarWidget() const;
    virtual Qt::Alignment statusBarWidgetAlignment() const;

    virtual QString title() const;
    virtual bool showStatusBarSeparator() const;

    /**
     * @brief DoneCurrent 释放surface
     */
    virtual void DoneCurrent();

    virtual void play(); /*play media if file is music or video*/
    virtual void pause(); /*pause playing if file is music or video*/
    virtual void stop(); /*stop playing if file is music or video*/

    virtual void copyFile() const;

signals:
    void titleChanged();
};

DFM_END_NAMESPACE

#endif // DFMFILEPREVIEWDIALOG_H
