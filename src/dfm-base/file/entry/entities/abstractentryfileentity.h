// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ENTRYFILEENTITY_H
#define ENTRYFILEENTITY_H

#include <dfm-base/dfm_base_global.h>
#include <dfm-base/file/entry/entryfileinfo.h>

#include <QUrl>
#include <QIcon>
#include <QDebug>

namespace dfmbase {

class AbstractEntryFileEntity : public QObject
{
    Q_OBJECT
public:
    explicit AbstractEntryFileEntity(const QUrl &url);
    virtual ~AbstractEntryFileEntity();

    virtual QString displayName() const = 0;
    virtual QIcon icon() const = 0;
    virtual bool exists() const = 0;

    virtual bool showProgress() const = 0;
    virtual bool showTotalSize() const = 0;
    virtual bool showUsageSize() const = 0;
    virtual EntryFileInfo::EntryOrder order() const = 0;

    virtual void refresh() { }
    virtual quint64 sizeTotal() const { return 0; }
    virtual quint64 sizeUsage() const { return 0; }
    virtual QString description() const { return {}; }
    virtual QUrl targetUrl() const { return {}; }
    virtual bool isAccessable() const { return true; }
    virtual bool renamable() const { return false; }
    virtual QVariantHash extraProperties() const { return datas; }
    virtual void setExtraProperty(const QString &key, const QVariant &val) { datas[key] = val; }

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
    static bool registCreator(const QString &suffix)
    {
        if (creators.contains(suffix)) {
            qDebug() << "register failed: already exists" << suffix;
            return false;
        }
        creators.insert(suffix, [](const QUrl &url) {
            return new T(url);
        });
        return true;
    }

    static AbstractEntryFileEntity *create(const QString &suffix, const QUrl &url)
    {
        return creators.contains(suffix) ? creators.value(suffix)(url) : nullptr;
    }
};

}

#endif   // ENTRYFILEENTITY_H
