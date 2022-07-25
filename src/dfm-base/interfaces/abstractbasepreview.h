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
#ifndef FILEPREVIEWINTERFACE_H
#define FILEPREVIEWINTERFACE_H

#include "dfm_base_global.h"

#include <QObject>
#include <QUrl>

namespace dfmbase {
class AbstractBasePreview : public QObject
{
    Q_OBJECT
public:
    explicit AbstractBasePreview(QObject *parent = nullptr);

    virtual void initialize(QWidget *window, QWidget *statusBar);
    virtual bool setFileUrl(const QUrl &url) = 0;
    virtual QUrl fileUrl() const = 0;

    virtual QWidget *contentWidget() const = 0;
    virtual QWidget *statusBarWidget() const;
    virtual Qt::Alignment statusBarWidgetAlignment() const;

    virtual QString title() const;
    virtual bool showStatusBarSeparator() const;

    virtual void play(); /*play media if file is music or video*/
    virtual void pause(); /*pause playing if file is music or video*/
    virtual void stop(); /*stop playing if file is music or video*/

signals:
    void titleChanged();
};
}
#endif   // FILEPREVIEWINTERFACE_H
