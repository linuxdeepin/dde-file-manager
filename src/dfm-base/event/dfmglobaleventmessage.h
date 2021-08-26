/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
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
#ifndef DFMGLOBALEVENTMESSAGE_H
#define DFMGLOBALEVENTMESSAGE_H

#include <QObject>

class DFMGlobalEventMessage : public QObject
{
    Q_OBJECT

public:
    explicit DFMGlobalEventMessage(QObject *parent = nullptr);

    static DFMGlobalEventMessage* globalInstance();

Q_SIGNALS:
    void eventError(const QString &title, const QString &info);
    void eventInfor(const QString &title, const QString &info);
};

#define eventMessage DFMGlobalEventMessage::globalInstance();

#endif // DFMGLOBALEVENTMESSAGE_H
