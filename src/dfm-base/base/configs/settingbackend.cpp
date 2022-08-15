/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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

#include "settingbackend.h"
#include "private/settingbackend_p.h"

#include <QDebug>

DFMBASE_USE_NAMESPACE

BidirectionHash<QString, Application::ApplicationAttribute> SettingBackendPrivate::keyToAA {
    { "base.open_action.allways_open_on_new_window", Application::kAllwayOpenOnNewWindow },
    { "base.open_action.open_file_action", Application::kOpenFileMode },
    { "base.new_tab_windows.default_window_path", Application::kUrlOfNewWindow },
    { "base.new_tab_windows.new_tab_path", Application::kUrlOfNewTab },
    { "base.default_view.icon_size", Application::kIconSizeLevel },
    { "base.default_view.view_mode", Application::kViewMode },
    { "base.default_view.view_size_adjustable", Application::kViewSizeAdjustable },
};

BidirectionHash<QString, Application::GenericAttribute> SettingBackendPrivate::keyToGA {
    { "base.hidden_files.show_hidden", Application::kShowedHiddenFiles },
    { "base.hidden_files.show_suffix", Application::kShowedFileSuffix },
    { "advance.index.index_internal", Application::kIndexInternal },
    { "advance.index.index_external", Application::kIndexExternal },
    { "advance.index.index_search", Application::kIndexFullTextSearch },
    { "advance.search.show_hidden", Application::kShowedHiddenOnSearch },
    { "advance.preview.compress_file_preview", Application::kPreviewCompressFile },
    { "advance.preview.text_file_preview", Application::kPreviewTextFile },
    { "advance.preview.document_file_preview", Application::kPreviewDocumentFile },
    { "advance.preview.image_file_preview", Application::kPreviewImage },
    { "advance.preview.video_file_preview", Application::kPreviewVideo },
    { "advance.mount.auto_mount", Application::kAutoMount },
    { "advance.mount.auto_mount_and_open", Application::kAutoMountAndOpen },
    { "advance.mount.mtp_show_bottom_info", Application::kMTPShowBottomInfo },
    { "advance.mount.always_show_offline_remote_connection", Application::kAlwaysShowOfflineRemoteConnections },
    { "advance.dialog.default_chooser_dialog", Application::kOverrideFileChooserDialog },
    { "advance.dialog.delete_confirmation_dialog", Application::kShowDeleteConfirmDialog },
    { "advance.other.hide_system_partition", Application::kHiddenSystemPartition },
    { "advance.other.hide_loop_partitions", Application::kHideLoopPartitions },
    { "advance.other.show_crumbbar_clickable_area", Application::kShowCsdCrumbBarClickableArea },
    { "advance.other.show_filesystemtag_on_diskicon", Application::kShowFileSystemTagOnDiskIcon },
    { "advance.items_in_sidebar.recent", Application::kShowRecentFileEntry }
};

SettingBackend::SettingBackend(QObject *parent)
    : DSettingsBackend(parent), d(new SettingBackendPrivate())
{
    Q_ASSERT(Application::instance());

    connect(Application::instance(), &Application::appAttributeEdited, this, &SettingBackend::onValueChanged);
    connect(Application::instance(), &Application::genericAttributeEdited, this, &SettingBackend::onValueChanged);
}

SettingBackend::~SettingBackend()
{
}

SettingBackend *SettingBackend::instance()
{
    static SettingBackend ins;
    return &ins;
}

QStringList SettingBackend::keys() const
{
    return d->keyToAA.keys() + d->keyToGA.keys() + d->getters.keys();
}

QVariant SettingBackend::getOption(const QString &key) const
{
    auto &&appVal = d->getAsAppAttr(key);
    if (appVal.isValid())
        return appVal;

    auto &&genVal = d->getAsGenAttr(key);
    if (genVal.isValid())
        return genVal;

    // if a setting item is existed both in settings and dconfig the sametime,
    // return it from settings.
    return d->getByFunc(key);
}

void SettingBackend::doSync()
{
}

void SettingBackend::addSettingAccessor(const QString &key, GetOptFunc get, SaveOptFunc set)
{
    if (set)
        d->setters.insert(key, set);
    else
        qInfo() << "null setter function is passed for key: " << key;
    if (get)
        d->getters.insert(key, get);
    else
        qInfo() << "null getter function is passed for key: " << key;
}

void SettingBackend::addSettingAccessor(Application::ApplicationAttribute attr, SaveOptFunc set)
{
    if (!d->keyToAA.containsValue(attr)) {
        qWarning() << "NO mapped for ApplicationAttr::" << attr;
        return;
    }
    auto uiKey = d->keyToAA.key(attr);
    addSettingAccessor(uiKey, nullptr, set);
}

void SettingBackend::addSettingAccessor(Application::GenericAttribute attr, SaveOptFunc set)
{
    if (!d->keyToGA.containsValue(attr)) {
        qWarning() << "NO map for GenericAttr::" << attr;
        return;
    }
    auto uiKey = d->keyToGA.key(attr);
    addSettingAccessor(uiKey, nullptr, set);
}

void SettingBackend::doSetOption(const QString &key, const QVariant &value)
{
    QSignalBlocker blocker(this);
    Q_UNUSED(blocker)

    d->saveAsAppAttr(key, value);
    d->saveAsGenAttr(key, value);
    d->saveByFunc(key, value);
}

void SettingBackend::onValueChanged(int attribute, const QVariant &value)
{
    QString key = d->keyToAA.key(static_cast<Application::ApplicationAttribute>(attribute));

    if (key.isEmpty())
        key = d->keyToGA.key(static_cast<Application::GenericAttribute>(attribute));

    if (key.isEmpty())
        return;

    emit optionChanged(key, value);
}

void SettingBackendPrivate::saveAsAppAttr(const QString &key, const QVariant &val)
{
    int attribute = keyToAA.value(key, static_cast<Application::ApplicationAttribute>(-1));
    if (attribute >= 0)
        Application::instance()->setAppAttribute(static_cast<Application::ApplicationAttribute>(attribute), val);
}

QVariant SettingBackendPrivate::getAsAppAttr(const QString &key)
{
    int attribute = keyToAA.value(key, static_cast<Application::ApplicationAttribute>(-1));
    if (attribute >= 0)
        return Application::instance()->appAttribute(static_cast<Application::ApplicationAttribute>(attribute));
    return QVariant();
}

void SettingBackendPrivate::saveAsGenAttr(const QString &key, const QVariant &val)
{
    int attribute = keyToGA.value(key, static_cast<Application::GenericAttribute>(-1));
    if (attribute >= 0)
        Application::instance()->setGenericAttribute(static_cast<Application::GenericAttribute>(attribute), val);
}

QVariant SettingBackendPrivate::getAsGenAttr(const QString &key)
{
    int attribute = keyToGA.value(key, static_cast<Application::GenericAttribute>(-1));
    if (attribute >= 0)
        return Application::instance()->genericAttribute(static_cast<Application::GenericAttribute>(attribute));
    return QVariant();
}

void SettingBackendPrivate::saveByFunc(const QString &key, const QVariant &val)
{
    if (setters.contains(key)) {
        auto setter = setters.value(key);
        if (setter)
            setter(val);
    }
}

QVariant SettingBackendPrivate::getByFunc(const QString &key)
{
    if (getters.contains(key)) {
        auto getter = getters.value(key);
        if (getter)
            return getter();
    }
    return QVariant();
}
