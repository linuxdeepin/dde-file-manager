/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
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
#ifndef ENTRYFILEINFO_H
#define ENTRYFILEINFO_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-base/mimetype/mimedatabase.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

class QMenu;

DFMBASE_BEGIN_NAMESPACE

namespace DeviceId {
extern const char *const kBlockDeviceIdPrefix;
}   // namespace DeviceId

namespace SuffixInfo {
extern const char *const kUserDir;
extern const char *const kProtocol;
extern const char *const kBlock;
extern const char *const kStashedRemote;
}   // namespace SuffixInfo

class EntryFileInfoPrivate;
class EntryFileInfo : public AbstractFileInfo
{
public:
    enum EntryOrder {
        kOrderUserDir,

        kOrderSysDiskRoot,
        kOrderSysDiskData,
        kOrderSysDisks,
        kOrderRemovableDisks,
        kOrderOptical,

        kOrderSmb,
        kOrderStashedSmb,
        kOrderFtp,
        kOrderMTP,
        kOrderGPhoto2,
        kOrderFiles,

        kOrderApps,
    };

    explicit EntryFileInfo(const QUrl &url);
    virtual ~EntryFileInfo() override;

    EntryOrder order() const;

    bool renamable() const;
    QString displayName() const;
    long sizeTotal() const;
    long sizeUsage() const;
    long sizeFree() const;
    QString fileSystem() const;
    bool showTotalSize() const;
    bool showUsedSize() const;
    bool showProgress() const;
    bool removable() const;
    QUrl targetUrl() const;
    QMenu *createMenu() const;

    // AbstractFileInfo interface
    virtual bool exists() const override;
    virtual QString filePath() const override;
    virtual QString baseName() const override;
    virtual QString suffix() const override;
    virtual QString path() const override;
    virtual QIcon fileIcon() const override;
    virtual void refresh() override;

private:
    QSharedPointer<EntryFileInfoPrivate> d;
};

DFMBASE_END_NAMESPACE

typedef QSharedPointer<DFMBASE_NAMESPACE::EntryFileInfo> DFMEntryFileInfoPointer;

#endif   // ENTRYFILEINFO_H
