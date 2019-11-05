/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *               2019 ~ 2019 Chris Xiong
 *
 * Author:     Chris Xiong<chirs241097@gmail.com>
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

#ifndef UDISKSUTILS_H
#define UDISKSUTILS_H

#include <QString>
#include <QScopedPointer>

class DBlockDevice;
class UDisksBlock
{
public:
    UDisksBlock(const QString &devnode);
    ~UDisksBlock();

    bool isReadOnly() const;
    QString displayName() const;
    QString iconName() const;
    QString fsType() const;
    qint64 sizeTotal() const;
    qint64 sizeUsed() const;

    DBlockDevice* operator ->();
private:
    QScopedPointer<DBlockDevice> blk;
};
#endif // UDISKSUTILS_H
