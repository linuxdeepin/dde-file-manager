// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
