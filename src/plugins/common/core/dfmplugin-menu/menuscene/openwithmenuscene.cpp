// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openwithmenuscene.h"
#include "action_defines.h"
#include "private/openwithmenuscene_p.h"
#include "menuutils.h"

#include <dfm-base/mimetype/mimesappsmanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/desktopfileinfo.h>
#include <dfm-base/mimetype/mimesappsmanager.h>
#include <dfm-base/utils/properties.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/dfm_menu_defines.h>

#include <dfm-framework/dpf.h>

#include <QMenu>
#include <QVariant>
#include <QSettings>

using namespace dfmplugin_menu;
DFMBASE_USE_NAMESPACE

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
    d->selectFileInfos = params.value(MenuParamKey::kSelectFileInfos).value<QList<FileInfoPointer>>();
    if (d->selectFiles.count() > 0) {
        d->focusFileInfo = params.value(MenuParamKey::kFocusFileInfo).value<FileInfoPointer>();
        d->focusFile = d->focusFileInfo->urlOf(UrlInfoType::kUrl);
    }
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    d->isFocusOnDDEDesktopFile = params.value(MenuParamKey::kIsFocusOnDDEDesktopFile, false).toBool();
    d->isSystemPathIncluded = params.value(MenuParamKey::kIsSystemPathIncluded, false).toBool();

    if (!d->initializeParamsIsValid()) {
        qWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    MimesAppsManager::instance()->initMimeTypeApps();
    d->recommendApps = MimesAppsManager::instance()->getRecommendedApps(d->focusFileInfo->urlOf(UrlInfoType::kRedirectedFileUrl));

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
    for (const auto &fileInfo : d->selectFileInfos) {
        redirectedUrlList << fileInfo->urlOf(UrlInfoType::kRedirectedFileUrl);
    }

    foreach (QString app, d->recommendApps) {
        DesktopFileInfo desktop(QUrl::fromLocalFile(app));
        QAction *action = subMenu->addAction(desktop.fileIcon(), desktop.displayOf(DisPlayInfoType::kFileDisplayName));

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

        for (auto info : d->selectFileInfos) {
            if (Q_UNLIKELY(info.isNull())) {
                qDebug() << errString;
                break;
            }

            // if the suffix is the same, it can be opened with the same application
            if (info->nameOf(NameInfoType::kSuffix) != d->focusFileInfo->nameOf(NameInfoType::kSuffix)) {

                QStringList mimeTypeList { info->nameOf(NameInfoType::kMimeTypeName) };
                QUrl parentUrl = info->urlOf(UrlInfoType::kParentUrl);
                auto parentInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(info->urlOf(UrlInfoType::kUrl), Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
                if (!info.isNull()) {
                    mimeTypeList << parentInfo->nameOf(NameInfoType::kMimeTypeName);
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
        dpfSlotChannel->push("dfmplugin_utils", "slot_OpenWith_ShowDialog", d->windowId, selectUrls);

        return true;
    }

    return AbstractMenuScene::triggered(action);
}
