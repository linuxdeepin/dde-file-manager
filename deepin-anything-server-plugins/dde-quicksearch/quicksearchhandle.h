/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     shihua <tangtong@deepin.com>
 *
 * Maintainer: shihua <tangtong@deepin.com>
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
#ifndef QUICKSEARCHHANDLE_H
#define QUICKSEARCHHANDLE_H

#include <dasinterface.h>


using namespace DAS_NAMESPACE;

class QuickSearchHandle : public DASInterface
{
    Q_OBJECT
public:
    explicit QuickSearchHandle(QObject *const parent = nullptr)
        : DASInterface{ parent } {}
    virtual ~QuickSearchHandle() = default;

    QuickSearchHandle(const QuickSearchHandle &other) = delete;
    QuickSearchHandle &operator=(const QuickSearchHandle &other) = delete;


    virtual void onFileCreate(const QByteArrayList &files) override;
    virtual void onFileDelete(const QByteArrayList &files) override;
    virtual void onFileRename(const QList<QPair<QByteArray, QByteArray>> &files) override;
};



#endif // QUICKSEARCHHANDLE_H
