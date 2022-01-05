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
#ifndef ENTRYFILEENTITY_H
#define ENTRYFILEENTITY_H

#include "dfm-base/dfm_base_global.h"
#include "dfm-framework/service/qtclassfactory.h"
#include "file/entry/entryfileinfo.h"

#include <QUrl>
#include <QIcon>
#include <QAction>
#include <QDebug>

DFMBASE_BEGIN_NAMESPACE

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
    virtual void onOpen() = 0;
    virtual EntryFileInfo::EntryOrder order() const = 0;

    virtual QMenu *createMenu() { return nullptr; }
    virtual bool removable() { return false; }
    virtual void refresh() {}
    virtual long sizeTotal() const { return 0; }
    virtual long sizeUsage() const { return 0; }
    virtual QString fileSystem() const { return {}; }
    virtual QString description() const { return {}; }
    virtual QUrl targetUrl() const { return {}; }
    virtual bool isEncrypted() const { return false; }
    virtual bool isUnlocked() const { return false; }
    virtual QString clearDeviceId() const { return ""; }

protected:
    QUrl entryUrl {};
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

DFMBASE_END_NAMESPACE

#endif   // ENTRYFILEENTITY_H
