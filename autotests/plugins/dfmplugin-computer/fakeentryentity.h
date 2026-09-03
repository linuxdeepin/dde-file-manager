// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FAKEENTRYENTITY_H
#define FAKEENTRYENTITY_H

#include <dfm-base/interfaces/abstractentryfileentity.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <QIcon>
#include <QUrl>
#include <QVariant>

DFMBASE_USE_NAMESPACE
using namespace GlobalServerDefines;

namespace dfmplugin_computer {

class FakeEntryEntity : public AbstractEntryFileEntity
{
    Q_OBJECT
public:
    explicit FakeEntryEntity(const QUrl &url)
        : AbstractEntryFileEntity(url)
    {
    }

    void setExists(bool exists) { setExtraProperty("__test_exists", exists); }
    void setTargetUrl(const QUrl &url) { setExtraProperty(DeviceProperty::kMountPoint, url.path()); }
    void setDisplayName(const QString &name) { setExtraProperty(DeviceProperty::kDisplayName, name); }
    void setOrder(EntryOrder order) { setExtraProperty("__test_order", static_cast<int>(order)); }
    void setIsAccessable(bool access) { setExtraProperty("__test_access", access); }
    void setRenamable(bool rename) { setExtraProperty("__test_renamable", rename); }
    void setExtraProp(const QString &key, const QVariant &val) { setExtraProperty(key, val); }

    bool exists() const override { return propBool("__test_exists", false); }
    QString displayName() const override { return propString(DeviceProperty::kDisplayName, entryUrl.fileName()); }
    QIcon icon() const override { return QIcon::fromTheme("drive-harddisk"); }
    bool showProgress() const override { return propBool("__test_showProgress", false); }
    bool showTotalSize() const override { return propBool("__test_showTotalSize", false); }
    bool showUsageSize() const override { return propBool("__test_showUsageSize", false); }
    EntryOrder order() const override { return static_cast<EntryOrder>(propInt("__test_order", static_cast<int>(kOrderCustom))); }
    quint64 sizeTotal() const override { return propULongLong("__test_sizeTotal", 0); }
    quint64 sizeUsage() const override { return propULongLong("__test_sizeUsage", 0); }
    QString description() const override { return propString("__test_description", QString()); }
    QUrl targetUrl() const override
    {
        QString path = propString(DeviceProperty::kMountPoint, QString());
        if (path.isEmpty())
            return QUrl();
        return QUrl::fromLocalFile(path);
    }
    bool isAccessable() const override { return propBool("__test_access", false); }
    bool renamable() const override { return propBool("__test_renamable", false); }

private:
    QVariant prop(const QString &key, const QVariant &def) const
    {
        return datas.contains(key) ? datas.value(key) : def;
    }
    bool propBool(const QString &key, bool def) const { return prop(key, def).toBool(); }
    int propInt(const QString &key, int def) const { return prop(key, def).toInt(); }
    QString propString(const QString &key, const QString &def) const { return prop(key, def).toString(); }
    quint64 propULongLong(const QString &key, quint64 def) const { return prop(key, def).toULongLong(); }
};

}

#endif // FAKEENTRYENTITY_H
