// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openwithmenuscene.h"
#include "action_defines.h"
#include "private/openwithmenuscene_p.h"
#include "menuutils.h"
#include "utils/menuhelper.h"

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
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    const auto &tmpParams = dfmplugin_menu::MenuUtils::perfectMenuParams(params);
    d->isFocusOnDDEDesktopFile = tmpParams.value(MenuParamKey::kIsFocusOnDDEDesktopFile, false).toBool();
    d->isSystemPathIncluded = tmpParams.value(MenuParamKey::kIsSystemPathIncluded, false).toBool();

    if (!d->initializeParamsIsValid()) {
        fmWarning() << "menu scene:" << name() << " init failed." << d->selectFiles.isEmpty() << d->focusFile << d->currentDir;
        return false;
    }

    QString errString;
    d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(d->focusFile, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
    if (d->focusFileInfo.isNull()) {
        fmDebug() << errString;
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

    if (!Helper::canOpenSelectedItems(d->selectFiles))
        return false;

    if (d->isFocusOnDDEDesktopFile || d->isSystemPathIncluded)
        return AbstractMenuScene::create(parent);

    QAction *tempAction = parent->addAction(d->predicateName.value(ActionID::kOpenWith));
    d->predicateAction[ActionID::kOpenWith] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOpenWith));

    QMenu *subMenu = new QMenu(parent);
    tempAction->setMenu(subMenu);
    // create menu using in paint thread,get redirected urls need more time.

    foreach (QString app, d->recommendApps) {
        DesktopFileInfo desktop(QUrl::fromLocalFile(app));
        QAction *action = subMenu->addAction(desktop.fileIcon(), desktop.displayOf(DisPlayInfoType::kFileDisplayName));

        // TODO(Lee or others): 此种外部注入的未分配谓词
        d->predicateAction[app] = action;
        action->setProperty(kAppName, app);
        action->setProperty(ActionPropertyKey::kActionID, ActionID::kOpenWithApp);
    }

    tempAction = subMenu->addAction(d->predicateName.value(ActionID::kOpenWithCustom));
    d->predicateAction[ActionID::kOpenWithCustom] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, ActionID::kOpenWithCustom);

    return AbstractMenuScene::create(parent);
}

void OpenWithMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;

    if (auto openAction = d->predicateAction.value(ActionID::kOpenWith)) {
        if (!d->focusFileInfo->exists())
            openAction->setDisabled(true);
    }

    // open with by focus fileinfo

    AbstractMenuScene::updateState(parent);
}

bool OpenWithMenuScene::triggered(QAction *action)
{
    auto actProperty = action->property(ActionPropertyKey::kActionID);
    if (actProperty != ActionID::kOpenWithApp && actProperty != ActionID::kOpenWithCustom)
        return AbstractMenuScene::triggered(action);

    QList<QUrl> redirectedUrlList;
    for (const auto &fileUrl : d->selectFiles) {
        QString errString;
        auto fileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(fileUrl, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
        if (fileInfo.isNull()) {
            fmDebug() << errString;
            continue;
        }
        redirectedUrlList << fileInfo->urlOf(UrlInfoType::kRedirectedFileUrl);
    }

    if (actProperty == ActionID::kOpenWithApp) {
        auto appName = action->property(kAppName).toString();
        return dpfSignalDispatcher->publish(GlobalEventType::kOpenFilesByApp, 0, redirectedUrlList, QList<QString>() << appName);
    } else {
        dpfSlotChannel->push("dfmplugin_utils", "slot_OpenWith_ShowDialog", d->windowId, redirectedUrlList);
        return true;
    }
}
