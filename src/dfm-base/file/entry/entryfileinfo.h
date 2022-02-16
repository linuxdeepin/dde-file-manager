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
        kOrderStashedSmb = kOrderSmb,   // same order with mounted smb
        kOrderFtp,
        kOrderMTP,
        kOrderGPhoto2,
        kOrderFiles,

        kOrderApps,

        kOrderCustom,
    };

    explicit EntryFileInfo(const QUrl &url);
    virtual ~EntryFileInfo() override;

    EntryOrder order() const;

    bool renamable() const;
    QString displayName() const;
    qint64 sizeTotal() const;
    qint64 sizeUsage() const;
    qint64 sizeFree() const;
    bool showTotalSize() const;
    bool showUsedSize() const;
    bool showProgress() const;
    QUrl targetUrl() const;
    QMenu *createMenu() const;
    bool isAccessable() const;
    QString description() const;
    QVariant extraProperty(const QString &property) const;
    void setExtraProperty(const QString &property, const QVariant &value);

    // AbstractFileInfo interface
    virtual bool exists() const override;
    virtual QString filePath() const override;
    virtual QString baseName() const override;
    virtual QString suffix() const override;
    virtual QString path() const override;
    virtual QIcon fileIcon() const override;
    virtual void refresh() override;
    virtual QVariantHash extraProperties() const override;

private:
    EntryFileInfoPrivate *d = nullptr;
};

DFMBASE_END_NAMESPACE

typedef QSharedPointer<DFMBASE_NAMESPACE::EntryFileInfo> DFMEntryFileInfoPointer;
Q_DECLARE_METATYPE(DFMEntryFileInfoPointer)

#endif   // ENTRYFILEINFO_H
