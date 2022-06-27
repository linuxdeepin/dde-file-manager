/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "openwithmenuscene.h"
#include "action_defines.h"
#include "private/openwithmenuscene_p.h"
#include "menuutils.h"

#include "services/common/menu/menu_defines.h"

#include "dfm-base/mimetype/mimesappsmanager.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/desktopfileinfo.h"
#include "dfm-base/mimetype/mimesappsmanager.h"
#include "dfm-base/utils/properties.h"
#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/framework.h>

#include <QMenu>
#include <QVariant>
#include <QSettings>

using namespace dfmplugin_menu;
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE

static const char *const kAppName = "AppName";
static const char *const kSelectedUrls = "SelectedUrls";

AbstractMenuScene *OpenWithMenuCreator::create()
{
    return new OpenWithMenuScene();
}

OpenWithMenuScenePrivate::OpenWithMenuScenePrivate(OpenWithMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[ActionID::kOpenWith] = tr("Open with");
    predicateName[ActionID::kOpenWithCustom] = tr("Select default program");
}

OpenWithMenuScene::OpenWithMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new OpenWithMenuScenePrivate(this))
{
}

QString OpenWithMenuScene::name() const
{
    return OpenWithMenuCreator::name();
}

bool OpenWithMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();

    const auto &tmpParams = dfmplugin_menu::MenuUtils::perfectMenuParams(params);
    d->isFocusOnDDEDesktopFile = tmpParams.value(MenuParamKey::kIsFocusOnDDEDesktopFile, false).toBool();
    d->isSystemPathIncluded = tmpParams.value(MenuParamKey::kIsSystemPathIncluded, false).toBool();

    if (!d->initializeParamsIsValid()) {
        qWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    QString errString;
    d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(d->focusFile, true, &errString);
    if (d->focusFileInfo.isNull()) {
        qDebug() << errString;
        return false;
    }

    MimesAppsManager::instance()->initMimeTypeApps();
    d->recommendApps = MimesAppsManager::instance()->getRecommendedApps(d->focusFileInfo->redirectedFileUrl());

    // why?
    d->recommendApps.removeAll("/usr/share/applications/dde-open.desktop");
    d->recommendApps.removeAll("/usr/share/applications/display-im6.q16.desktop");
    d->recommendApps.removeAll("/usr/share/applications/display-im6.q16hdri.desktop");

    return AbstractMenuScene::initialize(params);
}

AbstractMenuScene *OpenWithMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (d->predicateAction.values().contains(action))
        return const_cast<OpenWithMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool OpenWithMenuScene::create(QMenu *parent)
{
    if (d->selectFiles.isEmpty() || !d->focusFile.isValid())
        return false;

    if (d->isFocusOnDDEDesktopFile || d->isSystemPathIncluded)
        return AbstractMenuScene::create(parent);

    QAction *tempAction = parent->addAction(d->predicateName.value(ActionID::kOpenWith));
    d->predicateAction[ActionID::kOpenWith] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOpenWith));

    QMenu *subMenu = new QMenu(parent);
    tempAction->setMenu(subMenu);

    QList<QUrl> redirectedUrlList;
    for (const auto &fileUrl : d->selectFiles) {
        QString errString;
        auto fileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(fileUrl, true, &errString);
        if (fileInfo.isNull()) {
            qDebug() << errString;
            continue;
        }
        redirectedUrlList << fileInfo->redirectedFileUrl();
    }

    foreach (QString app, d->recommendApps) {
        DesktopFileInfo desktop(QUrl::fromLocalFile(app));
        QAction *action = subMenu->addAction(desktop.fileIcon(), desktop.fileDisplayName());

        // TODO(Lee or others): 此种外部注入的未分配谓词
        d->predicateAction[app] = action;
        action->setProperty(kAppName, app);
        action->setProperty(kSelectedUrls, QVariant::fromValue(redirectedUrlList));
        action->setProperty(ActionPropertyKey::kActionID, ActionID::kOpenWithApp);
    }

    tempAction = subMenu->addAction(d->predicateName.value(ActionID::kOpenWithCustom));
    d->predicateAction[ActionID::kOpenWithCustom] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, ActionID::kOpenWithCustom);
    tempAction->setProperty(kSelectedUrls, QVariant::fromValue(redirectedUrlList));

    return AbstractMenuScene::create(parent);
}

void OpenWithMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    // open with
    if (auto openWith = d->predicateAction.value(ActionID::kOpenWith)) {
        // app support mime types
        QStringList supportedMimeTypes;
        QMimeType fileMimeType = d->focusFileInfo->fileMimeType();
        QString defaultAppDesktopFile = MimesAppsManager::getDefaultAppDesktopFileByMimeType(fileMimeType.name());
        QSettings desktopFile(defaultAppDesktopFile, QSettings::IniFormat);
        desktopFile.setIniCodec("UTF-8");
        Properties mimeTypeProperties(defaultAppDesktopFile, "Desktop Entry");
        supportedMimeTypes = mimeTypeProperties.value("MimeType").toString().split(';');
        supportedMimeTypes.removeAll("");

        QString errString;
        QList<QUrl> redirectedUrls;

        for (auto url : d->selectFiles) {
            auto info = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(url, true, &errString);
            if (Q_UNLIKELY(info.isNull())) {
                qDebug() << errString;
                break;
            }

            // if the suffix is the same, it can be opened with the same application
            if (info->suffix() != d->focusFileInfo->suffix()) {

                QStringList mimeTypeList { info->mimeTypeName() };
                QUrl parentUrl = info->parentUrl();
                auto parentInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(url, true, &errString);
                if (!info.isNull()) {
                    mimeTypeList << parentInfo->mimeTypeName();
                }

                bool matched = false;
                // or,the application suooprt mime type contains the type of the url file mime type
                for (const QString &oneMimeType : mimeTypeList) {
                    if (supportedMimeTypes.contains(oneMimeType)) {
                        matched = true;
                        break;
                    }
                }

                // disable open action when there are different opening methods
                if (!matched) {
                    openWith->setDisabled(true);
                    break;
                }
            }
        }
    }

    AbstractMenuScene::updateState(parent);
}

bool OpenWithMenuScene::triggered(QAction *action)
{
    auto actProperty = action->property(ActionPropertyKey::kActionID);
    if (actProperty == ActionID::kOpenWithApp) {
        auto appName = action->property(kAppName).toString();
        auto selectUrls = action->property(kSelectedUrls).value<QList<QUrl>>();

        return dpfSignalDispatcher->publish(GlobalEventType::kOpenFilesByApp, 0, selectUrls, QList<QString>() << appName);
    }

    if (actProperty == ActionID::kOpenWithCustom) {
        auto selectUrls = action->property(kSelectedUrls).value<QList<QUrl>>();
        dpfSlotChannel->push("dfmplugin_utils", "slot_OpenWith_ShowDialog", selectUrls);
        return true;
    }

    return AbstractMenuScene::triggered(action);
}
