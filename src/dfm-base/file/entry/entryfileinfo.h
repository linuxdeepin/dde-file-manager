// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENTRYFILEINFO_H
#define ENTRYFILEINFO_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/abstractentryfileentity.h>

namespace dfmbase {

class EntryFileInfoPrivate;
class EntryFileInfo : public FileInfo
{
public:
    explicit EntryFileInfo(const QUrl &url);
    virtual ~EntryFileInfo() override;
    AbstractEntryFileEntity *entity() const;

    AbstractEntryFileEntity::EntryOrder order() const;

    bool renamable() const;
    QString displayName() const;
    quint64 sizeTotal() const;
    quint64 sizeUsage() const;
    quint64 sizeFree() const;
    bool showTotalSize() const;
    bool showUsedSize() const;
    bool showProgress() const;
    QUrl targetUrl() const;
    bool isAccessable() const;
    QString description() const;
    QVariant extraProperty(const QString &property) const;
    void setExtraProperty(const QString &property, const QVariant &value);

    // AbstractFileInfo interface
    virtual bool exists() const override;
    virtual QString nameOf(const FileNameInfoType type) const override;
    virtual QString displayOf(const DisplayInfoType type) const override;
    virtual QString pathOf(const FilePathInfoType type) const override;
    virtual QIcon fileIcon() override;
    virtual void refresh() override;
    virtual QVariantHash extraProperties() const override;

private:
    QSharedPointer<EntryFileInfoPrivate> d = nullptr;
};

}

typedef QSharedPointer<DFMBASE_NAMESPACE::EntryFileInfo> DFMEntryFileInfoPointer;
Q_DECLARE_METATYPE(DFMEntryFileInfoPointer)

#endif   // ENTRYFILEINFO_H
