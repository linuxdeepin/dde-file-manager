// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "config/organizerconfig.h"
#include "organizerconfig_p.h"
#include "organizer_defines.h"

#include <dfm-io/dfmio_utils.h>

#include <QStandardPaths>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

using namespace ddplugin_organizer;

namespace {
inline constexpr char kGroupGeneral[] { "" };   // "General" is default group in QSetting, so using empty string to access 'General' group.
inline constexpr char kKeyEnable[] { "Enable" };
inline constexpr char kKeyMode[] { "Mode" };
inline constexpr char kKeyVersion[] { "Version" };

inline constexpr char kGroupScreen[] { "Screen_Resolution" };

inline constexpr char kGroupCollectionNormalized[] { "Collection_Normalized" };
inline constexpr char kKeyClassification[] { "Classification" };

inline constexpr char kGroupCollectionCustomed[] { "Collection_Customed" };
inline constexpr char kGroupCollectionBase[] { "CollectionBase" };
inline constexpr char kKeyName[] { "Name" };
inline constexpr char kKeyKey[] { "Key" };
inline constexpr char kGroupItems[] { "Items" };

inline constexpr char kGroupCollectionStyle[] { "CollectionStyle" };
inline constexpr char kKeyScreen[] { "screen" };
inline constexpr char kKeyX[] { "X" };
inline constexpr char kKeyY[] { "Y" };
inline constexpr char kKeyWidth[] { "Width" };
inline constexpr char kKeyHeight[] { "Height" };
inline constexpr char kKeySizeMode[] { "SizeMode" };
inline constexpr char kKeyCustomGeo[] { "CustomGeometry" };

inline constexpr char kGroupClassifierType[] { "Classifier_Type" };
inline constexpr char kKeyEnabledItems[] { "EnabledItems" };

inline constexpr char kKeyLastStyleConfigId[] { "LastStyleConfigId" };

}   // namepace

OrganizerConfigPrivate::OrganizerConfigPrivate(OrganizerConfig *qq)
    : q(qq)
{
}

OrganizerConfigPrivate::~OrganizerConfigPrivate()
{
    delete settings;
    settings = nullptr;
}

QVariant OrganizerConfigPrivate::value(const QString &group, const QString &key, const QVariant &defaultVar)
{
    if (key.isEmpty())
        return defaultVar;

    QVariant ret;
    if (group.isEmpty()) {
        ret = settings->value(key, defaultVar);
    } else {
        settings->beginGroup(group);
        ret = settings->value(key, defaultVar);
        settings->endGroup();
    }

    return ret;
}

void OrganizerConfigPrivate::setValue(const QString &group, const QString &key, const QVariant &var)
{
    settings->beginGroup(group);
    settings->setValue(key, var);
    settings->endGroup();
}

OrganizerConfig::OrganizerConfig(QObject *parent)
    : QObject(parent), d(new OrganizerConfigPrivate(this))
{
    // 只能在主线程创建
    Q_ASSERT(qApp->thread() == thread());

    auto configPath = path();
    fmDebug() << "OrganizerConfig initializing with path:" << configPath;

    QFileInfo configFile(configPath);
    if (!configFile.exists()) {
        fmInfo() << "Config file does not exist, creating directory structure";
        if (!configFile.absoluteDir().mkpath(".")) {
            fmCritical() << "Failed to create config directory:" << configFile.absoluteDir().path();
        }
    }

    d->settings = new QSettings(configPath, QSettings::IniFormat);

    // delay sync
    d->syncTimer.setSingleShot(true);
    connect(
            &d->syncTimer, &QTimer::timeout, this, [this]() {
                d->settings->sync();
            },
            Qt::QueuedConnection);
}

bool OrganizerConfig::isEnable() const
{
    return d->value(kGroupGeneral, kKeyEnable, false).toBool();
}

void OrganizerConfig::setEnable(bool e)
{
    d->setValue(kGroupGeneral, kKeyEnable, e);
}

int OrganizerConfig::mode() const
{
    return d->value(kGroupGeneral, kKeyMode, OrganizerMode::kNormalized).toInt();
}

void OrganizerConfig::setMode(int m)
{
    d->setValue(kGroupGeneral, kKeyMode, m);
}

void OrganizerConfig::setVersion(const QString &v)
{
    d->setValue(kGroupGeneral, kKeyVersion, v);
}

QList<QSize> OrganizerConfig::surfaceSizes()
{
    QList<QSize> ret;
    d->settings->beginGroup(kGroupScreen);
    for (auto key : d->settings->allKeys()) {
        auto val = d->settings->value(key).toString();
        QStringList vals = val.split(":");
        if (vals.count() < 2) {
            fmWarning() << "Invalid screen resolution format for key" << key << ":" << val;
            continue;
        }
        QSize size(vals.at(0).toInt(), vals.at(1).toInt());
        ret.append(size);
        fmDebug() << "Loaded screen size for" << key << ":" << size;
    }

    d->settings->endGroup();
    return ret;
}

void OrganizerConfig::setScreenInfo(const QMap<QString, QString> info)
{
    d->settings->remove(kGroupScreen);
    d->settings->beginGroup(kGroupScreen);

    for (auto iter = info.cbegin(); iter != info.cend(); ++iter)
        d->settings->setValue(iter.key(), iter.value());
    d->settings->endGroup();
}

void OrganizerConfig::sync(int ms)
{
    if (ms < 1)
        d->settings->sync();
    else
        d->syncTimer.start(ms);
}

int OrganizerConfig::classification() const
{
    return d->value(kGroupCollectionNormalized, kKeyClassification, Classifier::kType).toInt();
}

void OrganizerConfig::setClassification(int cf)
{
    d->setValue(kGroupCollectionNormalized, kKeyClassification, cf);
}

QList<CollectionBaseDataPtr> OrganizerConfig::collectionBase(bool custom) const
{
    QStringList profileKeys;
    d->settings->beginGroup(custom ? kGroupCollectionCustomed : kGroupCollectionNormalized);
    d->settings->beginGroup(kGroupCollectionBase);
    profileKeys = d->settings->childGroups();
    d->settings->endGroup();
    d->settings->endGroup();

    QList<CollectionBaseDataPtr> ret;
    for (const QString &key : profileKeys)
        if (auto base = collectionBase(custom, key))
            ret.append(base);

    return ret;
}

CollectionBaseDataPtr OrganizerConfig::collectionBase(bool custom, const QString &key) const
{
    d->settings->beginGroup(custom ? kGroupCollectionCustomed : kGroupCollectionNormalized);
    d->settings->beginGroup(kGroupCollectionBase);
    d->settings->beginGroup(key);

    CollectionBaseDataPtr base(new CollectionBaseData);
    base->name = d->settings->value(kKeyName, "").toString();
    base->key = d->settings->value(kKeyKey, "").toString();

    {
        d->settings->beginGroup(kGroupItems);
        auto keys = d->settings->childKeys();
        // must be sorted by int value
        std::sort(keys.begin(), keys.end(), [](const QString &t1, const QString &t2) {
            return t1.toInt() < t2.toInt();
        });

        for (const QString &index : keys) {
            QUrl url = d->settings->value(index).toString();
            if (url.isValid())
                base->items.append(url);
        }

        d->settings->endGroup();
    }

    d->settings->endGroup();
    d->settings->endGroup();
    d->settings->endGroup();

    if (key != base->key || base->key.isEmpty() || base->name.isEmpty()) {
        fmWarning() << "Invalid collection base data - expected key:" << key
                    << "actual key:" << base->key << "name:" << base->name;
        base.clear();
    } else {
        fmDebug() << "Loaded collection base:" << base->name << "with" << base->items.size() << "items";
    }
    return base;
}

void OrganizerConfig::updateCollectionBase(bool custom, const CollectionBaseDataPtr &base)
{
    d->settings->beginGroup(custom ? kGroupCollectionCustomed : kGroupCollectionNormalized);
    d->settings->beginGroup(kGroupCollectionBase);
    // delete old datas
    d->settings->remove(base->key);

    d->settings->beginGroup(base->key);
    d->settings->setValue(kKeyName, base->name);
    d->settings->setValue(kKeyKey, base->key);

    {
        d->settings->beginGroup(kGroupItems);

        int index = 0;
        for (auto iter = base->items.begin(); iter != base->items.end();) {
            d->settings->setValue(QString::number(index), iter->toString());
            ++index;
            ++iter;
        }

        d->settings->endGroup();
    }

    d->settings->endGroup();
    d->settings->endGroup();
    d->settings->endGroup();
}

void OrganizerConfig::writeCollectionBase(bool custom, const QList<CollectionBaseDataPtr> &base)
{
    d->settings->beginGroup(custom ? kGroupCollectionCustomed : kGroupCollectionNormalized);
    // delete all old datas
    d->settings->remove(kGroupCollectionBase);
    d->settings->beginGroup(kGroupCollectionBase);

    for (auto iter = base.begin(); iter != base.end(); ++iter) {
        d->settings->beginGroup((*iter)->key);
        d->settings->setValue(kKeyName, (*iter)->name);
        d->settings->setValue(kKeyKey, (*iter)->key);

        {
            d->settings->beginGroup(kGroupItems);

            int index = 0;
            for (auto it = (*iter)->items.begin(); it != (*iter)->items.end();) {
                d->settings->setValue(QString::number(index), it->toString());
                ++index;
                ++it;
            }

            d->settings->endGroup();
        }

        d->settings->endGroup();
    }

    d->settings->endGroup();
    d->settings->endGroup();
}

CollectionStyle OrganizerConfig::collectionStyle(const QString &styleId, const QString &key) const
{
    d->settings->beginGroup(styleId.isEmpty() ? kGroupCollectionNormalized : styleId);
    d->settings->beginGroup(kGroupCollectionStyle);
    d->settings->beginGroup(key);
    CollectionStyle style;
    style.screenIndex = d->settings->value(kKeyScreen, -1).toInt();
    style.key = d->settings->value(kKeyKey, "").toString();

    {
        int x = d->settings->value(kKeyX, -1).toInt();
        int y = d->settings->value(kKeyY, -1).toInt();
        int w = d->settings->value(kKeyWidth, 0).toInt();
        int h = d->settings->value(kKeyHeight, 0).toInt();
        style.rect = QRect(x, y, w, h);
    }

    style.sizeMode = d->settings->value(kKeySizeMode).value<CollectionFrameSize>();
    style.customGeo = d->settings->value(kKeyCustomGeo).toBool();

    d->settings->endGroup();
    d->settings->endGroup();
    d->settings->endGroup();
    return style;
}

void OrganizerConfig::updateCollectionStyle(const QString &styleId, const CollectionStyle &style)
{
    d->settings->beginGroup(styleId.isEmpty() ? kGroupCollectionNormalized : styleId);
    d->settings->beginGroup(kGroupCollectionStyle);
    // delete old datas
    d->settings->remove(style.key);

    d->settings->beginGroup(style.key);

    d->settings->setValue(kKeyScreen, style.screenIndex);
    d->settings->setValue(kKeyKey, style.key);
    d->settings->setValue(kKeyX, style.rect.x());
    d->settings->setValue(kKeyY, style.rect.y());
    d->settings->setValue(kKeyWidth, style.rect.width());
    d->settings->setValue(kKeyHeight, style.rect.height());
    d->settings->setValue(kKeySizeMode, static_cast<int>(style.sizeMode));
    d->settings->setValue(kKeyCustomGeo, style.customGeo);

    d->settings->endGroup();
    d->settings->endGroup();
    d->settings->endGroup();
}

void OrganizerConfig::writeCollectionStyle(const QString &styleId, const QList<CollectionStyle> &styles)
{
    d->settings->beginGroup(styleId.isEmpty() ? kGroupCollectionNormalized : styleId);
    // delete all old datas
    d->settings->remove(kGroupCollectionStyle);
    d->settings->beginGroup(kGroupCollectionStyle);

    int validCount = 0;
    for (auto iter = styles.begin(); iter != styles.end(); ++iter) {
        if (iter->key.isEmpty())
            continue;

        d->settings->beginGroup(iter->key);

        d->settings->setValue(kKeyScreen, iter->screenIndex);
        d->settings->setValue(kKeyKey, iter->key);
        d->settings->setValue(kKeyX, iter->rect.x());
        d->settings->setValue(kKeyY, iter->rect.y());
        d->settings->setValue(kKeyWidth, iter->rect.width());
        d->settings->setValue(kKeyHeight, iter->rect.height());
        d->settings->setValue(kKeySizeMode, static_cast<int>(iter->sizeMode));
        d->settings->setValue(kKeyCustomGeo, iter->customGeo);

        d->settings->endGroup();
    }

    d->settings->endGroup();
    d->settings->endGroup();
}

OrganizerConfig::~OrganizerConfig()
{
    delete d;
    d = nullptr;
}

QString OrganizerConfig::path() const
{
    // user config path
    auto paths = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    Q_ASSERT(!paths.isEmpty());

    QString configPath = paths.first();
    configPath = DFMIO::DFMUtils::buildFilePath(configPath.toStdString().c_str(),
                                                QApplication::organizationName().toStdString().c_str(),
                                                "dde-desktop",
                                                "ddplugin-organizer.conf", nullptr);

    return configPath;
}

void OrganizerConfig::setLastStyleConfigId(const QString &id)
{
    d->settings->beginGroup(kGroupScreen);
    d->settings->setValue(kKeyLastStyleConfigId, id);
    d->settings->endGroup();
}

QString OrganizerConfig::lastStyleConfigId() const
{
    d->settings->beginGroup(kGroupScreen);
    auto id = d->settings->value(kKeyLastStyleConfigId, "").toString();
    d->settings->endGroup();
    return id;
}

bool OrganizerConfig::hasConfigId(const QString &configId) const
{
    return d->settings->childGroups().contains(configId);
}