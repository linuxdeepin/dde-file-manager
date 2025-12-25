// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tagwidget.h"
#include "private/tagwidget_p.h"
#include "utils/tagmanager.h"
#include "utils/taghelper.h"
#include "tagcrumbedit.h"
#include "tagcolorlistwidget.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-framework/event/event.h>

#include <DArrowRectangle>

using namespace dfmplugin_tag;

TagWidget::TagWidget(QWidget *parent)
    : DFrame(parent),
      d(new TagWidgetPrivate(this, QUrl()))
{
}

TagWidget::TagWidget(QUrl url, QWidget *parent)
    : DFrame(parent),
      d(new TagWidgetPrivate(this, url))
{
}

TagWidget::~TagWidget()
{
}

void TagWidget::initialize()
{
    d->initializeUI();
#ifdef ENABLE_TESTING
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<QFrame *>(this), AcName::kAcTagWidget);
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<DLabel *>(d->tagLable), AcName::kAcTagLable);
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<TagCrumbEdit *>(d->crumbEdit), ::AcName::kAcTagCrumbEdit);
    dpfSlotChannel->push("dfmplugin_utils", "slot_Accessible_SetAccessibleName",
                         qobject_cast<DLabel *>(d->tagLeftLable), ::AcName::kAcTagLeftLable);
#endif
    initConnection();

    loadTags(d->url);
}

void TagWidget::setLayoutHorizontally(bool horizontal)
{
    d->horizontalLayout = horizontal;
}

void TagWidget::setUrl(const QUrl &url)
{
    if (d->url == url)
        return;

    d->url = url;
    loadTags(url);
}

void TagWidget::loadTags(const QUrl &url)
{
    if (!d->crumbEdit || !d->colorListWidget || !shouldShow(url))
        return;

    const QStringList tagNameList = TagManager::instance()->getTagsByUrls({ url.toString() });
    QMap<QString, QColor> tagsMap = TagManager::instance()->getTagsColor(tagNameList);
    QList<QColor> selectColors;
    d->crumbEdit->setProperty("LoadFileTags", true);
    d->crumbEdit->clear();

    for (auto it = tagsMap.begin(); it != tagsMap.end(); ++it) {
        // defualt tag need show checked
        if (TagHelper::instance()->isDefualtTag(it.key()))
            selectColors << it.value();

        DCrumbTextFormat format = d->crumbEdit->makeTextFormat();
        format.setText(it.key());

        format.setBackground(QBrush(it.value()));
        format.setBackgroundRadius(5);

        d->crumbEdit->insertCrumb(format);
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
    if (!d->crumbEdit->isEditing() && !d->crumbEdit->property("updateCrumbsColor").toBool()) {
        updateCrumbsColor(TagManager::instance()->assignColorToTags((d->crumbEdit->crumbList())));
        if (!d->crumbEdit->property("LoadFileTags").toBool()) {
            bool ret = TagManager::instance()->setTagsForFiles(d->crumbEdit->crumbList(), { d->url });

            if (!ret) {
                loadTags(d->url);
                return;
            }
        }
    }
}

void TagWidget::onCheckedColorChanged(const QColor &color)
{
    Q_UNUSED(color)

    const QStringList &tagNameList = TagManager::instance()->getTagsByUrls({ d->url });
    QMap<QString, QColor> nameColors = TagManager::instance()->getTagsColor(tagNameList);
    QList<QUrl> urlList { d->url };
    QList<QColor> checkedColors = d->colorListWidget->checkedColorList();

    QStringList newTagNames;
    for (const QColor &color : checkedColors) {
        QString colorName = TagHelper::instance()->qureyDisplayNameByColor(color);
        if (colorName.isEmpty())
            continue;

        newTagNames << colorName;
    }

    for (const QString &tagName : tagNameList) {
        if (!TagHelper::instance()->isDefualtTag(tagName))
            newTagNames << tagName;
    }

    TagManager::instance()->setTagsForFiles(newTagNames, urlList);
    loadTags(d->url);
}

void TagWidget::onTagChanged(const QVariantMap &fileAndTags)
{
    if (fileAndTags.contains(d->url.path()))
        loadTags(d->url);
}

void TagWidget::filterInput()
{
    TagHelper::instance()->crumbEditInputFilter(d->crumbEdit);
}

void TagWidget::initConnection()
{
    if (!d->crumbEdit || !d->colorListWidget)
        return;

    connect(d->crumbEdit, &QTextEdit::textChanged, this, &TagWidget::filterInput);
    connect(d->crumbEdit, &DCrumbEdit::crumbListChanged, this, &TagWidget::onCrumbListChanged);
    connect(d->colorListWidget, &TagColorListWidget::checkedColorChanged, this, &TagWidget::onCheckedColorChanged);

    connect(TagManager::instance(), &TagManager::filesTagged, this, &TagWidget::onTagChanged);
    connect(TagManager::instance(), &TagManager::filesUntagged, this, &TagWidget::onTagChanged);
}

void TagWidget::updateCrumbsColor(const QMap<QString, QColor> &tagsColor)
{
    if (tagsColor.isEmpty())
        return;

    d->crumbEdit->setProperty("updateCrumbsColor", true);
    d->crumbEdit->clear();

    for (auto it = tagsColor.begin(); it != tagsColor.end(); ++it) {
        DCrumbTextFormat format = d->crumbEdit->makeTextFormat();
        format.setText(it.key());

        format.setBackground(QBrush(it.value()));
        format.setBackgroundRadius(5);

        d->crumbEdit->insertCrumb(format);
    }

    d->crumbEdit->setProperty("updateCrumbsColor", false);
}
