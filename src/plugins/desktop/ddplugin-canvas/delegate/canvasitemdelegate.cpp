// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "canvasitemdelegate_p.h"
#include "itemeditor.h"
#include "view/canvasview.h"
#include "model/canvasselectionmodel.h"
#include "view/canvasview_p.h"
#include "view/operator/fileoperatorproxy.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/iconutils.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/utils/universalutils.h>

#include <dfm-framework/dpf.h>

#include <DApplication>

#include <QPainter>
#include <QPainterPath>

#include <cmath>
#include <mutex>
#include <linux/limits.h>

Q_DECLARE_METATYPE(QRectF *)
Q_DECLARE_METATYPE(dfmbase::ElideTextLayout *)

QT_BEGIN_NAMESPACE
Q_WIDGETS_EXPORT void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);
QT_END_NAMESPACE

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace ddplugin_canvas;

#define EDITOR_SHOW_SUFFIX "_d_whether_show_suffix"

const int CanvasItemDelegate::kTextPadding = 4;
const int CanvasItemDelegate::kIconSpacing = 2;
const int CanvasItemDelegate::kIconBackRadius = 18;
const int CanvasItemDelegate::kIconRectRadius = 4;
const int CanvasItemDelegate::kIconBackgroundMargin = 4;

inline constexpr int kIconModeBackRadiusCoefficient = 16;

CanvasItemDelegatePrivate::CanvasItemDelegatePrivate(CanvasItemDelegate *qq)
    : q(qq)
{
}

CanvasItemDelegatePrivate::~CanvasItemDelegatePrivate()
{
}

ElideTextLayout *CanvasItemDelegatePrivate::createTextlayout(const QModelIndex &index, const QPainter *painter) const
{
    bool showSuffix = Application::instance()->genericAttribute(Application::kShowedFileSuffix).toBool();
    QString name = showSuffix ? index.data(Global::ItemRoles::kItemFileDisplayNameRole).toString()
                              : index.data(Global::ItemRoles::kItemFileBaseNameOfRenameRole).toString();
    int lineHeight = UniversalUtils::getTextLineHeight(index, q->parent()->fontMetrics());
    ElideTextLayout *layout = new ElideTextLayout(name);
    layout->setAttribute(ElideTextLayout::kWrapMode, (uint)QTextOption::WrapAtWordBoundaryOrAnywhere);
    layout->setAttribute(ElideTextLayout::kLineHeight, lineHeight);
    layout->setAttribute(ElideTextLayout::kAlignment, Qt::AlignHCenter);

    if (painter) {
        layout->setAttribute(ElideTextLayout::kFont, painter->font());
        layout->setAttribute(ElideTextLayout::kTextDirection, painter->layoutDirection());
    }

    return layout;
}

bool CanvasItemDelegatePrivate::needExpend(const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rText, QRect *needText) const
{
    // calc that showing text require how large area.
    QRect calcNeedRect = rText;
    calcNeedRect.setBottom(INT_MAX);
    auto needHeight = q->textPaintRect(option, index, calcNeedRect, false).height();
    // expanded rect is in text rect and adds its height.
    calcNeedRect.setHeight(needHeight);
    if (needText)
        *needText = calcNeedRect;

    return calcNeedRect.height() > rText.height();
}

CanvasItemDelegate::CanvasItemDelegate(QAbstractItemView *parentPtr)
    : QStyledItemDelegate(parentPtr), d(new CanvasItemDelegatePrivate(this))
{
    // 初始化图标等级、大小信息
    d->iconSizes << 32 << 48 << 64 << 96 << 128;

    // 初始化默认图标为小
    const int iconLv = 1;
    Q_ASSERT(iconLv < d->iconSizes.size());
    setIconLevel(iconLv);
    d->textLineHeight = parent()->fontMetrics().height();

    connect(ClipBoard::instance(), &ClipBoard::clipboardDataChanged, this, &CanvasItemDelegate::clipboardDataChanged);
}

CanvasItemDelegate::~CanvasItemDelegate()
{
}

QSize CanvasItemDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    Q_UNUSED(opt)
    Q_UNUSED(index)
    return d->itemSizeHint;
}

void CanvasItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // the global option.
    QStyleOptionViewItem indexOption = option;

    // init option for each index.
    // and the rect of index was inited outside.
    initStyleOption(&indexOption, index);

    painter->save();

    // paint a translucent effect.
    painter->setOpacity(isTransparent(index) ? 0.3 : 1.0);

    // get item paint geomerty
    // the method to get rect for each element is equal to paintGeomertys(option, index);
    {
        // draw icon and background
        const QRect rIcon = iconRect(option.rect);
        paintBackground(painter, indexOption, rIcon);
        const std::optional<QRectF> &pIcon = paintIcon( painter, indexOption.icon,
                                                        { rIcon,
                                                          Qt::AlignCenter,
                                                          (option.state & QStyle::State_Enabled) ? QIcon::Normal : QIcon::Disabled,
                                                          QIcon::Off,
                                                          isThumnailIconIndex(index) });   // why Enabled?

        // If the thumbnail drawing is empty, then redraw the file fileicon
        if (!pIcon.has_value()) {
            const QIcon &fileIcon = index.data(Global::ItemRoles::kItemFileIconRole).value<QIcon>();
            paintIcon(painter, fileIcon,
                      { rIcon,
                        Qt::AlignCenter,
                        (option.state & QStyle::State_Enabled) ? QIcon::Normal : QIcon::Disabled,
                        QIcon::Off,
                        false });   // why Enabled?
        }
        // paint emblems to icon
        paintEmblems(painter, rIcon, parent()->model()->fileInfo(index));

        // do not draw text if index is in editing,
        if (!parent()->isPersistentEditorOpen(index)) {
            // draw text
            paintLabel(painter, indexOption, index, labelRect(option.rect, rIcon));
        }
    }

    painter->restore();
}

QWidget *CanvasItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    Q_UNUSED(index);
    auto editor = new ItemEditor(parent);
    if (FileUtils::supportLongName(this->parent()->model()->rootUrl()))
        editor->setCharCountLimit();
    connect(editor, &ItemEditor::inputFocusOut, this, &CanvasItemDelegate::commitDataAndCloseEditor);
    editor->setOpacity(isTransparent(index) ? 0.3 : 1);
    return editor;
}

void CanvasItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    ItemEditor *itemEditor = qobject_cast<ItemEditor *>(editor);
    if (!itemEditor)
        return;

    // option.rect is view->visualRect;
    // geo is equal parent()->itemRect();
    auto geo = option.rect.marginsRemoved(CanvasViewPrivate::gridMarginsHelper(parent()));
    auto margins = QMargins(0, 0, 0, 0);
    {
        // get text rect
        auto gridTop = geo;
        auto icon = iconRect(gridTop);
        auto label = labelRect(gridTop, icon);
        auto text = d->availableTextRect(label);

        // get editor begin pos that is y of text rect minus kTextPadding.
        margins.setTop(text.top() - geo.top() - CanvasItemDelegate::kTextPadding);
    }
    // icon height + kIconSpacing is the editor begin pos.
    // as margins.setTop(parent()->iconSize().height() + kIconSpacing);
    // the max height is from the text editor top to canvas view bottom.
    itemEditor->setMaxHeight(parent()->height() - geo.y());
    itemEditor->setBaseGeometry(geo, d->itemSizeHint, margins);
    return;
}

void CanvasItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    ItemEditor *itemEditor = qobject_cast<ItemEditor *>(editor);
    if (!itemEditor)
        return;

    // 是否显示判断后缀
    bool showSuffix = Application::instance()->genericAttribute(Application::kShowedFileSuffix).toBool();

    QString suffix = index.data(Global::ItemRoles::kItemFileSuffixOfRenameRole).toString();
    fmDebug() << "Display" << index.data(Global::ItemRoles::kItemFileDisplayNameRole).toString()
              << "FileName" << index.data(Global::ItemRoles::kItemNameRole).toString()
              << "FileNameofrenmae" << index.data(Global::ItemRoles::kItemFileNameOfRenameRole).toString()
              << "BaseName" << index.data(Global::ItemRoles::kItemFileBaseNameRole).toString()
              << "BaseNameofrename" << index.data(Global::ItemRoles::kItemFileBaseNameOfRenameRole).toString()
              << "suffix" << index.data(Global::ItemRoles::kItemFileSuffixRole).toString()
              << "suffixofrename" << suffix;
    if (showSuffix) {
        QString name = index.data(Global::ItemRoles::kItemFileNameOfRenameRole).toString();
        itemEditor->setMaximumLength(NAME_MAX);
        itemEditor->setText(name);
        itemEditor->select(name.left(name.size() - suffix.size() - (suffix.isEmpty() ? 0 : 1)));
    } else {
        itemEditor->setProperty(EDITOR_SHOW_SUFFIX, suffix);
        itemEditor->setMaximumLength(NAME_MAX - suffix.toLocal8Bit().size() - (suffix.isEmpty() ? 0 : 1));

        QString name = index.data(Global::ItemRoles::kItemFileBaseNameOfRenameRole).toString();
        itemEditor->setText(name);
        itemEditor->select(name);
    }
}

void CanvasItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    ItemEditor *itemEditor = qobject_cast<ItemEditor *>(editor);
    if (!itemEditor)
        return;

    fmDebug() << index << itemEditor->text();
    QString newName = itemEditor->text();
    if (newName.isEmpty())
        return;

    QString suffix = editor->property(EDITOR_SHOW_SUFFIX).toString();
    if (!suffix.isEmpty())
        newName += QStringLiteral(".") + suffix;

    if (index.data(Global::ItemRoles::kItemFileNameOfRenameRole) == newName)
        return;

    CanvasProxyModel *canvasModel = qobject_cast<CanvasProxyModel *>(model);
    Q_ASSERT(canvasModel);

    if (const FileInfoPointer &fileInfo = canvasModel->fileInfo(index)) {
        QUrl oldUrl = fileInfo->urlOf(UrlInfoType::kUrl);
        QUrl newUrl = fileInfo->getUrlByType(UrlInfoType::kGetUrlByNewFileName, newName);
        QMetaObject::invokeMethod(FileOperatorProxyIns, "renameFile", Qt::QueuedConnection, Q_ARG(int, parent()->winId()), Q_ARG(QUrl, oldUrl), Q_ARG(QUrl, newUrl));
    }
}

bool CanvasItemDelegate::mayExpand(QModelIndex *who) const
{
    const QModelIndexList &list = parent()->selectionModel()->selectedIndexesCache();
    bool expend = list.size() == 1;
    if (expend && who)
        *who = list.first();

    return expend;
}

QRectF CanvasItemDelegate::boundingRect(const QList<QRectF> &rects)
{
    QRectF bounding;

    if (rects.isEmpty())
        return bounding;

    bounding = rects.first();

    for (const QRectF &r : rects) {
        if (r.top() < bounding.top()) {
            bounding.setTop(r.top());
        }

        if (r.left() < bounding.left()) {
            bounding.setLeft(r.left());
        }

        if (r.right() > bounding.right()) {
            bounding.setRight(r.right());
        }

        if (r.bottom() > bounding.bottom()) {
            bounding.setBottom(r.bottom());
        }
    }

    return bounding;
}

QSize CanvasItemDelegate::paintDragIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    // the global option.
    QStyleOptionViewItem indexOption = option;

    // init option for each index.
    // and the rect of index was inited outside.
    initStyleOption(&indexOption, index);

    painter->setRenderHints(painter->renderHints() | QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);
    const std::optional<QRectF> &pIcon = paintIcon( painter, indexOption.icon,
                                                    { indexOption.rect, Qt::AlignCenter, QIcon::Normal,
                                                      QIcon::Off, isThumnailIconIndex(index) });
    // If the thumbnail drawing is empty, then redraw the file fileicon
    if (!pIcon.has_value()) {
        const QIcon &fileIcon = index.data(Global::ItemRoles::kItemFileIconRole).value<QIcon>();
        const std::optional<QRectF> paintRect = paintIcon(painter, fileIcon,
                                                          { indexOption.rect, Qt::AlignCenter, QIcon::Normal,
                                                            QIcon::Off, false });
        if (paintRect.has_value()) {
            return paintRect->size().toSize();
        } else {
            return QSize();
        }
    }

    return pIcon->size().toSize();
}

int CanvasItemDelegate::textLineHeight() const
{
    return d->textLineHeight;
}

QList<QRect> CanvasItemDelegate::paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QList<QRect> geometries;

    // the global option.
    QStyleOptionViewItem indexOption = option;

    // init option for the index.
    // and the rect of index was inited outside.
    initStyleOption(&indexOption, index);

    // icon rect, for draw icon
    auto icon = iconRect(indexOption.rect);
    geometries << icon;

    QRect text;

    // label rect is a hot zone and contain text rect.
    QRect label = labelRect(indexOption.rect, icon);

    // do not expand if in editing.
    if (!parent()->isPersistentEditorOpen(index) && d->isHighlight(indexOption) && mayExpand()) {
        // checking text whether needs to expend and output used text rect.
        d->needExpend(indexOption, index, d->availableTextRect(label), &text);
    } else {
        // calc text rect base on label. and text rect is for draw text.
        text = textPaintRect(indexOption, index, d->availableTextRect(label), true);
    }

    // identification area for mouse press
    {
        auto area = text;
        // text rect spread upward to label.
        area.setTop(label.top());
        geometries << area;
    }

    // painting-used area
    geometries << text;
    return geometries;
}

QRect CanvasItemDelegate::expendedGeomerty(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // the global option.
    QStyleOptionViewItem indexOption = option;

    // init option for the index.
    // and the rect of index was inited outside.
    initStyleOption(&indexOption, index);

    // icon rect, for draw icon
    auto icon = iconRect(indexOption.rect);

    QRect text;

    // label rect is a hot zone and contain text rect.
    QRect label = labelRect(indexOption.rect, icon);

    // checking the text whether needs to expend and output used text rect.
    d->needExpend(indexOption, index, d->availableTextRect(label), &text);

    QRect rect = indexOption.rect;

    // expend
    if (rect.bottom() < text.bottom())
        rect.setBottom(text.bottom());
    return rect;
}

Qt::Alignment CanvasItemDelegate::visualAlignment(Qt::LayoutDirection direction, Qt::Alignment alignment)
{
    if (!(alignment & Qt::AlignHorizontal_Mask))
        alignment |= Qt::AlignLeft;
    if (!(alignment & Qt::AlignAbsolute) && (alignment & (Qt::AlignLeft | Qt::AlignRight))) {
        if (direction == Qt::RightToLeft)
            alignment ^= (Qt::AlignLeft | Qt::AlignRight);
        alignment |= Qt::AlignAbsolute;
    }
    return alignment;
}

QList<QRectF> CanvasItemDelegate::elideTextRect(const QModelIndex &index, const QRect &rect, const Qt::TextElideMode &elideMode) const
{
    // create text Layout.
    QScopedPointer<ElideTextLayout> layout(d->createTextlayout(index));

    d->extendLayoutText(parent()->model()->fileInfo(index), layout.data());

    // elide mode
    auto textLines = layout->layout(rect, elideMode);
    return textLines;
}

bool CanvasItemDelegate::isTransparent(const QModelIndex &index) const
{
    // in cutting
    if (ClipBoard::instance()->clipboardAction() == ClipBoard::kCutAction) {
        FileInfoPointer file = parent()->model()->fileInfo(index);
        if (!file.get())
            return false;

        if (ClipBoard::instance()->clipboardFileUrlList().contains(file->urlOf(UrlInfoType::kUrl)))
            return true;
    }
    return false;
}

bool CanvasItemDelegate::isThumnailIconIndex(const QModelIndex &index) const
{
    if (!index.isValid() || !parent() || !parent()->model())
        return false;

    FileInfoPointer info { parent()->model()->fileInfo(index) };
    if (info) {
        if (IconUtils::shouldSkipThumbnailFrame(info->nameOf(NameInfoType::kMimeTypeName)))
            return false;

        const auto &attribute { info->extendAttributes(ExtInfoType::kFileThumbnail) };
        if (attribute.isValid() && !attribute.value<QIcon>().isNull())
            return true;
    }
    return false;
}

void CanvasItemDelegate::drawNormlText(QPainter *painter, const QStyleOptionViewItem &option,
                                       const QModelIndex &index, const QRectF &rText) const
{
    painter->save();
    painter->setPen(option.palette.color(QPalette::Text));

    qreal pixelRatio = painter->device()->devicePixelRatioF();
    QImage textImage((rText.size() * pixelRatio).toSize(), QImage::Format_ARGB32_Premultiplied);
    textImage.fill(Qt::transparent);
    textImage.setDevicePixelRatio(pixelRatio);

    QPixmap textPixmap;
    {
        QPainter p(&textImage);
        p.setPen(painter->pen());
        p.setFont(painter->font());

        // create text Layout.
        QScopedPointer<ElideTextLayout> layout(d->createTextlayout(index, &p));

        d->extendLayoutText(parent()->model()->fileInfo(index), layout.data());

        // elide and draw
        layout->layout(QRectF(QPoint(0, 0), QSizeF(textImage.size()) / pixelRatio), option.textElideMode, &p);
        p.end();

        textPixmap = QPixmap::fromImage(textImage);
        textPixmap.setDevicePixelRatio(pixelRatio);
        qt_blurImage(textImage, 6, false);

        p.begin(&textImage);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(textImage.rect(), option.palette.color(QPalette::Shadow));
        p.end();
    }

    painter->drawImage(rText.translated(0, 1), textImage);
    painter->drawPixmap(rText.topLeft(), textPixmap);
    painter->restore();
}

void CanvasItemDelegate::drawHighlightText(QPainter *painter, const QStyleOptionViewItem &option,
                                           const QModelIndex &index, const QRect &rText) const
{
    // single item selected and not in drag will to expand.
    bool isDrag = painter->device() != parent()->viewport();
    if (mayExpand() && !isDrag) {
        QRect needRect;
        if (d->needExpend(option, index, rText, &needRect)) {
            drawExpandText(painter, option, index, needRect);
            return;
        }
    }

    {
        painter->save();
        painter->setPen(option.palette.color(QPalette::BrightText));
        auto background = option.palette.brush(QPalette::Normal, QPalette::Highlight);

        // create text Layout.
        QScopedPointer<ElideTextLayout> layout(d->createTextlayout(index, painter));
        layout->setAttribute(ElideTextLayout::kBackgroundRadius, kIconRectRadius);

        d->extendLayoutText(parent()->model()->fileInfo(index), layout.data());

        // elide and draw
        layout->layout(rText, option.textElideMode, painter, background);
        painter->restore();
    }
}

void CanvasItemDelegate::drawExpandText(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRectF &rect) const
{
    painter->save();
    painter->setPen(option.palette.color(QPalette::BrightText));
    auto background = option.palette.brush(QPalette::Normal, QPalette::Highlight);

    // create text Layout.
    QScopedPointer<ElideTextLayout> layout(d->createTextlayout(index, painter));
    layout->setAttribute(ElideTextLayout::kBackgroundRadius, kIconRectRadius);

    d->extendLayoutText(parent()->model()->fileInfo(index), layout.data());

    // elide and draw
    layout->layout(rect, option.textElideMode, painter, background);
    painter->restore();
}

QPixmap CanvasItemDelegate::getIconPixmap(const QIcon &icon, const QSize &size,
                                          qreal pixelRatio, QIcon::Mode mode, QIcon::State state)
{
    if (icon.isNull())
        return QPixmap();

    // 确保当前参数参入获取图片大小大于0
    if (size.width() <= 0 || size.height() <= 0)
        return QPixmap();

    // 根据设备像素比获取合适大小的pixmap
    QSize deviceSize = size * pixelRatio;
    auto px = icon.pixmap(deviceSize, mode, state);
    px.setDevicePixelRatio(pixelRatio);

    return px;
}

CanvasView *CanvasItemDelegate::parent() const
{
    return dynamic_cast<CanvasView *>(QObject::parent());
}

QSize CanvasItemDelegate::iconSize(int lv) const
{
    if (lv >= minimumIconLevel() && lv <= maximumIconLevel()) {
        int size = d->iconSizes.at(lv);
        return QSize(size, size);
    }

    return QSize();
}

int CanvasItemDelegate::iconLevel() const
{
    return d->currentIconLevel;
}

int CanvasItemDelegate::setIconLevel(int lv)
{
    if (lv == d->currentIconLevel)
        return lv;

    if (lv >= minimumIconLevel() && lv <= maximumIconLevel()) {
        d->currentIconLevel = lv;
        parent()->setIconSize(iconSize(lv));
        return lv;
    }

    return -1;
}

int CanvasItemDelegate::minimumIconLevel() const
{
    return 0;
}

int CanvasItemDelegate::maximumIconLevel() const
{
    return d->iconSizes.count() - 1;
}

QRect CanvasItemDelegate::iconRect(const QRect &paintRect) const
{
    QRect rIcon = paintRect;

    rIcon.setSize(parent()->iconSize());
    rIcon.moveCenter(paintRect.center());
    rIcon.moveTop(paintRect.top());

    return rIcon;
}

QRect CanvasItemDelegate::labelRect(const QRect &paintRect, const QRect &usedRect)
{
    QRect lable = paintRect;
    // label rect is under the icon.
    lable.setTop(usedRect.bottom() + kIconBackgroundMargin);

    // minus text padding at left and right.
    lable.setWidth(paintRect.width() - 2 * kTextPadding);

    // move left begin text padding.
    lable.moveLeft(lable.left() + kTextPadding);

    return lable;
}

QRect CanvasItemDelegate::textPaintRect(const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rText, bool elide) const
{
    QRect rect = rText;

    // per line
    auto lines = elideTextRect(index, rect, elide ? option.textElideMode : Qt::ElideNone);

    // total rect
    rect = boundingRect(lines).toRect();
    return rect;
}

void CanvasItemDelegate::updateItemSizeHint() const
{
    d->textLineHeight = parent()->fontMetrics().height();
    int width = parent()->iconSize().width() * 17 / 10;
    int height = parent()->iconSize().height() + kIconSpacing + kTextPadding + 2 * d->textLineHeight + kTextPadding;
    d->itemSizeHint = QSize(width, height);
}

void CanvasItemDelegate::commitDataAndCloseEditor()
{
    // todo 需观察editor不是currentIndex的情况
    auto view = parent();
    QModelIndex index = view->currentIndex();
    if (view->isPersistentEditorOpen(index)) {
        QWidget *editor = parent()->indexWidget(index);
        if (editor) {
            // send to view and call method of the same name.
            emit commitData(editor);
            emit closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
        } else {
            fmWarning() << "currentIndex is not in editing.";
        }
    }
}

void CanvasItemDelegate::revertAndcloseEditor()
{
    // todo 需观察editor不是currentIndex的情况
    auto view = parent();
    QModelIndex index = view->currentIndex();
    if (view->isPersistentEditorOpen(index))
        view->closePersistentEditor(index);
}

void CanvasItemDelegate::clipboardDataChanged()
{
    auto index = parent()->currentIndex();
    if (parent()->isPersistentEditorOpen(index)) {
        if (ItemEditor *wid = qobject_cast<ItemEditor *>(parent()->indexWidget(index)))
            wid->setOpacity(isTransparent(index) ? 0.3 : 1);
    }

    parent()->update();
}

void CanvasItemDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    auto view = parent();
    auto model = view->selectionModel();

    // selected
    if (model->isSelected(index)) {
        // set seleted state. it didn't be seted in QStyledItemDelegate::initStyleOption.
        option->state |= QStyle::State_Selected;
    } else {
        option->state &= QStyle::StateFlag(~QStyle::State_Selected);
    }

    // enable
    if (option->state & QStyle::State_Enabled) {
        QPalette::ColorGroup cg;
        if (view->model()->flags(index) & Qt::ItemIsEnabled) {
            cg = QPalette::Normal;
        } else {   // item is not enable.
            option->state &= ~QStyle::State_Enabled;
            cg = QPalette::Disabled;
        }
        option->palette.setCurrentColorGroup(cg);
    }

    option->palette.setColor(QPalette::Text, QColor("white"));
    option->palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#797979"));

    // selected and show selected decoration need highlight
    if ((option->state & QStyle::State_Selected) && option->showDecorationSelected) {
        option->palette.setColor(QPalette::Inactive, QPalette::Text, QColor("#e9e9e9"));
    } else {
        option->palette.setColor(QPalette::Inactive, QPalette::Text, QColor("#797979"));
    }

    option->palette.setColor(QPalette::BrightText, Qt::white);
    option->palette.setBrush(QPalette::Shadow, QColor(0, 0, 0, 178));

    // multi-selected background
    if ((option->state & QStyle::State_HasFocus) && option->showDecorationSelected && model->selectedIndexesCache().size() > 1) {
        option->palette.setColor(QPalette::Window, QColor("#0076F9"));
        option->backgroundBrush = QColor("#0076F9");
    } else {   // normal
        option->palette.setColor(QPalette::Window, QColor("#2da6f7"));
        option->backgroundBrush = QColor("#2da6f7");
    }

    // int cut
    if (isTransparent(index))
        option->backgroundBrush = QColor("#BFE4FC");

    // why?
    option->state &= ~QStyle::State_MouseOver;

    // elide mode for each file
    option->textElideMode = Qt::ElideLeft;
}

/*!
 * \brief paint icon
 * \param painter
 * \param icon: the icon to paint
 * \param rect: icon rect
 * \param alignment: alignment if icon
 * \param mode: icon mode (Normal, Disabled, Active, Selected )
 * \param state: The state for which a pixmap is intended to be used. (On, Off)
 */
std::optional<QRectF> CanvasItemDelegate::paintIcon(QPainter *painter, const QIcon &icon, const PaintIconOpts &opts)
{
    // Copy of QStyle::alignedRect
    Qt::Alignment alignment { visualAlignment(painter->layoutDirection(), opts.alignment) };
    const qreal pixelRatio = painter->device()->devicePixelRatioF();
    const QPixmap &px = getIconPixmap(icon, opts.rect.size().toSize(), pixelRatio, opts.mode, opts.state);

    // 缩略图缩放到指定的size，绘制不出来就直接返回，绘制fileicon
    if (px.isNull() && opts.isThumb)
        return std::nullopt;
    // 保持图标原始比例
    qreal w = px.width() / px.devicePixelRatio();
    qreal h = px.height() / px.devicePixelRatio();

    // 如果图标大于目标区域，等比例缩放
    if (w > opts.rect.width() || h > opts.rect.height()) {
        qreal scale = qMin(opts.rect.width() / w, opts.rect.height() / h);
        w *= scale;
        h *= scale;
    }

    qreal x = opts.rect.x();
    qreal y = opts.rect.y();

    if ((alignment & Qt::AlignVCenter) == Qt::AlignVCenter)
        y += (opts.rect.size().height() - h) / 2.0;
    else if ((alignment & Qt::AlignBottom) == Qt::AlignBottom)
        y += opts.rect.size().height() - h;
    if ((alignment & Qt::AlignRight) == Qt::AlignRight)
        x += opts.rect.size().width() - w;
    else if ((alignment & Qt::AlignHCenter) == Qt::AlignHCenter)
        x += (opts.rect.size().width() - w) / 2.0;

    if (opts.isThumb) {
        painter->save();
        painter->setRenderHints(painter->renderHints() | QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);

        auto iconStyle { IconUtils::getIconStyle(opts.rect.size().toSize().width()) };

        // 计算可用的图像绘制区域（减去阴影和边框）
        QRectF availableRect = opts.rect;
        availableRect.adjust(iconStyle.shadowRange, iconStyle.shadowRange, -iconStyle.shadowRange, -iconStyle.shadowRange);
        availableRect.adjust(iconStyle.stroke, iconStyle.stroke, -iconStyle.stroke, -iconStyle.stroke);

        // 计算缩略图的最佳显示尺寸 - 如果小于可用区域则放大铺满
        qreal scaleX = availableRect.width() / (w > 0 ? w : 1);
        qreal scaleY = availableRect.height() / (h > 0 ? h : 1);
        qreal scale = qMin(scaleX, scaleY);

        // 如果原图小于可用区域，则等比放大；否则保持原逻辑
        if (scale > 1.0) {
            w *= scale;
            h *= scale;
            // 重新计算居中位置
            x = opts.rect.x() + (opts.rect.width() - w) / 2.0;
            y = opts.rect.y() + (opts.rect.height() - h) / 2.0;
        }

        QRectF backgroundRect { x, y, w, h };
        QRectF imageRect { backgroundRect };

        // 绘制带有阴影的背景
        auto stroke { iconStyle.stroke };
        backgroundRect.adjust(-stroke, -stroke, stroke, stroke);
        const auto &originPixmap { IconUtils::renderIconBackground(backgroundRect.size(), iconStyle) };
        const auto &shadowPixmap { IconUtils::addShadowToPixmap(originPixmap, iconStyle.shadowOffset, iconStyle.shadowRange, 0.2) };
        painter->drawPixmap(backgroundRect, shadowPixmap, QRectF());
        imageRect.adjust(iconStyle.shadowRange, iconStyle.shadowRange, -iconStyle.shadowRange, -iconStyle.shadowRange);

        QPainterPath clipPath;
        auto radius { iconStyle.radius - iconStyle.stroke };
        clipPath.addRoundedRect(imageRect, radius, radius);
        painter->setClipPath(clipPath);
        painter->drawPixmap(imageRect, px, QRectF());
        painter->restore();

        return backgroundRect;
    }

    // 使用QRectF和drawPixmap的重载版本来正确处理缩放
    QRectF targetRect(x, y, w, h);
    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter->drawPixmap(targetRect, px, px.rect());
    painter->restore();

    return targetRect;
}

QRectF CanvasItemDelegate::paintEmblems(QPainter *painter, const QRectF &rect, const FileInfoPointer &info)
{
    // todo(zy) uing extend painter by registering.
    if (dpfSlotChannel->push("dfmplugin_emblem", "slot_FileEmblems_Paint", painter, rect, info).toBool()) {
        static std::once_flag printLog;
        std::call_once(printLog, []() {
            fmInfo() << "publish `kPaintEmblems` event successfully!";
        });
    }
    return rect;
}

void CanvasItemDelegate::paintBackground(QPainter *painter, const QStyleOptionViewItem &option, const QRect &iconRect) const
{
    bool isDragMode = (static_cast<QPaintDevice *>(parent()->viewport()) != painter->device());
    bool isSelected = !isDragMode && (option.state & QStyle::State_Selected) && option.showDecorationSelected;
    if (!isSelected)
        return;

    QColor backgroundColor(33, 33, 33, qRound(255 * 0.25));
    QColor borderColor(241, 241, 241, qRound(255 * 0.25));
    QRect backgroundRect = iconRect.adjusted(-kIconBackgroundMargin, -kIconBackgroundMargin, kIconBackgroundMargin, kIconBackgroundMargin);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    int radius = parent()->iconSize().width() / kIconModeBackRadiusCoefficient;
    path.addRoundedRect(backgroundRect, radius, radius);
    painter->fillPath(path, backgroundColor);

    painter->setPen(borderColor);
    painter->drawPath(path);

    painter->restore();
}

void CanvasItemDelegatePrivate::extendLayoutText(const FileInfoPointer &info, dfmbase::ElideTextLayout *layout)
{
    // extend layout
    dpfHookSequence->run("ddplugin_canvas", "hook_CanvasItemDelegate_LayoutText", info, layout);
}

void CanvasItemDelegate::paintLabel(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index, const QRect &rLabel) const
{
    // draw text
    QRectF textRect = d->availableTextRect(rLabel);

    painter->save();
    if (d->isHighlight(option)) {
        drawHighlightText(painter, option, index, textRect.toRect());
    } else {
        drawNormlText(painter, option, index, textRect.toRect());
    }
    painter->restore();
}
