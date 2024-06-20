// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "base/application/application.h"
#include "private/application_p.h"

#include "base/application/settings.h"

#include <QCoreApplication>
#include <QMetaEnum>
#include <QtConcurrent>
#include <QDBusInterface>

Q_LOGGING_CATEGORY(logDFMBase, "org.deepin.dde.filemanager.lib.base")

namespace dfmbase {

Q_GLOBAL_STATIC_WITH_ARGS(Settings, gsGlobal, ("deepin/dde-file-manager", Settings::kGenericConfig))
Q_GLOBAL_STATIC_WITH_ARGS(Settings, asGlobal, ("deepin/dde-file-manager/dde-file-manager", Settings::kGenericConfig))

Q_GLOBAL_STATIC_WITH_ARGS(Settings, gosGlobal, ("deepin/dde-file-manager.obtusely", Settings::kGenericConfig))
Q_GLOBAL_STATIC_WITH_ARGS(Settings, aosGlobal, ("deepin/dde-file-manager/dde-file-manager.obtusely", Settings::kGenericConfig))

Q_GLOBAL_STATIC_WITH_ARGS(Settings, dpGlobal, ("", "", "/tmp/.config/deepin/dde-file-manager/dde-file-manager.dp"))

// blumia: since dde-desktop now also do show file selection dialog job, thus dde-desktop should share the same config file
//         with dde-file-manager, so we use GenericConfig with specify path to simulate AppConfig.

Application *ApplicationPrivate::self = nullptr;

ApplicationPrivate::ApplicationPrivate(Application *qq)
{
    Q_ASSERT_X(!self, "Application", "there should be only one application object");
    self = qq;
}

ApplicationPrivate::~ApplicationPrivate()
{
    self = nullptr;
}

void ApplicationPrivate::_q_onSettingsValueChanged(const QString &group, const QString &key, const QVariant &value, bool edited)
{
    if (group == QT_STRINGIFY(ApplicationAttribute)) {
        const QMetaEnum &me = QMetaEnum::fromType<Application::ApplicationAttribute>();

        bool ok = false;
        Application::ApplicationAttribute aa = static_cast<Application::ApplicationAttribute>(me.keyToValue(QByteArray("k" + key.toLatin1()).constData(), &ok));
        if (!ok) {
            qCWarning(logDFMBase) << "Cannot cast value " << key << " to ApplicationAttribute!";
            return;
        }

        if (edited)
            Q_EMIT self->appAttributeEdited(aa, value);

        Q_EMIT self->appAttributeChanged(aa, value);

        if (aa == Application::kIconSizeLevel) {
            Q_EMIT self->iconSizeLevelChanged(value.toInt());
        } else if (aa == Application::kViewMode) {
            Q_EMIT self->viewModeChanged(value.toInt());
        }
    } else if (group == QT_STRINGIFY(GenericAttribute)) {
        const QMetaEnum &me = QMetaEnum::fromType<Application::GenericAttribute>();

        bool ok = false;
        Application::GenericAttribute ga = static_cast<Application::GenericAttribute>(me.keyToValue(QByteArray("k" + key.toLatin1()).constData(), &ok));
        if (!ok) {
            qCWarning(logDFMBase) << "Cannot cast value " << key << " to GenericAttribute!";
            return;
        }

        if (edited)
            Q_EMIT self->genericAttributeEdited(ga, value);

        Q_EMIT self->genericAttributeChanged(ga, value);

        switch (ga) {
        case Application::kPreviewDocumentFile:
        case Application::kPreviewImage:
        case Application::kPreviewTextFile:
        case Application::kPreviewVideo:
        case Application::kPreviewAudio:
        case Application::kPreviewCompressFile:
        case Application::kShowThunmbnailInRemote:
            if (ga == Application::kPreviewCompressFile)
                Q_EMIT self->previewCompressFileChanged(value.toBool());
            Q_EMIT self->previewAttributeChanged(ga, value.toBool());
            break;
        case Application::kShowedHiddenFiles:
            Q_EMIT self->showedHiddenFilesChanged(value.toBool());
            break;
        case Application::kShowedFileSuffix:
            Q_EMIT self->showedFileSuffixChanged(value.toBool());
            break;
        case Application::kShowCsdCrumbBarClickableArea:
            Q_EMIT self->csdClickableAreaAttributeChanged(value.toBool());
            break;
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
    const QString key = QString::fromLatin1(me.valueToKey(aa)).remove(0, 1);

    return appSetting()->value(group, key);
}

QUrl Application::appUrlAttribute(Application::ApplicationAttribute aa)
{
    const QString group(QT_STRINGIFY(ApplicationAttribute));
    const QMetaEnum &me = QMetaEnum::fromType<ApplicationAttribute>();
    const QString key = QString::fromLatin1(me.valueToKey(aa)).remove(0, 1);

    return appSetting()->urlValue(group, key);
}

void Application::setAppAttribute(Application::ApplicationAttribute aa, const QVariant &value)
{
    const QString group(QT_STRINGIFY(ApplicationAttribute));
    const QMetaEnum &me = QMetaEnum::fromType<ApplicationAttribute>();
    const QString key = QString::fromLatin1(me.valueToKey(aa)).remove(0, 1);

    // clear all self iconSize, use globbal iconSize
    if (key == "IconSizeLevel") {
        auto settings = appObtuselySetting();
        const QStringList &keys = settings->keyList("FileViewState");
        for (const QString &url : keys) {
            auto map = settings->value("FileViewState", url).toMap();
            if (map.contains("iconSizeLevel")) {
                qCDebug(logDFMBase) << "reset" << url << "iconSizeLevel to " << value.toInt();
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

QVariant Application::genericAttribute(Application::GenericAttribute ga)
{
    const QString group(QT_STRINGIFY(GenericAttribute));
    const QMetaEnum &me = QMetaEnum::fromType<GenericAttribute>();
    const QString key = QString::fromLatin1(me.valueToKey(ga)).remove(0, 1);

    return genericSetting()->value(group, key);
}

void Application::setGenericAttribute(Application::GenericAttribute ga, const QVariant &value)
{
    const QString group(QT_STRINGIFY(GenericAttribute));
    const QMetaEnum &me = QMetaEnum::fromType<GenericAttribute>();
    const QString key = QString::fromLatin1(me.valueToKey(ga)).remove(0, 1);

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

Settings *Application::dataPersistence()
{
    if (!dpGlobal.exists()) {
        dpGlobal->setAutoSync(true);
#ifndef DFM_NO_FILE_WATCHER
        dpGlobal->setWatchChanges(true);
#endif
    }

    return dpGlobal;
}

void Application::appAttributeTrigger(TriggerAttribute ta, quint64 winId)
{
    switch (ta) {
    case kRestoreViewMode: {
        auto defaultViewMode = appAttribute(Application::kViewMode).toInt();
        auto settings = appObtuselySetting();

        const QStringList &keys = settings->keyList("FileViewState");
        const QStringList &defaultKeys = settings->defaultConfigkeyList("FileViewState");
        for (const QString &url : keys) {
            if (defaultKeys.contains(url)) {
                auto defaultMap = settings->defaultConfigValue("FileViewState", url).toMap();
                if (defaultMap.contains("viewMode") && defaultMap["viewMode"] != defaultViewMode)
                    continue;
            }

            auto map = settings->value("FileViewState", url).toMap();
            if (map.contains("viewMode")) {
                qCDebug(logDFMBase) << "Set " << url << "viewMode to " << defaultViewMode;
                map["viewMode"] = defaultViewMode;
                settings->setValue("FileViewState", url, map);
            }
        }

        settings->sync();

        if (instance())
            Q_EMIT instance()->viewModeChanged(defaultViewMode);
        break;
        }
    case kClearSearchHistory:
        Q_EMIT instance()->clearSearchHistory(winId);
        break;
    }
}

Application::Application(ApplicationPrivate *dd, QObject *parent)
    : QObject(parent), d(dd)
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

}
