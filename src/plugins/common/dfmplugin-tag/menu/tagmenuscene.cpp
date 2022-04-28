/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuyangming<liuyangming@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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
#include "tagmenuscene.h"
#include "private/tagmenuscene_p.h"
#include "utils/taghelper.h"
#include "utils/tagmanager.h"
#include "widgets/tagcolorlistwidget.h"

#include "services/common/menu/menu_defines.h"
#include "services/common/propertydialog/property_defines.h"
#include "services/filemanager/workspace/workspace_defines.h"
#include "dfm-base/dfm_global_defines.h"
#include <dfm-base/base/schemefactory.h>

#include <dfm-framework/framework.h>

#include <QMenu>
#include <QWidgetAction>

DPTAG_USE_NAMESPACE
DSC_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

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
    d->windowId = params.value(MenuParamKey::kWindowId).toULongLong();

    d->predicateName.insert(TagActionId::kActTagColorListKey, "");
    d->predicateName.insert(TagActionId::kActTagAddKey, tr("Add color tags"));

    return AbstractMenuScene::initialize(params);
}

bool TagMenuScene::create(QMenu *parent)
{
    if (!parent)
        return false;

    for (const QUrl &url : d->selectFiles) {
        auto info = InfoFactory::create<AbstractFileInfo>(url, true);
        if (!TagManager::instance()->canTagFile(info))
            return false;
    }

    QAction *colorListAction = createColorListAction();
    colorListAction->setProperty(ActionPropertyKey::kActionID, QString(TagActionId::kActTagColorListKey));
    parent->addAction(colorListAction);
    d->predicateAction.insert(TagActionId::kActTagColorListKey, colorListAction);

    QAction *tagAction = createTagAction();
    tagAction->setProperty(ActionPropertyKey::kActionID, QString(TagActionId::kActTagAddKey));
    parent->addAction(tagAction);
    d->predicateAction.insert(TagActionId::kActTagAddKey, tagAction);

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

    QRectF viewRect = TagHelper::workspaceServIns()->getViewVisibleGeometry(d->windowId);
    QRectF iconRect = TagHelper::workspaceServIns()->getItemRect(d->windowId, d->focusFile, DFMGLOBAL_NAMESPACE::kItemIconRole);

    TagHelper::instance()->showTagEdit(viewRect, iconRect, d->selectFiles);

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
    TagColorListWidget *tagWidget = getMenuListWidget();

    if (tagWidget) {
        if (color.isValid()) {
            const QString &tagName = TagHelper::instance()->qureyDisplayNameByColor(color);
            tagWidget->setToolTipText(tr("Add tag \"%1\"").arg(tagName));
        } else {
            tagWidget->clearToolTipText();
        }
    }
}

void TagMenuScene::onColorClicked(const QColor &color)
{
    Q_UNUSED(color)

    TagColorListWidget *tagWidget = getMenuListWidget();

    if (tagWidget) {
        QList<QColor> colors = tagWidget->checkedColorList();
        QStringList tags = {};
        for (const QColor &color : colors) {
            tags.append(TagHelper::instance()->qureyDisplayNameByColor(color));
        }

        TagManager::instance()->setTagsForFiles(tags, d->selectFiles);
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

QAction *TagMenuScene::createTagAction() const
{
    QAction *action = new QAction;
    action->setText(d->predicateName.value(TagActionId::kActTagAddKey));

    return action;
}

QAction *TagMenuScene::createColorListAction() const
{
    TagColorListWidget *colorListWidget = new TagColorListWidget;
    QWidgetAction *action = new QWidgetAction(nullptr);

    action->setDefaultWidget(colorListWidget);

    QStringList tags = TagManager::instance()->getTagsByUrls(d->selectFiles);
    QList<QColor> colors;

    for (const QString &tag : tags) {
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
