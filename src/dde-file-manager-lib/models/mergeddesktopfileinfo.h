// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MERGEDDESKTOPFILEINFO_H
#define MERGEDDESKTOPFILEINFO_H

#include "dabstractfileinfo.h"

class MergedDesktopFileInfoPrivate;
class MergedDesktopFileInfo : public DAbstractFileInfo
{
public:
    MergedDesktopFileInfo(const DUrl &url, const DUrl &parentUrl);

    DUrl parentUrl() const override;
    QString iconName() const override;
    QString genericIconName() const override;

    DUrl mimeDataUrl() const override;

    bool canRedirectionFileUrl() const override;
    DUrl redirectedFileUrl() const override;

private:
    Q_DECLARE_PRIVATE(MergedDesktopFileInfo)
};

#endif // MERGEDDESKTOPFILEINFO_H
