/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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

#ifndef ARRANGEDDESKTOPFILEINFO_H
#define ARRANGEDDESKTOPFILEINFO_H

#include "dabstractfileinfo.h"

class ArrangedDesktopFileInfoPrivate;
class ArrangedDesktopFileInfo : public DAbstractFileInfo
{
public:
    ArrangedDesktopFileInfo(const DUrl &url);

    virtual bool isDir() const override;
    QString fileName() const override;

    bool exists() const override;

private:
    Q_DECLARE_PRIVATE(ArrangedDesktopFileInfo)
};

#endif // ARRANGEDDESKTOPFILEINFO_H
