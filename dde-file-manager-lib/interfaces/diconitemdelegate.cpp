#include "diconitemdelegate.h"
#include "dfileviewhelper.h"
#include "views/fileitem.h"
#include "views/deditorwidgetmenu.h"
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
#define SELECTED_BACKGROUND_COLOR "#2da6f7"
#define TEXT_COLOR "#303030"

class DIconItemDelegatePrivate : public DStyledItemDelegatePrivate
{
public:
    DIconItemDelegatePrivate(DIconItemDelegate *qq)
        : DStyledItemDelegatePrivate(qq) {}

    QPointer<FileIconItem> expandedItem;

    mutable QHash<QString, QString> elideMap;
    mutable QHash<QString, QString> wordWrapMap;
    mutable QHash<QString, int> textHeightMap;
    mutable QHash<QString, QTextDocument*> documentMap;
    mutable QModelIndex expandedIndex;
    mutable QModelIndex lastAndExpandedInde;

    QList<int> iconSizes;
    /// default icon size is 64px.
    int currentIconSizeIndex = 1;
};

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

    d->iconSizes << 48 << 64 << 96 << 128 << 256;

    connect(parent, &DFileViewHelper::triggerEdit, this, &DIconItemDelegate::onTriggerEdit);
    connect(parent->parent(), &QAbstractItemView::iconSizeChanged,
            this, &DIconItemDelegate::onIconSizeChanged);

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

void DIconItemDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    Q_D(const DIconItemDelegate);

    /// judgment way of the whether drag model(another way is: painter.devType() != 1)
    bool isDragMode = ((QPaintDevice*)parent()->parent()->viewport() != painter->device());
    bool isEnabled = option.state & QStyle::State_Enabled;

    painter->setPen(isEnabled ? TEXT_COLOR : DISABLE_LABEL_COLOR);

    if((index == d->expandedIndex || index == d->editingIndex) && !isDragMode)
        return;

    if (parent()->isCut(index))
        painter->setOpacity(0.3);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    bool isSelected = !isDragMode && (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;
    bool isDropTarget = parent()->isDropTarget(index);

    if (isDropTarget && !isSelected) {
        QPen pen;
        QRectF rect = opt.rect;
        QPainterPath path;

        rect.moveTopLeft(QPointF(0.5, 0.5) + rect.topLeft());

        pen.setColor(SELECTED_BACKGROUND_COLOR);

        path.addRoundedRect(rect, ICON_MODE_RECT_RADIUS, ICON_MODE_RECT_RADIUS);

        painter->setPen(pen);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->fillPath(path, QColor(43, 167, 248, 0.50 * 255));
        painter->drawPath(path);
        painter->setRenderHint(QPainter::Antialiasing, false);
        painter->setPen(Qt::black);
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

            if (parent()->indexOfRow(index) == parent()->rowCount() - 1) {
                d->lastAndExpandedInde = index;
            }

            parent()->updateGeometries();
        }
    } else {
        /// init file name text

        if(d->elideMap.contains(str)) {
            str = d->elideMap.value(str);
        } else {
            QString elide_str = DFMGlobal::elideText(str, label_rect.size(),
                                                  painter->fontMetrics(),
                                                  QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                  opt.textElideMode);

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
        painter->setPen(Qt::white);
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

    if(str.indexOf("\n") >=0 && !str.endsWith("\n")) {
        QTextDocument *doc = d->documentMap.value(str);

        if(!doc) {
            doc = new QTextDocument(str, const_cast<DIconItemDelegate*>(this));

            QTextCursor cursor(doc);
            QTextOption text_option(Qt::AlignHCenter);

            text_option.setWrapMode(QTextOption::NoWrap);
//            doc->setDefaultFont(painter->font());
            doc->setDefaultTextOption(text_option);
            doc->setTextWidth(label_rect.width());
            cursor.movePosition(QTextCursor::Start);

            do {
                QTextBlockFormat format = cursor.blockFormat();

                format.setLineHeight(TEXT_LINE_HEIGHT, QTextBlockFormat::FixedHeight);
                cursor.setBlockFormat(format);
            } while (cursor.movePosition(QTextCursor::NextBlock));

            d->documentMap[str] = doc;
        }

        QAbstractTextDocumentLayout::PaintContext ctx;

        ctx.palette.setColor(QPalette::Text, painter->pen().color());

        if (isSelected) {
            QRect rect = opt.rect;

            rect.moveTop(label_rect.top() - TEXT_PADDING);
            rect.setHeight(doc->size().height());

            QPainterPath path;

            path.addRoundedRect(rect, ICON_MODE_RECT_RADIUS, ICON_MODE_RECT_RADIUS);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->fillPath(path, QColor(SELECTED_BACKGROUND_COLOR));
            painter->restore();
        } else {
            painter->fillRect(label_rect, Qt::transparent);
        }

        painter->save();
        painter->translate(label_rect.left(), label_rect.top() - TEXT_PADDING);
        doc->documentLayout()->draw(painter, ctx);
        painter->restore();
    } else {
        if(isSelected) {
            QRect rect;

            painter->drawText(label_rect, Qt::AlignHCenter, str, &rect);
            rect += QMargins(TEXT_PADDING, TEXT_PADDING, TEXT_PADDING, TEXT_PADDING);

            QPainterPath path;

            path.addRoundedRect(rect, ICON_MODE_RECT_RADIUS, ICON_MODE_RECT_RADIUS);
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing);
            painter->fillPath(path, QColor(SELECTED_BACKGROUND_COLOR));
            painter->restore();
        } else {
            painter->fillRect(label_rect, Qt::transparent);
        }

        painter->drawText(label_rect, Qt::AlignHCenter, str);
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

    Q_UNUSED(new DEditorWidgetMenu(item->edit))

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

    QList<QRect> geomertys;

    /// init icon geomerty

    QRect icon_rect = option.rect;

    icon_rect.setSize(parent()->parent()->iconSize());
    icon_rect.moveCenter(option.rect.center());
    icon_rect.moveTop(option.rect.top());

    geomertys << icon_rect;

    QString str = index.data(Qt::DisplayRole).toString();

    if(str.isEmpty()) {
        return geomertys;
    }

    /// init file name geometry

    QRect label_rect = option.rect;

    label_rect.setTop(icon_rect.bottom() + TEXT_PADDING  + ICON_MODE_ICON_SPACING);

    /// init file name text

    if(d->elideMap.contains(str)) {
        str = d->elideMap.value(str);
    } else {
        QString elide_str = DFMGlobal::elideText(str, label_rect.size(),
                                              option.fontMetrics,
                                              QTextOption::WrapAtWordBoundaryOrAnywhere,
                                              option.textElideMode);

        d->elideMap[str] = elide_str;

        str = elide_str;
    }

    /// draw icon and file name label

    geomertys << option.fontMetrics.boundingRect(label_rect, Qt::AlignHCenter, str);
    geomertys.last().setTop(icon_rect.bottom());

    return geomertys;
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

void DIconItemDelegate::onIconSizeChanged()
{
    Q_D(DIconItemDelegate);

    d->elideMap.clear();
    d->wordWrapMap.clear();
    d->textHeightMap.clear();

    int width = parent()->parent()->iconSize().width() * 1.8;

    d->itemSizeHint = QSize(width, parent()->parent()->iconSize().height() + 2 * TEXT_PADDING  + ICON_MODE_ICON_SPACING + 3 * TEXT_LINE_HEIGHT);
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
