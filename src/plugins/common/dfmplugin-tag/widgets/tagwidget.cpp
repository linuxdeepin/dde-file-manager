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
#include "tagwidget.h"
#include "private/tagwidget_p.h"
#include "utils/tagmanager.h"
#include "utils/taghelper.h"
#include "tagcrumbedit.h"
#include "tagcolorlistwidget.h"

DPTAG_USE_NAMESPACE

TagWidget::TagWidget(QUrl url, QWidget *parent)
    : QFrame(parent),
      d(new TagWidgetPrivate(this, url))
{
    d->initializeUI();
    initConnection();

    loadTags(url);
}

TagWidget::~TagWidget()
{
}

void TagWidget::loadTags(const QUrl &url)
{
    if (!d->crumbEdit || !d->colorListWidget || !shouldShow(url))
        return;

    const QStringList tagNameList = TagManager::instance()->getTagsByUrls({ url });
    QMap<QString, QColor> tagsMap = TagManager::instance()->getTagsColor(tagNameList);

    if (d->url == url && tagNameList.length() == d->currentTagWithColorMap.count()) {
        bool needRefreshEdit = false;

        for (const QString &tag : tagNameList) {
            QString colorName = TagHelper::instance()->qureyColorNameByDisplayName(tag);

            if (!d->currentTagWithColorMap.contains(tag) || d->currentTagWithColorMap.value(tag) != colorName) {
                needRefreshEdit = true;
                break;
            }
        }

        if (!needRefreshEdit)
            return;
    }

    QList<QColor> selectColors;

    d->crumbEdit->setProperty("LoadFileTags", true);
    d->crumbEdit->clear();
    d->currentTagWithColorMap.clear();
    for (auto it = tagsMap.begin(); it != tagsMap.end(); ++it) {
        // defualt tag need show checked
        if (TagHelper::instance()->isDefualtTag(it.key()))
            selectColors << it.value();

        DCrumbTextFormat format = d->crumbEdit->makeTextFormat();
        format.setText(it.key());

        format.setBackground(QBrush(it.value()));
        format.setBackgroundRadius(5);

        d->crumbEdit->insertCrumb(format, 0);
        d->currentTagWithColorMap.insert(it.key(), TagHelper::instance()->qureyColorNameByColor(it.value()));
    }
    d->crumbEdit->setProperty("LoadFileTags", false);

    d->colorListWidget->setCheckedColorList(selectColors);
}

bool TagWidget::shouldShow(const QUrl &url)
{
    Q_UNUSED(url)
    //TODO(liuyangming)
    return true;
}

void TagWidget::onCrumbListChanged()
{
    if (!d->crumbEdit->isEditing() && !d->crumbEdit->property("LoadFileTags").toBool()) {
        bool ret = TagManager::instance()->setTagsForFiles(d->crumbEdit->crumbList(), { d->url });

        if (!ret) {
            loadTags(d->url);
            return;
        }
    }
}

void TagWidget::onCheckedColorChanged(const QColor &color)
{
    Q_UNUSED(color)

    const QStringList tagNameList = TagManager::instance()->getTagsByUrls({ d->url });
    QMap<QString, QColor> nameColors = TagManager::instance()->getTagsColor(tagNameList);
    QList<QUrl> urlList { d->url };
    QList<QColor> checkedColors = d->colorListWidget->checkedColorList();

    QStringList newTagNames;
    for (const QColor &color : checkedColors) {
        QString tagName = TagHelper::instance()->qureyDisplayNameByColor(color);
        if (tagName.isEmpty())
            continue;

        newTagNames << tagName;
    }

    for (const QString &tagName : tagNameList) {
        if (!TagHelper::instance()->isDefualtTag(tagName))
            newTagNames << tagName;
    }

    TagManager::instance()->setTagsForFiles(newTagNames, urlList);
    loadTags(d->url);
}

void TagWidget::onTagChanged(const QMap<QString, QList<QString>> &fileAndTags)
{
    if (fileAndTags.contains(d->url.path()))
        loadTags(d->url);
}

void TagWidget::initConnection()
{
    if (!d->crumbEdit || !d->colorListWidget)
        return;

    connect(d->crumbEdit, &DCrumbEdit::crumbListChanged, this, &TagWidget::onCrumbListChanged);
    connect(d->colorListWidget, &TagColorListWidget::checkedColorChanged, this, &TagWidget::onCheckedColorChanged);

    connect(TagManager::instance(), &TagManager::filesTagged, this, &TagWidget::onTagChanged);
    connect(TagManager::instance(), &TagManager::filesUntagged, this, &TagWidget::onTagChanged);
}
