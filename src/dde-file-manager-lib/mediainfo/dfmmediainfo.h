/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#pragma once
#include "dfmglobal.h"
#include <QObject>

DFM_BEGIN_NAMESPACE
class DFMMediaInfoPrivate;
class DFMMediaInfo : public QObject
{
    Q_OBJECT
public:
    enum MeidiaType{
        General,
        Video,
        Audio,
        Text,
        Other,
        Image,
        Menu,
        Max,
    };

    DFMMediaInfo(const QString &filename, QObject *parent=nullptr);
    ~DFMMediaInfo();
    QString Value(const QString &key, MeidiaType meidiaType = General);
    void startReadInfo();
    void stopReadInfo();
signals:
    void Finished();

private:
    QScopedPointer<DFMMediaInfoPrivate> d_private;
    Q_DECLARE_PRIVATE_D(d_private, DFMMediaInfo)
};

DFM_END_NAMESPACE
