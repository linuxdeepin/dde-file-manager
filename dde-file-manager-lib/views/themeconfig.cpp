/*
 * Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
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
#include "themeconfig.h"
#include "dfilemanagerwindow.h"

#include <DThemeManager>

#include <QSettings>
#include <QPixmapCache>
#include <QGuiApplication>
#include <QIcon>
#include <QImageReader>

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

class _ThemeConfig : public ThemeConfig {
public:
    _ThemeConfig() {update(DThemeManager::instance()->theme());}

    QString theme;
    QVariantHash configs;

    static QHash<States, QString> stateMap;
    static QHash<QString, States> stringMap;
    static QStringList cachedPixmap;
};

QHash<ThemeConfig::States, QString> _ThemeConfig::stateMap;
QHash<QString, ThemeConfig::States> _ThemeConfig::stringMap;
QStringList _ThemeConfig::cachedPixmap;

Q_GLOBAL_STATIC(_ThemeConfig, tc_global)

#define D(t) static_cast<const _ThemeConfig*>(t)

ThemeConfig *ThemeConfig::instace()
{
    return tc_global;
}

bool ThemeConfig::hasValue(const QString &scope, const QString &key, States state) const
{
    return D(this)->configs.contains(QString("%1.%2.%3").arg(scope, key).arg((int)state));
}

void ThemeConfig::setValue(const QString &scope, const QString &key, States state, const QVariant &value)
{
    _ThemeConfig *that = static_cast<_ThemeConfig*>(this);

    that->configs[QString("%1.%2.%3").arg(scope, key).arg((int)state)] = value;
}

static QString stateToString(ThemeConfig::State state)
{
    if (state == ThemeConfig::Normal)
        return QString();

    QString name = _ThemeConfig::stateMap.value(state);

    if (Q_LIKELY(!name.isEmpty()))
        return name;

    if (state & ThemeConfig::Hover) {
        name.append(".hover");
    }

    if (state & ThemeConfig::Pressed) {
        name.append(".pressed");
    }

    if (state & ThemeConfig::Checked) {
        name.append(".checked");
    }

    if (state & ThemeConfig::Dislable) {
        name.append(".disable");
    }

    if (state & ThemeConfig::Focus) {
        name.append(".focus");
    }

    if (state & ThemeConfig::Inactive) {
        name.append(".inactive");
    }

    _ThemeConfig::stateMap[state] = name;

    return name;
}

static ThemeConfig::States stringToState(const QString &string)
{
    ThemeConfig::States state = _ThemeConfig::stringMap.value(string, ThemeConfig::Normal);

    if (Q_UNLIKELY(state != ThemeConfig::Normal))
        return state;

    for (const QString &one : string.split('.')) {
        if (one.isEmpty())
            continue;

        switch (one.at(0).toLatin1()) {
        case 'h':
            if (Q_LIKELY(one == "hover"))
                state |= ThemeConfig::Hover;
            break;
        case 'p':
            if (Q_LIKELY(one == "pressed"))
                state |= ThemeConfig::Pressed;
            break;
        case 'c':
            if (Q_LIKELY(one == "checked"))
                state |= ThemeConfig::Checked;
            break;
        case 'd':
            if (Q_LIKELY(one == "disable"))
                state |= ThemeConfig::Dislable;
            break;
        case 'f':
            if (Q_LIKELY(one == "focus"))
                state |= ThemeConfig::Focus;
            break;
        case 'i':
            if (Q_LIKELY(one == "inactive"))
                state |= ThemeConfig::Inactive;
            break;
        default:
            break;
        }
    }

    _ThemeConfig::stringMap[string] = state;

    return state;
}

QVariant ThemeConfig::value(const QString &scope, const QString &key, States state) const
{
    const QVariant &v = D(this)->configs.value(QString("%1.%2.%3").arg(scope, key).arg((int)state));

    if (Q_UNLIKELY(!v.isValid() && state != Normal)) {
        return value(scope, key, Normal);
    }

    return v;
}

QColor ThemeConfig::color(const QString &scope, const QString &key, States state) const
{
    return qvariant_cast<QColor>(value(scope, key, state));
}

QString ThemeConfig::string(const QString &scope, const QString &key, States state) const
{
    return value(scope, key, state).toString();
}

int ThemeConfig::integer(const QString &scope, const QString &key, States state) const
{
    return value(scope, key, state).toInt();
}

static QPixmap pixmapByPath(const QString &path, qreal scaleRatio)
{
    if (path.isEmpty())
        return QPixmap();

    const QString pixmap_key = QString("%1@%2").arg(path).arg(scaleRatio);
    QPixmap *pixmap = QPixmapCache::find(pixmap_key);

    if (pixmap)
        return *pixmap;

    qreal sourceDevicePixelRatio = 1;
    QImageReader reader(qt_findAtNxFile(path, scaleRatio, &sourceDevicePixelRatio));

    reader.setScaledSize(reader.size() / sourceDevicePixelRatio * scaleRatio);

    const QImage &image = reader.read();
    QPixmap p = QPixmap::fromImage(image);

    p.setDevicePixelRatio(scaleRatio);

    if (QPixmapCache::insert(pixmap_key, p)) {
        _ThemeConfig::cachedPixmap.append(pixmap_key);
    }

    return p;
}

static QPixmap pixmapByIcon(const QIcon &icon, qreal scaleRatio)
{
    if (icon.isNull())
        return QPixmap();

    const QString pixmap_key = QString("%1@%1").arg(icon.cacheKey()).arg(scaleRatio);
    QPixmap *pixmap = QPixmapCache::find(pixmap_key);

    if (pixmap)
        return *pixmap;

    const QSize &size = icon.availableSizes().first();
    QPixmap p = icon.pixmap(size).scaledToHeight(size.height() * scaleRatio, Qt::SmoothTransformation);

    p.setDevicePixelRatio(scaleRatio);

    if (QPixmapCache::insert(pixmap_key, p))
        _ThemeConfig::cachedPixmap.append(pixmap_key);

    return p;
}

QPixmap ThemeConfig::pixmap(const QString &scope, const QString &key, States state, qreal scaleRatio) const
{
    if (scaleRatio < 1)
        scaleRatio = qApp->devicePixelRatio();

    const QVariant &value = string(scope, key, state);

    if (value.type() == QVariant::String) {
        return pixmapByPath(value.toString(), scaleRatio);
    }

    if (value.type() == QVariant::Icon) {
        return pixmapByIcon(qvariant_cast<QIcon>(value), scaleRatio);
    }

    return QPixmap();
}

ThemeConfig::ThemeConfig()
{
    QObject::connect(DThemeManager::instance(), &DThemeManager::widgetThemeChanged, [this] (QWidget *widget, const QString &theme) {
        if (qobject_cast<DFileManagerWindow*>(widget))
            update(theme);
    });
}

void ThemeConfig::update(const QString &theme)
{
    if (theme.isEmpty())
        return;

    _ThemeConfig *that = static_cast<_ThemeConfig*>(this);

    if (that->theme == theme)
        return;

    that->theme = theme;
    that->configs.clear();

    for (const QString &key : _ThemeConfig::cachedPixmap)
        QPixmapCache::remove(key);

    QSettings settings(QString(":/%1/config.ini").arg(theme), QSettings::IniFormat);

    for (const QString &group : settings.childGroups()) {
        settings.beginGroup(group);

        const QStringList &list = group.split(':');
        States state = list.count() > 1 ? stringToState(list.last()) : Normal;

        for (const QString &key : settings.allKeys()) {
            that->configs[QString("%1.%2.%3").arg(list.first(), key).arg((int)state)] = settings.value(key);
        }

        settings.endGroup();
    }
}

DFM_END_NAMESPACE
