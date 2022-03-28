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

#include "private/openwithmenuscene_p.h"

#include <services/common/menu/menu_defines.h>

#include <dfm-base/mimetype/mimesappsmanager.h>
#include <dfm-base/base/schemefactory.h>

#include <QMenu>
#include <QVariant>

DPMENU_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE

AbstractMenuScene *OpenWithMenuCreator::create()
{
    return new OpenWithMenuScene();
}

OpenWithMenuScenePrivate::OpenWithMenuScenePrivate(OpenWithMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[openWith] = tr("Open with");
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
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toString();
    d->focusFile = params.value(MenuParamKey::kFocusFile).toString();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).toStringList();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();

    // 文件不存在，则无文件相关菜单项
    if (d->selectFiles.isEmpty())
        return false;

    QString errString;
    d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(QUrl(d->focusFile), true, &errString);
    if (d->focusFileInfo.isNull()) {
        qDebug() << errString;
        return false;
    }

    return true;
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
    if (0 == d->selectFiles.count() || d->focusFile.isEmpty())
        return false;

    QStringList recommendApps;
    if (!d->focusFileInfo.isNull())
        recommendApps = MimesAppsManager::instance()->getRecommendedApps(d->focusFileInfo->redirectedFileUrl());

    if (0 == recommendApps.size())
        return false;

    QAction *tempAction = parent->addAction(d->predicateName.value(d->openWith));
    d->predicateAction[d->openWith] = tempAction;

    QMenu *subMenu = new QMenu(parent);

    recommendApps.removeOne("/usr/share/applications/dde-open.desktop");
    recommendApps.removeOne("/usr/share/applications/display-im6.q16.desktop");
    recommendApps.removeOne("/usr/share/applications/display-im6.q16hdri.desktop");

    QList<QUrl> redirectedUrlList;
    redirectedUrlList << d->focusFileInfo->redirectedFileUrl();
    for (QString path : d->selectFiles) {
        QString errString;
        auto fileInfo = DFMBASE_NAMESPACE::InfoFactory::create<AbstractFileInfo>(QUrl(d->focusFile), true, &errString);
        if (fileInfo.isNull()) {
            qDebug() << errString;
            continue;
        }
        redirectedUrlList << fileInfo->redirectedFileUrl();
    }

    foreach (const QString &app, recommendApps) {
        DesktopFile desktopFile(app);
        QString actionName = desktopFile.desktopFileName();
        QAction *action = subMenu->addAction(desktopFile.desktopFileName());

        // TODO(Lee or others): 此种外部注入的未分配谓词
        d->predicateAction[actionName] = tempAction;

        // action->setIcon(FileUtils::searchAppIcon(desktopFile));
        action->setProperty("app", app);
        action->setProperty("redirectedUrls", QVariant::fromValue(redirectedUrlList));
        subMenu->addAction(action);
        connect(action, &QAction::triggered, action, [this] {
            QAction *triggeredAction = qobject_cast<QAction *>(sender());

            if (triggeredAction)
                this->triggered(triggeredAction);
        });
    }

    return true;
}

void OpenWithMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;
}

bool OpenWithMenuScene::triggered(QAction *action)
{
    Q_UNUSED(action)
    // TODO(Lee or others):

    return false;
}
