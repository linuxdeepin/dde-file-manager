// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taghelper.h"
#include "tagmanager.h"
#include "widgets/tageditor.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <QMap>
#include <QVector>
#include <QPainter>
#include <QUrl>
#include <QIcon>

#include <random>
#include <mutex>

using ContextMenuCallback = std::function<void(quint64 windowId, const QUrl &url, const QPoint &globalPos)>;
using RenameCallback = std::function<void(quint64 windowId, const QUrl &url, const QString &name)>;
Q_DECLARE_METATYPE(ContextMenuCallback);
Q_DECLARE_METATYPE(RenameCallback);

using namespace dfmplugin_tag;
DFMBASE_USE_NAMESPACE
TagHelper *TagHelper::instance()
{
    static TagHelper ins;
    return &ins;
}

QUrl TagHelper::rootUrl()
{
    QUrl rootUrl;
    rootUrl.setScheme(scheme());
    rootUrl.setPath("/");
    return rootUrl;
}

/**
 * @brief 转换被选中文件的url，保证挂载目录不同的相同文件url一致,tag菜单不应该直接使用原生urls，需要转换。
 * @return 返回commonUrls
 */
QList<QUrl> TagHelper::commonUrls(const QList<QUrl> &urls)
{
    if (urls.isEmpty())
        return QList<QUrl>();

    const QUrl &url = urls.first();
    if (UniversalUtils::urlEquals(FileUtils::bindUrlTransform(url), url))
        return urls;

    QList<QUrl> commonUrls;
    for (const auto &file : urls) {
        commonUrls.append(FileUtils::bindUrlTransform(file));
    }
    return commonUrls;
}

QList<QColor> TagHelper::defualtColors() const
{
    QList<QColor> colors;

    for (const TagColorDefine &define : colorDefines) {
        colors << define.color;
    }

    return colors;
}

TagHelper::TagHelper(QObject *parent)
    : QObject(parent)
{
    initTagColorDefines();
}

QColor TagHelper::qureyColorByColorName(const QString &name) const
{
    auto ret = std::find_if(colorDefines.cbegin(), colorDefines.cend(), [name](const TagColorDefine &define) {
        return define.colorName == name;
    });

    if (ret != colorDefines.cend())
        return ret->color;

    return QColor();
}

QColor TagHelper::qureyColorByDisplayName(const QString &name) const
{
    auto ret = std::find_if(colorDefines.cbegin(), colorDefines.cend(), [name](const TagColorDefine &define) {
        return define.displayName == name;
    });

    if (ret != colorDefines.cend())
        return ret->color;

    return QColor();
}

QString TagHelper::qureyColorNameByColor(const QColor &color) const
{
    auto ret = std::find_if(colorDefines.cbegin(), colorDefines.cend(), [color](const TagColorDefine &define) {
        return define.color.name() == color.name();
    });

    if (ret != colorDefines.cend())
        return ret->colorName;

    return QString();
}

QString TagHelper::qureyIconNameByColorName(const QString &colorName) const
{
    auto ret = std::find_if(colorDefines.cbegin(), colorDefines.cend(), [colorName](const TagColorDefine &define) {
        return define.colorName == colorName;
    });

    if (ret != colorDefines.cend())
        return ret->iconName;

    return QString();
}

QString TagHelper::qureyIconNameByColor(const QColor &color) const
{
    auto ret = std::find_if(colorDefines.cbegin(), colorDefines.cend(), [color](const TagColorDefine &define) {
        return define.color.name() == color.name();
    });

    if (ret != colorDefines.cend())
        return ret->iconName;

    return QString();
}

QString TagHelper::qureyDisplayNameByColor(const QColor &color) const
{
    auto ret = std::find_if(colorDefines.cbegin(), colorDefines.cend(), [color](const TagColorDefine &define) {
        return define.color.name() == color.name();
    });

    if (ret != colorDefines.cend())
        return ret->displayName;

    return QString();
}

QString TagHelper::qureyColorNameByDisplayName(const QString &name) const
{
    auto ret = std::find_if(colorDefines.cbegin(), colorDefines.cend(), [name](const TagColorDefine &define) {
        return define.displayName == name;
    });

    if (ret != colorDefines.cend())
        return ret->colorName;

    return QString();
}

QString TagHelper::getTagNameFromUrl(const QUrl &url) const
{
    if (url.scheme() == TagManager::scheme())
        return url.path().mid(1, url.path().length() - 1);

    return QString();
}

QUrl TagHelper::makeTagUrlByTagName(const QString &tag) const
{
    QUrl tagUrl;
    tagUrl.setScheme(TagManager::scheme());
    tagUrl.setPath("/" + tag);

    return tagUrl;
}

QString TagHelper::getColorNameByTag(const QString &tagName) const
{
    auto ret = std::find_if(colorDefines.cbegin(), colorDefines.cend(), [tagName](const TagColorDefine &define) {
        return define.displayName == tagName;
    });

    if (ret != colorDefines.cend())
        return ret->colorName;

    return randomTagDefine().colorName;
}

bool TagHelper::isDefualtTag(const QString &tagName) const
{
    auto ret = std::find_if(colorDefines.cbegin(), colorDefines.cend(), [tagName](const TagColorDefine &define) {
        return define.displayName == tagName;
    });

    return ret != colorDefines.cend();
}

void TagHelper::paintTags(QPainter *painter, QRectF &rect, const QList<QColor> &colors) const
{
    bool antialiasing = painter->testRenderHint(QPainter::Antialiasing);
    const QPen pen = painter->pen();
    const QBrush brush = painter->brush();

    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(QPen(Qt::white, 1));

    for (const QColor &color : colors) {
        QPainterPath circle;

        painter->setBrush(QBrush(color));
        circle.addEllipse(QRectF(QPointF(rect.right() - kTagDiameter, rect.top()), rect.bottomRight()));
        painter->drawPath(circle);
        rect.setRight(rect.right() - kTagDiameter / 2);
    }

    painter->setPen(pen);
    painter->setBrush(brush);
    painter->setRenderHint(QPainter::Antialiasing, antialiasing);
}

QVariantMap TagHelper::createSidebarItemInfo(const QString &tag)
{
    ContextMenuCallback contextMenuCb { TagManager::contenxtMenuHandle };
    RenameCallback renameCb { TagManager::renameHandle };
    Qt::ItemFlags flags { Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEditable };
    QVariantMap infoMap {
        { "Property_Key_Url", makeTagUrlByTagName(tag) },
        { "Property_Key_Group", "Group_Tag" },
        { "Property_Key_DisplayName", tag },
        { "Property_Key_Icon", QIcon::fromTheme(TagManager::instance()->getTagIconName(tag)) },
        { "Property_Key_Editable", true },
        { "Property_Key_QtItemFlags", QVariant::fromValue(flags) },
        { "Property_Key_CallbackContextMenu", QVariant::fromValue(contextMenuCb) },
        { "Property_Key_CallbackRename", QVariant::fromValue(renameCb) },
        { "Property_Key_VisiableControl", "tags" },
        { "Property_Key_VisiableDisplayName", QObject::tr("Added tags") },
        { "Property_Key_ReportName", "Tag" }
    };

    return infoMap;
}

void TagHelper::showTagEdit(const QRectF &parentRect, const QRectF &iconRect, const QList<QUrl> &fileList, bool showInTagDir)
{
    TagEditor *editor = new TagEditor(nullptr, showInTagDir);

    editor->setBaseSize(160, 160);
    editor->setFilesForTagging(fileList);
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->setFocusOutSelfClosing(true);

    const QList<QString> &sameTagsInDiffFiles = TagManager::instance()->getTagsByUrls(fileList);
    editor->setDefaultCrumbs(sameTagsInDiffFiles);

    int showPosX = static_cast<int>(iconRect.center().x());
    int showPosY = static_cast<int>(iconRect.bottom());

    showPosX = qMax(showPosX, static_cast<int>((parentRect.left() + 10)));

    auto subValue = parentRect.bottom() - showPosY;
    if (subValue < editor->height()) {
        editor->setArrowDirection(DArrowRectangle::ArrowDirection::ArrowBottom);
        showPosY = qMin(static_cast<int>(parentRect.bottom()), showPosY);
    }

    editor->show(showPosX, showPosY);
}

void TagHelper::crumbEditInputFilter(DCrumbEdit *edit)
{
    if (!edit)
        return;

    QString srcTcxt = edit->toPlainText().remove(QChar::ObjectReplacementCharacter);
    QRegularExpression rx("[\\\\/\':\\*\\?\"<>|%&]");
    if (!srcTcxt.isEmpty() && srcTcxt.contains(rx)) {
        edit->textCursor().document()->setPlainText(srcTcxt.remove(rx));
        const auto &tagsColors = TagManager::instance()->getTagsColor(edit->crumbList());
        edit->setProperty("updateCrumbsColor", true);

        for (auto it = tagsColors.begin(); it != tagsColors.end(); ++it) {
            DCrumbTextFormat format = edit->makeTextFormat();
            format.setText(it.key());
            format.setBackground(QBrush(it.value()));
            format.setBackgroundRadius(5);
            edit->insertCrumb(format, 0);
        }
        edit->setProperty("updateCrumbsColor", false);
    }
}

void TagHelper::initTagColorDefines()
{
    colorDefines << TagColorDefine("Orange", "dfm_tag_orange", QObject::tr("Orange"), "#ffa503")
                 << TagColorDefine("Red", "dfm_tag_red", QObject::tr("Red"), "#ff1c49")
                 << TagColorDefine("Purple", "dfm_tag_purple", QObject::tr("Purple"), "#9023fc")
                 << TagColorDefine("Navy-blue", "dfm_tag_deepblue", QObject::tr("Navy-blue"), "#3468ff")
                 << TagColorDefine("Azure", "dfm_tag_lightblue", QObject::tr("Azure"), "#00b5ff")
                 << TagColorDefine("Grass-green", "dfm_tag_green", QObject::tr("Green"), "#58df0a")
                 << TagColorDefine("Yellow", "dfm_tag_yellow", QObject::tr("Yellow"), "#fef144")
                 << TagColorDefine("Gray", "dfm_tag_gray", QObject::tr("Gray"), "#cccccc");
}

TagColorDefine TagHelper::randomTagDefine() const
{
    std::random_device device {};

    // Choose a random index in colorDefines
    std::default_random_engine engine(device());
    std::uniform_int_distribution<int> uniform_dist(0, colorDefines.length() - 1);

    return colorDefines.at(uniform_dist(engine));
}

TagColorDefine::TagColorDefine(const QString &colorName, const QString &iconName, const QString &dispaly, const QColor &color)
    : colorName(colorName),
      iconName(iconName),
      displayName(dispaly),
      color(color)
{
}
