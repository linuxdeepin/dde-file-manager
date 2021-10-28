/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "application/application.h"
#include "private/application_p.h"
#if QT_HAS_INCLUDE("anything_interface.h")
#include "anything_interface.h"
#else
#ifndef DISABLE_QUICK_SEARCH
#define DISABLE_QUICK_SEARCH
#endif
#endif

#include "application/settings.h"
#ifdef FULLTEXTSEARCH_ENABLE
#include "fulltextsearch/fulltextsearch.h"
#endif

#include <QCoreApplication>
#include <QMetaEnum>
#include <QtConcurrent>

DFMBASE_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(Settings, gsGlobal, ("deepin/dde-file-manager", Settings::GenericConfig))
Q_GLOBAL_STATIC_WITH_ARGS(Settings, asGlobal, ("deepin/dde-file-manager/dde-file-manager", Settings::GenericConfig))

Q_GLOBAL_STATIC_WITH_ARGS(Settings, gosGlobal, ("deepin/dde-file-manager.obtusely", Settings::GenericConfig))
Q_GLOBAL_STATIC_WITH_ARGS(Settings, aosGlobal, ("deepin/dde-file-manager/dde-file-manager.obtusely", Settings::GenericConfig))

// blumia: since dde-desktop now also do show file selection dialog job, thus dde-desktop should share the same config file
//         with dde-file-manager, so we use GenericConfig with specify path to simulate AppConfig.



Application *ApplicationPrivate::self = nullptr;

ApplicationPrivate::ApplicationPrivate(Application *qq)
{
    Q_ASSERT_X(!self, "DFMApplication", "there should be only one application object");
    self = qq;
}

void ApplicationPrivate::_q_onSettingsValueChanged(const QString &group, const QString &key, const QVariant &value, bool edited)
{
    if (group == QT_STRINGIFY(ApplicationAttribute)) {
        const QMetaEnum &me = QMetaEnum::fromType<Application::ApplicationAttribute>();

        Application::ApplicationAttribute aa = static_cast<Application::ApplicationAttribute>(me.keyToValue(QByteArray("AA_" + key.toLatin1()).constData()));

        if (edited)
            Q_EMIT self->appAttributeEdited(aa, value);

        Q_EMIT self->appAttributeChanged(aa, value);

        if (aa == Application::AA_IconSizeLevel) {
            Q_EMIT self->iconSizeLevelChanged(value.toInt());
        } else if (aa == Application::AA_ViewMode) {
            Q_EMIT self->viewModeChanged(value.toInt());
        }
    } else if (group == QT_STRINGIFY(GenericAttribute)) {
        const QMetaEnum &me = QMetaEnum::fromType<Application::GenericAttribute>();

        Application::GenericAttribute ga = static_cast<Application::GenericAttribute>(me.keyToValue(QByteArray("GA_" + key.toLatin1()).constData()));

        if (edited)
            Q_EMIT self->genericAttributeEdited(ga, value);

        Q_EMIT self->genericAttributeChanged(ga, value);

        switch (ga) {
        case Application::GA_PreviewDocumentFile:
        case Application::GA_PreviewImage:
        case Application::GA_PreviewTextFile:
        case Application::GA_PreviewVideo:
            Q_EMIT self->previewAttributeChanged(ga, value.toBool());
            break;
        case Application::GA_ShowedHiddenFiles:
            Q_EMIT self->showedHiddenFilesChanged(value.toBool());
            break;
        case Application::GA_ShowRecentFileEntry:
            Q_EMIT self->recentDisplayChanged(value.toBool());
            break;
        case Application::GA_PreviewCompressFile:
            Q_EMIT self->previewCompressFileChanged(value.toBool());
            break;
        case Application::GA_ShowCsdCrumbBarClickableArea:
            Q_EMIT self->csdClickableAreaAttributeChanged(value.toBool());
            break;
#ifdef FULLTEXTSEARCH_ENABLE
        case DFMApplication::GA_IndexFullTextSearch:
            if (value.toBool()) {
                DFMFullTextSearchManager::getInstance()->fulltextIndex("/");/*全文搜索建立索引*/
            }
            break;
#endif
        default:
            break;
        }
    }
}

void ApplicationPrivate::_q_onSettingsValueEdited(const QString &group, const QString &key, const QVariant &value)
{
    _q_onSettingsValueChanged(group, key, value, true);
}

Application::Application(QObject *parent)
    : Application(new ApplicationPrivate(this), parent)
{
    qRegisterMetaType<ApplicationAttribute>();
    qRegisterMetaType<GenericAttribute>();
}

Application::~Application()
{

}

QVariant Application::appAttribute(Application::ApplicationAttribute aa)
{
    const QString group(QT_STRINGIFY(ApplicationAttribute));
    const QMetaEnum &me = QMetaEnum::fromType<ApplicationAttribute>();
    const QString key = QString::fromLatin1(me.valueToKey(aa)).split("_").last();

    return appSetting()->value(group, key);
}

QUrl Application::appUrlAttribute(Application::ApplicationAttribute aa)
{
    const QString group(QT_STRINGIFY(ApplicationAttribute));
    const QMetaEnum &me = QMetaEnum::fromType<ApplicationAttribute>();
    const QString key = QString::fromLatin1(me.valueToKey(aa)).split("_").last();

    return appSetting()->urlValue(group, key);
}

void Application::setAppAttribute(Application::ApplicationAttribute aa, const QVariant &value)
{
    const QString group(QT_STRINGIFY(ApplicationAttribute));
    const QMetaEnum &me = QMetaEnum::fromType<ApplicationAttribute>();
    const QString key = QString::fromLatin1(me.valueToKey(aa)).split("_").last();

    // clear all self iconSize, use globbal iconSize
    if (key == "IconSizeLevel") {
        auto settings = appObtuselySetting();
        const QStringList &keys = settings->keyList("FileViewState");
        for (const QString &url : keys) {
            auto map = settings->value("FileViewState", url).toMap();
            if (map.contains("iconSizeLevel")) {
                qDebug() << "reset" << url << "iconSizeLevel to " << value.toInt();
                map["iconSizeLevel"] = value;
                settings->setValue("FileViewState", url, map);
            }
        }
    }

    appSetting()->setValue(group, key, value);
}

bool Application::syncAppAttribute()
{
    return appSetting()->sync();
}

#ifndef DISABLE_QUICK_SEARCH
static ComDeepinAnythingInterface *getAnythingInterface()
{
    static ComDeepinAnythingInterface *interface = new ComDeepinAnythingInterface("com.deepin.anything", "/com/deepin/anything", QDBusConnection::systemBus());

    return interface;
}
#endif

QVariant Application::genericAttribute(Application::GenericAttribute ga)
{
    if (ga == GA_IndexInternal) {
#ifndef DISABLE_QUICK_SEARCH
        return getAnythingInterface()->autoIndexInternal();
#endif
    } else if (ga == GA_IndexExternal) {
#ifndef DISABLE_QUICK_SEARCH
        return getAnythingInterface()->autoIndexExternal();
#endif
    }

    const QString group(QT_STRINGIFY(GenericAttribute));
    const QMetaEnum &me = QMetaEnum::fromType<GenericAttribute>();
    const QString key = QString::fromLatin1(me.valueToKey(ga)).split("_").last();

    return genericSetting()->value(group, key);
}

void Application::setGenericAttribute(Application::GenericAttribute ga, const QVariant &value)
{
    if (ga == GA_IndexInternal) {
#ifndef DISABLE_QUICK_SEARCH
        return getAnythingInterface()->setAutoIndexInternal(value.toBool());
#endif
    } else if (ga == GA_IndexExternal) {
#ifndef DISABLE_QUICK_SEARCH
        return getAnythingInterface()->setAutoIndexExternal(value.toBool());
#endif
    }

    const QString group(QT_STRINGIFY(GenericAttribute));
    const QMetaEnum &me = QMetaEnum::fromType<GenericAttribute>();
    const QString key = QString::fromLatin1(me.valueToKey(ga)).split("_").last();

    genericSetting()->setValue(group, key, value);
}

bool Application::syncGenericAttribute()
{
    return genericSetting()->sync();
}

Application *Application::instance()
{
    return ApplicationPrivate::self;
}

Settings *Application::genericSetting()
{
    if (!gsGlobal.exists()) {
        if (instance()) {
            gsGlobal->moveToThread(instance()->thread());
            connect(gsGlobal, &Settings::valueChanged,
                    instance(), &Application::onSettingsValueChanged);
            connect(gsGlobal, &Settings::valueEdited,
                    instance(), &Application::onSettingsValueEdited);
        }

        gsGlobal->setAutoSync(true);
#ifndef DFM_NO_FILE_WATCHER
        gsGlobal->setWatchChanges(true);
#endif

        if (instance())
            Q_EMIT instance()->genericSettingCreated(gsGlobal);
    }

    return gsGlobal;
}

Settings *Application::appSetting()
{
    if (!asGlobal.exists()) {
        if (instance()) {
            asGlobal->moveToThread(instance()->thread());
            connect(asGlobal, &Settings::valueChanged,
                    instance(), &Application::onSettingsValueChanged);
            connect(asGlobal, &Settings::valueEdited,
                    instance(), &Application::onSettingsValueEdited);
        }

        asGlobal->setAutoSync(true);
#ifndef DFM_NO_FILE_WATCHER
        asGlobal->setWatchChanges(true);
#endif

        if (instance())
            Q_EMIT instance()->appSettingCreated(asGlobal);
    }

    return asGlobal;
}

Settings *Application::genericObtuselySetting()
{
    if (!gosGlobal.exists()) {
        gosGlobal->setAutoSync(false);
#ifndef DFM_NO_FILE_WATCHER
        gosGlobal->setWatchChanges(false);
#endif
    }

    return gosGlobal;
}

Settings *Application::appObtuselySetting()
{
    if (!aosGlobal.exists()) {
        aosGlobal->setAutoSync(false);
#ifndef DFM_NO_FILE_WATCHER
        aosGlobal->setWatchChanges(true);
#endif
    }

    return aosGlobal;
}

Application::Application(ApplicationPrivate *dd, QObject *parent)
    : QObject(parent)
    , d(dd)
{
    if (gsGlobal.exists()) {
        gsGlobal->moveToThread(thread());
        connect(gsGlobal, &Settings::valueChanged,
                this, &Application::onSettingsValueChanged);
    }

    if (asGlobal.exists()) {
        asGlobal->moveToThread(thread());
        connect(asGlobal, &Settings::valueChanged,
                this, &Application::onSettingsValueChanged);
    }
}

void Application::onSettingsValueChanged(const QString &group, const QString &key, const QVariant &value)
{
    d->_q_onSettingsValueChanged(group, key, value);
}

void Application::onSettingsValueEdited(const QString &group, const QString &key, const QVariant &value)
{
    d->_q_onSettingsValueEdited(group, key, value);
}

DFMBASE_END_NAMESPACE
