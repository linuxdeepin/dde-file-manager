// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENTRYFILEINFO_P_H
#define ENTRYFILEINFO_P_H

#include "interfaces/private/abstractfileinfo_p.h"
#include "file/entry/entities/abstractentryfileentity.h"
#include "file/entry/entryfileinfo.h"

namespace dfmbase {
class EntryFileInfoPrivate : public AbstractFileInfoPrivate
{
    friend class EntryFileInfo;
    QScopedPointer<AbstractEntryFileEntity> entity { nullptr };

public:
    explicit EntryFileInfoPrivate(const QUrl &url, EntryFileInfo *qq);
    void init();
    QString suffix() const;
    virtual ~EntryFileInfoPrivate() override;
};

}
#endif   // ENTRYFILEINFO_P_H
