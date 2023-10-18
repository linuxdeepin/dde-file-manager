// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENTRYFILEINFO_P_H
#define ENTRYFILEINFO_P_H

#include <dfm-base/interfaces/private/fileinfo_p.h>
#include <dfm-base/interfaces/abstractentryfileentity.h>
#include <dfm-base/file/entry/entryfileinfo.h>

namespace dfmbase {
class EntryFileInfoPrivate : public FileInfoPrivate
{
    friend class EntryFileInfo;
    QScopedPointer<AbstractEntryFileEntity> entity { nullptr };

public:
    explicit EntryFileInfoPrivate(EntryFileInfo *qq);
    void init();
    QString suffix() const;
    virtual ~EntryFileInfoPrivate() override;
};

}
#endif   // ENTRYFILEINFO_P_H
