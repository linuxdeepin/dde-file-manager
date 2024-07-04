// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENTRYFILEENTITY_H
#define ENTRYFILEENTITY_H

#include <dfm-base/dfm_base_global.h>

#include <QUrl>
#include <QIcon>
#include <QDebug>
#include <QObject>
#include <QVariant>

namespace dfmbase {

class AbstractEntryFileEntity : public QObject
{
    Q_OBJECT
public:
    enum EntryOrder {
        kOrderUserDir,

        kOrderSysDiskRoot,
        kOrderSysDiskData,
        kOrderSysDisks,
        kOrderRemovableDisks,
        kOrderOptical,

        kOrderSmb,
        kOrderFtp,
        kOrderMTP,
        kOrderGPhoto2,
        kOrderFiles,

        kOrderApps,

        kOrderCustom,
    };

    explicit AbstractEntryFileEntity(const QUrl &url);
    virtual ~AbstractEntryFileEntity();

    virtual QString displayName() const = 0;
    virtual QIcon icon() const = 0;
    virtual bool exists() const = 0;

    virtual bool showProgress() const = 0;
    virtual bool showTotalSize() const = 0;
    virtual bool showUsageSize() const = 0;
    virtual EntryOrder order() const = 0;

    virtual inline void refresh() { }
    virtual inline quint64 sizeTotal() const { return 0; }
    virtual inline quint64 sizeUsage() const { return 0; }
    virtual inline QString description() const { return {}; }
    virtual inline QUrl targetUrl() const { return {}; }
    virtual inline bool isAccessable() const { return true; }
    virtual inline bool renamable() const { return false; }
    virtual inline QVariantHash extraProperties() const { return datas; }
    virtual inline void setExtraProperty(const QString &key, const QVariant &val) { datas[key] = val; }

protected:
    QUrl entryUrl {};
    mutable QVariantHash datas;
};

class EntryEntityFactor
{
    using EntityCreator = std::function<AbstractEntryFileEntity *(const QUrl &url)>;
    static QHash<QString, EntityCreator> creators;

public:
    template<class T>
    static inline bool registCreator(const QString &suffix)
    {
        if (creators.contains(suffix)) {
            qCWarning(logDFMBase) << "register failed: already exists" << suffix;
            return false;
        }
        creators.insert(suffix, [](const QUrl &url) {
            return new T(url);
        });
        return true;
    }

    static inline AbstractEntryFileEntity *create(const QString &suffix, const QUrl &url)
    {
        return creators.contains(suffix) ? creators.value(suffix)(url) : nullptr;
    }
};

}   // namespace dfmbase

#endif   // ENTRYFILEENTITY_H
