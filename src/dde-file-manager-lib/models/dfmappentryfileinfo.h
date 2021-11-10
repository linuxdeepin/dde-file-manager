/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#ifndef DFMAPPENTRYFILEINFO_H
#define DFMAPPENTRYFILEINFO_H

#include "dfmrootfileinfo.h"
#include "shutil/desktopfile.h"

class DFMAppEntryFileInfoPrivate;
class DFMAppEntryFileInfo : public DFMRootFileInfo
{
public:
    explicit DFMAppEntryFileInfo(const DUrl &url);
    inline QString suffix() const override {
        return SUFFIX_APP_ENTRY;
    }
    QString fileDisplayName() const override;
    QString iconName() const override;
    bool exists() const override;
    FileType fileType() const override;
    QVector<MenuAction> menuActionList(MenuType type) const override;

    QString cmd() const;
    QString executableBin() const;

private:
    DesktopFile entryFile;
    QScopedPointer<DFMAppEntryFileInfoPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DFMAppEntryFileInfo)
};

#endif // DFMAPPENTRYFILEINFO_H
