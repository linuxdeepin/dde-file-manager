// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagmenuscene.h"
#include "private/tagmenuscene_p.h"
#include "utils/taghelper.h"
#include "utils/tagmanager.h"
#include "widgets/tagcolorlistwidget.h"
#include "events/tageventcaller.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_menu_defines.h>
#include <dfm-base/dfm_desktop_defines.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-framework/dpf.h>

#include <QMenu>
#include <QWidgetAction>

using namespace dfmplugin_tag;
DFMBASE_USE_NAMESPACE

static constexpr char kActionGroupId[] { "act_group_id" };

TagMenuScene::TagMenuScene(QObject *parent)
    : AbstractMenuScene(parent), d(new TagMenuScenePrivate(this))
{
}

TagMenuScene::~TagMenuScene()
{
}

QString TagMenuScene::name() const
{
    return TagMenuCreator::name();
}

bool TagMenuScene::initialize(const QVariantHash &params)
{
    d->currentDir = params.value(MenuParamKey::kCurrentDir).toUrl();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    if (d->onDesktop)
        d->onCollection = params.value("OnColletion", false).toBool();

    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    const auto &tmpParams = dfmplugin_menu_util::menuPerfectParams(params);
    d->isDDEDesktopFileIncluded = tmpParams.value(MenuParamKey::kIsDDEDesktopFileIncluded, false).toBool();
    d->isSystemPathIncluded = tmpParams.value(MenuParamKey::kIsSystemPathIncluded, false).toBool();

    d->predicateName.insert(TagActionId::kActTagColorListKey, "");
    d->predicateName.insert(TagActionId::kActTagAddKey, tr("Tag information"));

    QString errString;
    d->focusFileInfo = DFMBASE_NAMESPACE::InfoFactory::create<FileInfo>(d->focusFile, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
    if (d->focusFileInfo.isNull()) {
        fmDebug() << errString;
        return false;
    }

    auto subScenes = subscene();
    if (auto filterScene = dfmplugin_menu_util::menuSceneCreateScene("DConfigMenuFilter"))
        subScenes << filterScene;

    setSubscene(subScenes);

    return AbstractMenuScene::initialize(params);
}

bool TagMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    if (d->isDDEDesktopFileIncluded || d->isSystemPathIncluded || !d->focusFile.isValid())
        return false;
    // create by focus fileinfo
    if (!TagManager::instance()->canTagFile(d->focusFile))
        return false;

    if (!d->focusFileInfo->exists())
        return false;

    // create by focus fileinfo
    d->tagNames = TagManager::instance()->getTagsByUrls({ FileUtils::bindUrlTransform(d->focusFile) });
    QAction *colorListAction = createColorListAction(parent);
    colorListAction->setProperty(ActionPropertyKey::kActionID, QString(TagActionId::kActTagColorListKey));
    parent->addAction(colorListAction);
    d->predicateAction.insert(TagActionId::kActTagColorListKey, colorListAction);

    QAction *tagAction = createTagAction(parent);
    tagAction->setProperty(ActionPropertyKey::kActionID, QString(TagActionId::kActTagAddKey));
    parent->addAction(tagAction);
    d->predicateAction.insert(TagActionId::kActTagAddKey, tagAction);

    // Set same group ID to prevent other menu items from being inserted between them
    colorListAction->setProperty(kActionGroupId, "dfm_tag_group");
    tagAction->setProperty(kActionGroupId, "dfm_tag_group");
    return AbstractMenuScene::create(parent);
}

void TagMenuScene::updateState(QMenu *parent)
{
    // sort
    parent->removeAction(d->predicateAction[TagActionId::kActTagAddKey]);
    parent->insertAction(d->predicateAction[TagActionId::kActTagColorListKey], d->predicateAction[TagActionId::kActTagAddKey]);
    AbstractMenuScene::updateState(parent);
}

bool TagMenuScene::triggered(QAction *action)
{
    if (d->predicateAction.value(TagActionId::kActTagAddKey) != action || !d->focusFile.isValid())
        return false;

    QRectF viewRect;
    QRectF iconRect;

    if (d->onDesktop) {
        QPoint pos(0, 0);
        if (d->onCollection) {   // get rect from collection
            const QString id = TagEventCaller::getCollectionViewId(d->focusFile.toString(), &pos);
            if (id.isEmpty()) {
                fmCritical() << "can not find file on collection" << d->focusFile;
                return true;
            }
            const QRect &visualRect = TagEventCaller::getCollectionVisualRect(id, d->focusFile);
            iconRect = TagEventCaller::getCollectionIconRect(id, visualRect);
            if (auto view = TagEventCaller::getCollectionView(id)) {
                viewRect = d->getSurfaceRect(view);
                QPoint iconTopLeft = view->mapToGlobal(iconRect.topLeft().toPoint());
                iconRect.setRect(iconTopLeft.x(), iconTopLeft.y(), iconRect.width(), iconRect.height());
            } else {
                fmWarning() << "can not get collection view, id:" << id;
            }
        } else {   // get rect from desktop
            int viewIndex = TagEventCaller::getDesktopViewIndex(d->focusFile.toString(), &pos);
            if (viewIndex < 0) {
                fmCritical() << "can not find file on canvas" << d->focusFile << viewIndex;
                return true;
            }
            const QRect &visualRect = TagEventCaller::getVisualRect(viewIndex, d->focusFile);
            iconRect = TagEventCaller::getIconRect(viewIndex, visualRect);
            if (auto view = TagEventCaller::getDesktopView(viewIndex)) {
                viewRect = view->rect();
                QPoint iconTopLeft = view->mapToGlobal(iconRect.topLeft().toPoint());
                iconRect.setRect(iconTopLeft.x(), iconTopLeft.y(), iconRect.width(), iconRect.height());
            } else {
                fmWarning() << "can not get canvas view, index:" << viewIndex;
            }
        }
    } else {
        viewRect = TagEventCaller::getVisibleGeometry(d->windowId);
        iconRect = TagEventCaller::getItemRect(d->windowId, d->focusFile, DFMGLOBAL_NAMESPACE::kItemIconRole);
    }

    TagHelper::instance()->showTagEdit(viewRect, iconRect, d->selectFiles, (d->currentDir.scheme() == TagManager::scheme()));

    return AbstractMenuScene::triggered(action);
}

AbstractMenuScene *TagMenuScene::scene(QAction *action) const
{
    if (action == nullptr)
        return nullptr;

    if (!d->predicateAction.key(action).isEmpty())
        return const_cast<TagMenuScene *>(this);

    return AbstractMenuScene::scene(action);
}

void TagMenuScene::onHoverChanged(const QColor &color)
{
    if (!d->selectFiles.isEmpty()) {
        QList<QColor> sameColors;
        const auto &colorInfos = TagManager::instance()->getTagsColor(d->tagNames);

        if (!colorInfos.isEmpty()) {
            QMap<QString, QColor>::const_iterator dataIt = colorInfos.begin();
            for (; dataIt != colorInfos.end(); ++dataIt) {
                if (Q_LIKELY(dataIt.value().isValid()))
                    sameColors << dataIt.value();
            }
        }

        TagColorListWidget *tagWidget = getMenuListWidget();
        if (!tagWidget)
            return;

        if (Q_LIKELY(color.isValid())) {
            const QString &tagName = TagHelper::instance()->qureyDisplayNameByColor(color);
            if (sameColors.contains(color))
                tagWidget->setToolTipText(tr("Remove tag \"%1\"").arg(tagName));
            else
                tagWidget->setToolTipText(tr("Add tag \"%1\"").arg(tagName));

        } else {
            tagWidget->clearToolTipText();
        }
    }
}

void TagMenuScene::onColorClicked(const QColor &color)
{
    TagColorListWidget *tagWidget = getMenuListWidget();
    if (tagWidget) {
        QList<QColor> colors = tagWidget->checkedColorList();
        if (colors.contains(color)) {
            //add checked tag
            TagManager::instance()->addTagsForFiles({ TagHelper::instance()->qureyDisplayNameByColor(color) }, d->selectFiles);
        } else {
            // delete checked tag
            TagManager::instance()->removeTagsOfFiles({ TagHelper::instance()->qureyDisplayNameByColor(color) }, d->selectFiles);
        }
    }
}

TagColorListWidget *TagMenuScene::getMenuListWidget() const
{
    QAction *action = d->predicateAction.value(TagActionId::kActTagColorListKey);
    if (!action)
        return nullptr;

    if (QWidgetAction *widgetAction = qobject_cast<QWidgetAction *>(action)) {
        if (TagColorListWidget *tagWidget = qobject_cast<TagColorListWidget *>(widgetAction->defaultWidget())) {
            return tagWidget;
        }
    }

    return nullptr;
}

QAction *TagMenuScene::createTagAction(QMenu *parent) const
{
    QAction *action = new QAction(parent);
    action->setText(d->predicateName.value(TagActionId::kActTagAddKey));

    return action;
}

QAction *TagMenuScene::createColorListAction(QMenu *parent) const
{
    TagColorListWidget *colorListWidget = new TagColorListWidget(parent);
    QWidgetAction *action = new QWidgetAction(parent);

    action->setDefaultWidget(colorListWidget);
    // get tags by focus fileinfo
    QStringList tags = TagManager::instance()->getTagsByUrls({ FileUtils::bindUrlTransform(d->focusFile) });
    QList<QColor> colors;

    for (const QString &tag : tags) {
        // The tag name of the database is the display name of the tag
        if (!TagHelper::instance()->isDefualtTag(tag))
            continue;

        const QColor &color = TagHelper::instance()->qureyColorByDisplayName(tag);

        if (Q_LIKELY(color.isValid()))
            colors << color;
    }
    colorListWidget->setCheckedColorList(colors);

    connect(colorListWidget, &TagColorListWidget::hoverColorChanged, this, &TagMenuScene::onHoverChanged);
    connect(colorListWidget, &TagColorListWidget::checkedColorChanged, this, &TagMenuScene::onColorClicked);

    return action;
}

AbstractMenuScene *TagMenuCreator::create()
{
    return new TagMenuScene();
}
