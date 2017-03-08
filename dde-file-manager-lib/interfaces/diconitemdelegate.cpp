#include "diconitemdelegate.h"
#include "dfileviewhelper.h"
#include "views/fileitem.h"
#include "private/dstyleditemdelegate_p.h"

#include "dfilesystemmodel.h"
#include "app/define.h"

#include <QLabel>
#include <QPainter>
#include <QTextEdit>
#include <QLineEdit>
#include <QTextBlock>
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QAbstractItemView>

#define ICON_SPACING 16
#define ICON_MODE_RECT_RADIUS 4

class DIconItemDelegatePrivate : public DStyledItemDelegatePrivate
{
public:
    DIconItemDelegatePrivate(DIconItemDelegate *qq)
        : DStyledItemDelegatePrivate(qq) {}

    QSize textSize(const QString &text, const QFontMetrics &metrics, int lineHeight = -1) const;
    void drawText(QPainter *painter, const QRect &r, const QString &text,
                  int lineHeight = -1, QRect *br = Q_NULLPTR) const;

    QPointer<FileIconItem> expandedItem;

    mutable QHash<QString, QString> elideMap;
    mutable QHash<QString, QString> wordWrapMap;
    mutable QHash<QString, int> textHeightMap;
    mutable QModelIndex expandedIndex;
    mutable QModelIndex lastAndExpandedInde;

    QList<int> iconSizes;
    /// default icon size is 64px.
    int currentIconSizeIndex = 1;

    QColor focusTextBackgroundBorderColor = Qt::transparent;
    bool enabledTextShadow = false;
};

QSize DIconItemDelegatePrivate::textSize(const QString &text, const QFontMetrics &metrics, int lineHeight) const
{
    QString str = text;

    if (str.endsWith('\n'))
        str.chop(1);

    int max_width = 0;
    int height = 0;

    for (const QString &line : str.split('\n')) {
        max_width = qMax(metrics.width(line), max_width);

        if (lineHeight > 0)
            height += lineHeight;
        else
            height += metrics.height();
    }

    return QSize(max_width, height);
}

void DIconItemDelegatePrivate::drawText(QPainter *painter, const QRect &r, const QString &text, int lineHeight, QRect *br) const
{
    if (lineHeight <= 0)
        lineHeight = textLineHeight;

    QString str = text;

    if (str.endsWith('\n'))
        str.chop(1);

    QRect textRect = QRect(0, r.top(), 0, 0);

    for (const QString &line : str.split('\n')) {
        QRect br;

        painter->drawText(r.left(), textRect.bottom(), r.width(), lineHeight, Qt::AlignCenter, line, &br);
        textRect.setWidth(qMax(textRect.width(), br.width()));
        textRect.setBottom(textRect.bottom() + lineHeight);
    }

    if (br)
        *br = textRect;
}


DIconItemDelegate::DIconItemDelegate(DFileViewHelper *parent) :
    DStyledItemDelegate(*new DIconItemDelegatePrivate(this), parent)
{
    Q_D(DIconItemDelegate);

    d->expandedItem = new FileIconItem(parent->parent()->viewport());
    d->expandedItem->setAttribute(Qt::WA_TransparentForMouseEvents);
    d->expandedItem->setProperty("showBackground", true);
    d->expandedItem->edit->setReadOnly(true);
    d->expandedItem->canDeferredDelete = false;
    d->expandedItem->icon->setFixedSize(parent->parent()->iconSize());
    /// prevent flash when first call show()
    d->expandedItem->setFixedWidth(0);
    d->expandedItem->setBorderColor(Qt::transparent);

    d->iconSizes << 48 << 64 << 96 << 128 << 256;

    connect(parent, &DFileViewHelper::triggerEdit, this, &DIconItemDelegate::onTriggerEdit);

    parent->parent()->setIconSize(iconSizeByIconSizeLevel());
}

DIconItemDelegate::~DIconItemDelegate()
{
    Q_D(DIconItemDelegate);

    if (d->expandedItem) {
        d->expandedItem->setParent(0);
        d->expandedItem->canDeferredDelete = true;
        d->expandedItem->deleteLater();
    }
}

QString trimmedEnd(QString str)
{
    while (!str.isEmpty()) {
        switch (str.at(str.count() - 1).toLatin1()) {
        case '\t':
        case '\n':
        case '\r':
        case '\v':
        case '\f':
        case ' ':
            str.chop(1);
            break;
        default:
            return str;
        }
    }

    return str;
}

QT_BEGIN_NAMESPACE
Q_WIDGETS_EXPORT void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed = 0);
QT_END_NAMESPACE

void DIconItemDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    Q_D(const DIconItemDelegate);

    /// judgment way of the whether drag model(another way is: painter.devType() != 1)
    bool isDragMode = ((QPaintDevice*)parent()->parent()->viewport() != painter->device());
    bool isEnabled = option.state & QStyle::State_Enabled;
    bool hasFocus = option.state & QStyle::State_HasFocus;

    if (index == d->expandedIndex && !parent()->isSelected(index))
        const_cast<DIconItemDelegate*>(this)->hideNotEditingIndexWidget();

    if ((index == d->expandedIndex || index == d->editingIndex) && !isDragMode)
        return;

    if (parent()->isCut(index))
        painter->setOpacity(0.3);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    painter->setFont(opt.font);

    static QFont old_font = opt.font;

    if (old_font != opt.font) {
        if (d->expandedItem)
            d->expandedItem->setFont(opt.font);

        QWidget *editing_widget = editingIndexWidget();

        if (editing_widget)
            editing_widget->setFont(opt.font);

        const_cast<DIconItemDelegate*>(this)->updateItemSizeHint();
//        parent()->parent()->updateEditorGeometries();
    }

    old_font = opt.font;

    bool isSelected = !isDragMode && (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;
    bool isDropTarget = parent()->isDropTarget(index);

    if (isDropTarget && !isSelected) {
        QRectF rect = opt.rect;
        QPainterPath path;

        rect.moveTopLeft(QPointF(0.5, 0.5) + rect.topLeft());
        path.addRoundedRect(rect, ICON_MODE_RECT_RADIUS, ICON_MODE_RECT_RADIUS);

        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->fillPath(path, QColor(43, 167, 248, 0.50 * 255));
        painter->setPen(opt.backgroundBrush.color());
        painter->drawPath(path);
        painter->setRenderHint(QPainter::Antialiasing, false);
    }

    /// init icon geomerty

    QRect icon_rect = opt.rect;

    icon_rect.setSize(parent()->parent()->iconSize());
    icon_rect.moveLeft(opt.rect.left() + (opt.rect.width() - icon_rect.width()) / 2.0);
    icon_rect.moveTop(opt.rect.top());

    QString str = opt.text;

    /// init file name geometry

    QRect label_rect = opt.rect;

    label_rect.setTop(icon_rect.bottom() + TEXT_PADDING + ICON_MODE_ICON_SPACING);
    label_rect.setWidth(opt.rect.width() - 2 * TEXT_PADDING);
    label_rect.moveLeft(label_rect.left() + TEXT_PADDING);

    /// if has selected show all file name else show elide file name.
    bool singleSelected = parent()->selectedIndexsCount() < 2;

    if (isSelected && singleSelected) {
        const_cast<DIconItemDelegate*>(this)->hideNotEditingIndexWidget();

        int height = 0;

        /// init file name text

        if(d->wordWrapMap.contains(str)) {
            str = d->wordWrapMap.value(str);
            height = d->textHeightMap.value(str);
        } else {
            QString wordWrap_str = DFMGlobal::wordWrapText(str, label_rect.width(),
                                                           QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                           opt.font,
                                                           d->textLineHeight,
                                                           &height);

            wordWrap_str = trimmedEnd(wordWrap_str);

            d->wordWrapMap[str] = wordWrap_str;
            d->textHeightMap[wordWrap_str] = height;
            str = wordWrap_str;
        }

        if(height > label_rect.height()) {
            /// use widget(FileIconItem) show file icon and file name label.

            d->expandedIndex = index;

            setEditorData(d->expandedItem, index);
            parent()->setIndexWidget(index, d->expandedItem);

            QPalette palette = d->expandedItem->palette();

            palette.setColor(QPalette::Text, opt.palette.color(QPalette::Text));
            palette.setColor(QPalette::BrightText, opt.palette.color(QPalette::BrightText));
            palette.setColor(QPalette::Background, opt.backgroundBrush.color());
            d->expandedItem->setPalette(palette);
            d->expandedItem->updateStyleSheet();

            if (parent()->indexOfRow(index) == parent()->rowCount() - 1) {
                d->lastAndExpandedInde = index;
            }

            parent()->updateGeometries();

            return;
        }
    } else {
        /// init file name text

        if(d->elideMap.contains(str)) {
            str = d->elideMap.value(str);
        } else {
            QString elide_str = DFMGlobal::elideText(str, label_rect.size(),
                                                     QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                     opt.font,
                                                     opt.textElideMode, d->textLineHeight);

            elide_str = trimmedEnd(elide_str);

            d->elideMap[str] = elide_str;

            str = elide_str;
        }

        if (!singleSelected) {
            const_cast<DIconItemDelegate*>(this)->hideNotEditingIndexWidget();
        }
    }

    /// draw icon

    if (isSelected) {
        opt.icon.paint(painter, icon_rect, Qt::AlignCenter, QIcon::Selected);
    } else if (isDropTarget) {
        QPixmap pixmap = opt.icon.pixmap(icon_rect.size());

        QPainter p(&pixmap);

        p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        p.fillRect(QRect(QPoint(0, 0), icon_rect.size()), QColor(0, 0, 0, 255 * 0.1));
        p.end();

        painter->drawPixmap(icon_rect, pixmap);
    } else {
        opt.icon.paint(painter, icon_rect, Qt::AlignCenter, isEnabled ? QIcon::Normal : QIcon::Disabled);
    }

    /// draw file additional icon

    QList<QRect> cornerGeometryList = getCornerGeometryList(icon_rect, icon_rect.size() / 3);
    const QList<QIcon> &cornerIconList = parent()->additionalIcon(index);

    for (int i = 0; i < cornerIconList.count(); ++i) {
        cornerIconList.at(i).paint(painter, cornerGeometryList.at(i));
    }

    /// draw file name label
    if (isSelected) {
        QPainterPath path;

        QRect label_background_rect = label_rect;

        label_background_rect.setSize(d->textSize(str, painter->fontMetrics(), d->textLineHeight));
        label_background_rect.moveLeft(label_rect.left() + (label_rect.width() - label_background_rect.width()) / 2);
        label_background_rect += QMargins(TEXT_PADDING, TEXT_PADDING, TEXT_PADDING, TEXT_PADDING);

        path.addRoundedRect(label_background_rect, ICON_MODE_RECT_RADIUS, ICON_MODE_RECT_RADIUS);
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        painter->fillPath(path, opt.backgroundBrush);
        if (hasFocus && !singleSelected) {
            painter->setPen(QPen(focusTextBackgroundBorderColor(), 2));
            painter->drawPath(path);
        }
        painter->restore();
    } else {
        painter->fillRect(label_rect, Qt::transparent);
    }

    QImage text_image(label_rect.size(), QImage::Format_ARGB32_Premultiplied);
    text_image.fill(Qt::transparent);

    if (isSelected)
        painter->setPen(opt.palette.color(QPalette::BrightText));
    else
        painter->setPen(opt.palette.color(QPalette::Text));

    if (isSelected || !d->enabledTextShadow) {
        d->drawText(painter, label_rect, str, d->textLineHeight);
    } else {
        QPainter p(&text_image);
        p.setPen(painter->pen());
        d->drawText(&p, QRect(QPoint(0, 0), label_rect.size()), str, d->textLineHeight);
    }

    if (!isSelected && d->enabledTextShadow) {
        const QPixmap &text_pixmap = QPixmap::fromImage(text_image);

        qt_blurImage(text_image, 3, false);
        QPainter tmpPainter(&text_image);
        tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        tmpPainter.fillRect(text_image.rect(), opt.palette.color(QPalette::Shadow));
        painter->drawImage(label_rect.translated(0, 1), text_image);
        painter->drawPixmap(label_rect, text_pixmap);
    }

    painter->setOpacity(1);
}

QSize DIconItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &index) const
{
    Q_D(const DIconItemDelegate);

    const QSize &size = d->itemSizeHint;

    if (index.isValid() && index == d->lastAndExpandedInde) {
        d->expandedItem->setFixedWidth(size.width());

        return d->expandedItem->size();
    }

    return size;
}

QWidget *DIconItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    Q_D(const DIconItemDelegate);

    d->editingIndex = index;

    FileIconItem *item = new FileIconItem(parent);

    connect(item, &FileIconItem::inputFocusOut, this, &DIconItemDelegate::onEditWidgetFocusOut);
    connect(item, &FileIconItem::destroyed, this, [this, d] {
        d->editingIndex = QModelIndex();
    });

    return item;
}

void DIconItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QSize &icon_size = parent()->parent()->iconSize();

    editor->move(option.rect.topLeft());
    editor->setFixedWidth(option.rect.width());
    editor->setMinimumHeight(option.rect.height());

    FileIconItem *item = qobject_cast<FileIconItem*>(editor);

    if(!item)
        return;

    if(icon_size.width() != item->icon->size().width()) {
        QStyleOptionViewItem opt;

        initStyleOption(&opt, index);

        QPixmap pixmap = opt.icon.pixmap(icon_size, QIcon::Selected);
        QPainter painter(&pixmap);

        /// draw file additional icon

        QList<QRect> cornerGeometryList = getCornerGeometryList(QRect(QPoint(0, 0), icon_size), icon_size / 3);
        const QList<QIcon> &cornerIconList = parent()->additionalIcon(index);

        for (int i = 0; i < cornerIconList.count(); ++i) {
            cornerIconList.at(i).paint(&painter, cornerGeometryList.at(i));
        }

        item->icon->setFixedSize(icon_size);
        item->icon->setPixmap(pixmap);
    }
}

void DIconItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    Q_D(const DIconItemDelegate);

    FileIconItem *item = qobject_cast<FileIconItem*>(editor);

    if(!item)
        return;

    QStyleOptionViewItem opt;

    initStyleOption(&opt, index);

    const QSize &icon_size = parent()->parent()->iconSize();
    QPixmap pixmap = opt.icon.pixmap(icon_size, QIcon::Selected);
    QPainter painter(&pixmap);

    /// draw file additional icon

    QList<QRect> cornerGeometryList = getCornerGeometryList(QRect(QPoint(0, 0), icon_size), icon_size / 3);
    const QList<QIcon> &cornerIconList = parent()->additionalIcon(index);

    for (int i = 0; i < cornerIconList.count(); ++i) {
        cornerIconList.at(i).paint(&painter, cornerGeometryList.at(i));
    }

    item->icon->setPixmap(pixmap);
    item->edit->setPlainText(index.data(DFileSystemModel::FileNameRole).toString());
    item->edit->setAlignment(Qt::AlignHCenter);
    item->edit->document()->setTextWidth(d->itemSizeHint.width());
    item->setOpacity(parent()->isCut(index) ? 0.3 : 1);

    if(item->edit->isReadOnly())
        return;

    const QString &selectionWhenEditing = parent()->baseName(index);
    int endPos = selectionWhenEditing.isEmpty() ? -1 : selectionWhenEditing.length();

    if (endPos == -1) {
        item->edit->selectAll();
    } else {
        QTextCursor cursor = item->edit->textCursor();

        cursor.setPosition(0);
        cursor.setPosition(endPos, QTextCursor::KeepAnchor);

        item->edit->setTextCursor(cursor);
    }
}

QList<QRect> DIconItemDelegate::paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_D(const DIconItemDelegate);

    QList<QRect> geometrys;

    if (index == d->expandedIndex) {
        QRect geometry = d->expandedItem->icon->geometry();

        geometry.moveTopLeft(geometry.topLeft() + d->expandedItem->pos());

        geometrys << geometry;

        geometry = d->expandedItem->edit->geometry();
        geometry.moveTopLeft(geometry.topLeft() + d->expandedItem->pos());
        geometry.setTop(d->expandedItem->icon->y() + d->expandedItem->icon->height() + d->expandedItem->y());

        geometrys << geometry;

        return geometrys;
    }

    /// init icon geomerty

    QRect icon_rect = option.rect;

    icon_rect.setSize(parent()->parent()->iconSize());
    icon_rect.moveCenter(option.rect.center());
    icon_rect.moveTop(option.rect.top());

    geometrys << icon_rect;

    QString str = index.data(Qt::DisplayRole).toString();

    if(str.isEmpty()) {
        return geometrys;
    }

    /// init file name geometry

    QRect label_rect = option.rect;

    label_rect.setTop(icon_rect.bottom() + TEXT_PADDING + ICON_MODE_ICON_SPACING);

    QStyleOptionViewItem opt = option;
//    initStyleOption(&opt, index);

    bool isSelected = parent()->isSelected(index) && opt.showDecorationSelected;
    /// if has selected show all file name else show elide file name.
    bool singleSelected = parent()->selectedIndexsCount() < 2;

    if (isSelected && singleSelected) {
        int height = 0;

        if (d->wordWrapMap.contains(str)) {
            str = d->wordWrapMap.value(str);
            height = d->textHeightMap.value(str);
        } else {
            QString wordWrap_str = DFMGlobal::wordWrapText(str, label_rect.width(),
                                                           QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                           opt.font,
                                                           d->textLineHeight,
                                                           &height);

            wordWrap_str = trimmedEnd(wordWrap_str);

            d->wordWrapMap[str] = wordWrap_str;
            d->textHeightMap[wordWrap_str] = height;
            str = wordWrap_str;
        }
    } else {
        if (d->elideMap.contains(str)) {
            str = d->elideMap.value(str);
        } else {
            QString elide_str = DFMGlobal::elideText(str, label_rect.size(), QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                     opt.font, opt.textElideMode, d->textLineHeight);

            d->elideMap[str] = elide_str;
            str = elide_str;
        }
    }

    /// draw icon and file name label

    label_rect = option.fontMetrics.boundingRect(label_rect, Qt::AlignHCenter, str);
    label_rect.setTop(icon_rect.bottom());

    geometrys << label_rect;

    return geometrys;
}

QModelIndexList DIconItemDelegate::hasWidgetIndexs() const
{
    const QModelIndex &index = expandedIndex();

    if (!index.isValid())
        return DStyledItemDelegate::hasWidgetIndexs();

    return DStyledItemDelegate::hasWidgetIndexs() << index;
}

void DIconItemDelegate::hideNotEditingIndexWidget()
{
    Q_D(DIconItemDelegate);

    if (d->expandedIndex.isValid()) {
        parent()->setIndexWidget(d->expandedIndex, 0);
        d->expandedItem->hide();
        d->expandedIndex = QModelIndex();
        d->lastAndExpandedInde = QModelIndex();
    }
}

QModelIndex DIconItemDelegate::expandedIndex() const
{
    Q_D(const DIconItemDelegate);

    return d->expandedIndex;
}

FileIconItem *DIconItemDelegate::expandedIndexWidget() const
{
    Q_D(const DIconItemDelegate);

    return d->expandedItem;
}

int DIconItemDelegate::iconSizeLevel() const
{
    Q_D(const DIconItemDelegate);

    return d->currentIconSizeIndex;
}

int DIconItemDelegate::minimumIconSizeLevel() const
{
    return 0;
}

int DIconItemDelegate::maximumIconSizeLevel() const
{
    Q_D(const DIconItemDelegate);

    return d->iconSizes.count() - 1;
}

/*!
 * \brief Return current icon level if icon can increase; otherwise return -1.
 * \return
 */
int DIconItemDelegate::increaseIcon()
{
    Q_D(const DIconItemDelegate);

    return setIconSizeByIconSizeLevel(d->currentIconSizeIndex + 1);
}

/*!
 * \brief Return current icon level if icon can decrease; otherwise return -1.
 * \return
 */
int DIconItemDelegate::decreaseIcon()
{
    Q_D(const DIconItemDelegate);

    return setIconSizeByIconSizeLevel(d->currentIconSizeIndex - 1);
}

/*!
 * \brief Return current icon level if level is vaild; otherwise return -1.
 * \param level
 * \return
 */
int DIconItemDelegate::setIconSizeByIconSizeLevel(int level)
{
    Q_D(DIconItemDelegate);

    if (level == d->currentIconSizeIndex)
        return level;

    if (level >= minimumIconSizeLevel() && level <= maximumIconSizeLevel()) {
        d->currentIconSizeIndex = level;

        parent()->parent()->setIconSize(iconSizeByIconSizeLevel());

        return d->currentIconSizeIndex;
    }

    return -1;
}

void DIconItemDelegate::updateItemSizeHint()
{
    Q_D(DIconItemDelegate);

    d->elideMap.clear();
    d->wordWrapMap.clear();
    d->textHeightMap.clear();
    d->textLineHeight = parent()->parent()->fontMetrics().height();

    int width = parent()->parent()->iconSize().width() * 1.8;

    d->itemSizeHint = QSize(width, parent()->parent()->iconSize().height() + 2 * TEXT_PADDING  + ICON_MODE_ICON_SPACING + 3 * d->textLineHeight);
}

QColor DIconItemDelegate::focusTextBackgroundBorderColor() const
{
    Q_D(const DIconItemDelegate);

    return d->focusTextBackgroundBorderColor;
}

bool DIconItemDelegate::enabledTextShadow() const
{
    Q_D(const DIconItemDelegate);

    return d->enabledTextShadow;
}

void DIconItemDelegate::setFocusTextBackgroundBorderColor(QColor focusTextBackgroundBorderColor)
{
    Q_D(DIconItemDelegate);

    d->focusTextBackgroundBorderColor = focusTextBackgroundBorderColor;

//    if (d->expandedItem)
    //        d->expandedItem->setBorderColor(focusTextBackgroundBorderColor);
}

void DIconItemDelegate::setEnabledTextShadow(bool enabledTextShadow)
{
    Q_D(DIconItemDelegate);

    d->enabledTextShadow = enabledTextShadow;
}

bool DIconItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *e = static_cast<QKeyEvent*>(event);

        if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
            e->accept();

            return true;
        }
    } /*else if (event->type() == QEvent::FocusOut) {
        onEditWidgetFocusOut();

        return true;
    }*/

    return QStyledItemDelegate::eventFilter(object, event);
}

void DIconItemDelegate::onEditWidgetFocusOut()
{
    if (qApp->focusWidget() && qApp->focusWidget()->window() == parent()->parent()->window()
            && qApp->focusWidget() != parent()->parent()) {

        hideAllIIndexWidget();
    }
}

void DIconItemDelegate::onTriggerEdit(const QModelIndex &index)
{
    Q_D(DIconItemDelegate);

    if(index == d->expandedIndex) {
        parent()->setIndexWidget(index, 0);
        d->expandedItem->hide();
        d->expandedIndex = QModelIndex();
        d->lastAndExpandedInde = QModelIndex();
        parent()->parent()->edit(index);
    }
}

QSize DIconItemDelegate::iconSizeByIconSizeLevel() const
{
    Q_D(const DIconItemDelegate);

    int size = d->iconSizes.at(d->currentIconSizeIndex);

    return QSize(size, size);
}
