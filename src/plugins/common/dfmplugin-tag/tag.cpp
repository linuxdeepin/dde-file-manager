/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
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
#include "tag.h"
#include "files/tagfileinfo.h"
#include "files/tagfilewatcher.h"
#include "files/tagdiriterator.h"
#include "utils/taghelper.h"
#include "utils/tagmanager.h"
#include "utils/tagoperationhelper.h"
#include "widgets/tagwidget.h"
#include "menu/tagmenuscene.h"
#include "menu/tagdirmenuscene.h"
#include "events/tageventreceiver.h"

#include "dfm-base/base/urlroute.h"
#include "dfm-base/base/schemefactory.h"
#include "dfm-base/base/application/application.h"
#include "services/common/propertydialog/propertydialogservice.h"
#include "services/filemanager/detailspace/detailspaceservice.h"
#include "services/common/menu/menuservice.h"
#include "services/common/delegate/delegateservice.h"

#include <dfm-framework/dpf.h>

#include <QRectF>

Q_DECLARE_METATYPE(QRectF *)

DSC_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSB_FM_USE_NAMESPACE
DPTAG_USE_NAMESPACE

void Tag::initialize()
{
    UrlRoute::regScheme(TagManager::scheme(), "/", {}, true, tr("Tag"));

    InfoFactory::regClass<TagFileInfo>(TagManager::scheme());
    WatcherFactory::regClass<TagFileWatcher>(TagManager::scheme());
    DirIteratorFactory::regClass<TagDirIterator>(TagManager::scheme());
    delegateServIns->registerUrlTransform(TagManager::scheme(), TagHelper::redirectTagUrl);

    connect(TagHelper::winServIns(), &WindowsService::windowOpened, this, &Tag::onWindowOpened, Qt::DirectConnection);
    connect(dpfListener, &dpf::Listener::pluginsInitialized, this, &Tag::onAllPluginsInitialized, Qt::DirectConnection);

    TagManager::instance();
}

bool Tag::start()
{
    PropertyDialogService::service()->registerControlExpand(Tag::createTagWidget, 0, nullptr);
    DetailSpaceService::serviceInstance()->registerControlExpand(Tag::createTagWidget, 1);

    DetailFilterTypes filter = DetailFilterType::kFileSizeField;
    filter |= DetailFilterType::kFileChangeTImeField;
    filter |= DetailFilterType::kFileInterviewTimeField;
    DetailSpaceService::serviceInstance()->registerFilterControlField(TagManager::scheme(), filter);

    followEvent();
    TagEventReceiver::instance()->initConnect();

    MenuService::service()->registerScene(TagMenuCreator::name(), new TagMenuCreator);
    bindScene("FileOperatorMenu");

    WorkspaceService::service()->setWorkspaceMenuScene(TagManager::scheme(), TagDirMenuCreator::name());
    MenuService::service()->registerScene(TagDirMenuCreator::name(), new TagDirMenuCreator);

    return true;
}

void Tag::onWindowOpened(quint64 windId)
{
    auto window = TagHelper::winServIns()->findWindowById(windId);
    Q_ASSERT_X(window, "Tag", "Cannot find window by id");

    if (window->titleBar())
        regTagCrumbToTitleBar();
    else
        connect(window, &FileManagerWindow::titleBarInstallFinished, this, &Tag::regTagCrumbToTitleBar, Qt::DirectConnection);

    if (window->sideBar())
        installToSideBar();
    else
        connect(window, &FileManagerWindow::sideBarInstallFinished, this, &Tag::installToSideBar, Qt::DirectConnection);
}

void Tag::regTagCrumbToTitleBar()
{
    TitleBar::CustomCrumbInfo info;
    info.scheme = TagManager::scheme();
    info.supportedCb = [](const QUrl &url) -> bool { return url.scheme() == TagManager::scheme(); };
    info.seperateCb = [](const QUrl &url) -> QList<TitleBar::CrumbData> {
        QString tagName = TagHelper::instance()->getTagNameFromUrl(url);
        return { TitleBar::CrumbData(url, tr(""), TagManager::instance()->getTagIconName(tagName)) };
    };

    TagHelper::titleServIns()->addCustomCrumbar(info);
}

void Tag::onAllPluginsInitialized()
{
    TagHelper::workspaceServIns()->addScheme(TagManager::scheme());
    addFileOperations();
}

QWidget *Tag::createTagWidget(const QUrl &url)
{
    auto info = InfoFactory::create<AbstractFileInfo>(url);
    if (!TagManager::instance()->canTagFile(info))
        return nullptr;

    return new TagWidget(url);
}

void Tag::installToSideBar()
{
    QMap<QString, QColor> tagsMap = TagManager::instance()->getAllTags();

    QMap<QString, QColor>::const_iterator it = tagsMap.begin();
    while (it != tagsMap.end()) {
        SideBar::ItemInfo item = TagHelper::instance()->createSidebarItemInfo(it.key());
        TagHelper::sideBarServIns()->addItem(item);
        ++it;
    }
}

void Tag::addFileOperations()
{
    FileOperationsFunctions fileOpeationsHandle(new FileOperationsSpace::FileOperationsInfo);
    fileOpeationsHandle->openFiles = &TagOperationHelper::openFilesHandle;

    TagHelper::fileOperationsServIns()->registerOperations(TagManager::scheme(), fileOpeationsHandle);
}

void Tag::followEvent()
{
    TagHelper::eventSequence()->follow(Workspace::EventType::kPaintListItem, TagManager::instance(), &TagManager::paintListTagsHandle);
    TagHelper::eventSequence()->follow(Workspace::EventType::kPaintIconItem, TagManager::instance(), &TagManager::paintIconTagsHandle);

    // todo(zy) need to delete
    TagHelper::eventSequence()->follow(GlobalEventType::kTempDesktopPaintTag, TagManager::instance(), &TagManager::paintIconTagsHandle);
    // paste
    TagHelper::eventSequence()->follow("dfmplugin_workspace", "hook_PasteFilesShortcut", TagManager::instance(), &TagManager::pasteHandle);
    TagHelper::eventSequence()->follow("dfmplugin_workspace", "hook_FileDrop", TagManager::instance(), &TagManager::fileDropHandle);
}

void Tag::bindScene(const QString &parentScene)
{
    if (MenuService::service()->contains(parentScene)) {
        MenuService::service()->bind(TagMenuCreator::name(), parentScene);
    } else {
        connect(MenuService::service(), &MenuService::sceneAdded, this, [=](const QString &scene) {
            if (scene == parentScene)
                MenuService::service()->bind(TagMenuCreator::name(), scene);
        },
                Qt::DirectConnection);
    }
}
