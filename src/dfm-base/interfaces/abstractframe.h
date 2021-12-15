/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#ifndef ABSTRACTFRAME_H
#define ABSTRACTFRAME_H

#include "dfm-base/dfm_base_global.h"

#include <QFrame>
#include <QUrl>

DFMBASE_BEGIN_NAMESPACE

class AbstractFrame : public QFrame
{
    Q_OBJECT
public:
    explicit AbstractFrame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~AbstractFrame();

    virtual void setCurrentUrl(const QUrl &url) = 0;
};

DFMBASE_END_NAMESPACE

#endif   // ABSTRACTFRAME_H
