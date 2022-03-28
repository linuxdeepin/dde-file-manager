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
#include "private/opendirmenuscene_p.h"

#include <services/common/menu/menu_defines.h>

#include <dfm-base/utils/systempathutil.h>
#include <dfm-base/base/schemefactory.h>

#include <QMenu>
#include <QVariant>

DPMENU_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_USE_NAMESPACE

AbstractMenuScene *OpenDirMenuCreator::create()
{
    return new OpenDirMenuScene();
}

OpenDirMenuScenePrivate::OpenDirMenuScenePrivate(OpenDirMenuScene *qq)
    : AbstractMenuScenePrivate(qq)
{
    predicateName[openAsAdmin] = tr("Open as administrator");
    predicateName[selectAll] = tr("Select all");
    predicateName[openInNewWindow] = tr("Open in new window");
    predicateName[openInNewTab] = tr("Open in new tab");
    predicateName[openInTerminal] = tr("Open in terminal");
}

OpenDirMenuScene::OpenDirMenuScene(QObject *parent)
    : AbstractMenuScene(parent),
      d(new OpenDirMenuScenePrivate(this))
{
}

QString OpenDirMenuScene::name() const
{
    return OpenDirMenuCreator::name();
}

bool OpenDirMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toString();
    d->focusFile = params.value(MenuParamKey::kFocusFile).toString();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).toStringList();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();

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

AbstractMenuScene *OpenDirMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (d->predicateAction.values().contains(action))
        return const_cast<OpenDirMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool OpenDirMenuScene::create(QMenu *parent)
{
    if (d->isEmptyArea) {
        emptyMenu(parent);
    } else {
        normalMenu(parent);
    }

    return true;
}

void OpenDirMenuScene::updateState(QMenu *parent)
{
    if (!parent)
        return;
}

bool OpenDirMenuScene::triggered(QAction *action)
{
    Q_UNUSED(action)
    // TODO(Lee or others):
    return false;
}

void OpenDirMenuScene::emptyMenu(QMenu *parent)
{
    QAction *tempAction = parent->addAction(d->predicateName.value(d->openAsAdmin));
    d->predicateAction[d->openAsAdmin] = tempAction;

    tempAction = parent->addAction(d->predicateName.value(d->selectAll));
    d->predicateAction[d->selectAll] = tempAction;
}

void OpenDirMenuScene::normalMenu(QMenu *parent)
{
    if (d->selectFiles.count() == 1) {
        QAction *tempAction = parent->addAction(d->predicateName.value(d->openInNewWindow));
        d->predicateAction[d->openInNewWindow] = tempAction;

        tempAction = parent->addAction(d->predicateName.value(d->openInNewTab));
        d->predicateAction[d->openInNewTab] = tempAction;

        tempAction = parent->addAction(d->predicateName.value(d->openInTerminal));
        d->predicateAction[d->openInTerminal] = tempAction;

        if (d->focusFileInfo->isDir() && SystemPathUtil::instance()->isSystemPath(d->focusFileInfo->filePath())) {

            tempAction = parent->addAction(d->predicateName.value(d->openAsAdmin));
            d->predicateAction[d->openAsAdmin] = tempAction;
        }
    }
}
