/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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

#include "dfmlistitemdelegate.h"
//#include "dfileviewhelper.h"
#include "base/define.h"
#include "base/dfmglobal.h"

#include "dfmfileviewmodel.h"
//#include "dfilesystemmodel.h"
#include "dfmstyleditemdelegate_p.h"
//#include "dfmapplication.h"
//#include "controllers/vaultcontroller.h"
#include "widgets/dfmfileview/dfmfileviewitem.h"
#include "widgets/dfmfileview/dfmfileview.h"
#include <DListView>

#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QAbstractItemView>
#include <QLineEdit>
#include <QApplication>
#include <QToolTip>
#include <QPainterPath>

#include <DPalette>
#include <DApplicationHelper>

#define ICON_SPACING 16
#define LIST_MODE_RECT_RADIUS 8
#define LIST_EDITER_HEIGHT 22
#define LIST_MODE_EDITOR_LEFT_PADDING -3
#define LIST_VIEW_ICON_SIZE 24

DWIDGET_USE_NAMESPACE

class DFMListItemDelegatePrivate : public DFMStyledItemDelegatePrivate
{
public:
    explicit DFMListItemDelegatePrivate(DFMListItemDelegate *qq)
        : DFMStyledItemDelegatePrivate(qq) {}
};

DFMListItemDelegate::DFMListItemDelegate(DListView *parent) :
    DFMStyledItemDelegate(parent)
{
    parent->setIconSize(QSize(LIST_VIEW_ICON_SIZE, LIST_VIEW_ICON_SIZE));
}

void DFMListItemDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
{
    Q_D(const DFMListItemDelegate);
    //根据UI要求 交替亮度为深色主题

    painter->save();//保存之前的绘制样式

    //反走样抗锯齿
    painter->setRenderHints(QPainter::Antialiasing
                           |QPainter::TextAntialiasing
                           |QPainter::SmoothPixmapTransform);
    //绘制新的背景交替
    DPalette pl(DApplicationHelper::instance()->palette(option.widget));
    QColor baseColor = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);
    //默认调整色保持背板颜色
    QColor adjustItemAlterColor = baseColor;//交替色
    QColor adjustHoverItemColor = baseColor;//hover色
    if (option.widget) {
        DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(baseColor);
        if (ct == DGuiApplicationHelper::DarkType) {
            adjustItemAlterColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +5);
            adjustHoverItemColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +10);
        }

        if (ct == DGuiApplicationHelper::LightType) {
            adjustItemAlterColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +5);
            adjustHoverItemColor = DGuiApplicationHelper::adjustColor(baseColor, 0, 0, 0, 0, 0, 0, +10);
        }
    }

    //获取item范围
    auto itemRect = dynamic_cast<DListView*>(parent())->visualRect(index);
    //左右间隔10 px UI设计要求 选中与交替渐变背景
    QRect dstRect(itemRect.x() + LIST_MODE_LEFT_MARGIN,
                  itemRect.y(),
                  itemRect.width() - (LIST_MODE_LEFT_MARGIN + LIST_MODE_RIGHT_MARGIN),
                  itemRect.height());

    //取模设置当前的交替变化
    if (index.row() % 2 == 1) {
        auto blockColorBrush = parent()->palette().background();
        QPainterPath path;
        path.addRoundedRect(dstRect, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);//圆角8 UI要求
        painter->fillPath(path, adjustItemAlterColor);
    } else {
        painter->setBrush(baseColor);
    }

    //设置hover高亮
    if (option.state & QStyle::StateFlag::State_MouseOver) {
        QPainterPath path;
        path.addRoundedRect(dstRect, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);
        painter->fillPath(path, adjustHoverItemColor);
    }

    painter->restore(); //恢复之前的绘制，防止在此逻辑前的绘制丢失

    /// judgment way of the whether drag model(another way is: painter.devType() != 1)
    bool isDragMode = false/* qobject_cast<QPaintDevice *>(parent()->viewport()) != painter->device()*/;
    bool isDropTarget = false/*parent()->isDropTarget(index)*/;
    bool isEnabled = option.state & QStyle::State_Enabled;

//    if (parent()->isTransparent(index)) {
//        painter->setOpacity(0.3);
//    }

//    const QList<int> &columnRoleList = parent()->columnRoleList();

    int column_x = 0;

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    painter->setFont(opt.font);

    static QFont old_font = opt.font;

//    if (old_font != opt.font) {
//        QWidget *editing_widget = editingIndexWidget();

//        if (editing_widget) {
//            editing_widget->setFont(opt.font);
//        }

//        const_cast<DFMListItemDelegate *>(this)->updateItemSizeHint();
//    }

    old_font = opt.font;

    opt.rect += QMargins(-LIST_MODE_LEFT_MARGIN, 0, -LIST_MODE_RIGHT_MARGIN, 0);

    /// draw background
    //列表拖拽时要绘制活动色
    bool drawBackground =/* !isDragMode &&*/ (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;

    QPalette::ColorGroup cg = (option.widget ? option.widget->isEnabled() : (option.state & QStyle::State_Enabled))
                              ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
        cg = QPalette::Inactive;

    bool isSelected = (opt.state & QStyle::State_Selected) && opt.showDecorationSelected;
    QPalette::ColorRole colorRole = QPalette::Background;
    if ((isSelected || isDropTarget)) {
        colorRole = QPalette::Highlight;
    }

    if (drawBackground) {
        QPainterPath path;
        path.addRoundedRect(opt.rect, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);
        painter->save();
        painter->setOpacity(1);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->fillPath(path, option.palette.color(cg, colorRole));
        painter->restore();
    } else if (isDropTarget) {
        QRectF rect = opt.rect;
        QPainterPath path;
        rect += QMarginsF(-0.5, -0.5, -0.5, -0.5);
        path.addRoundedRect(rect, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->fillPath(path, option.palette.color(cg, colorRole));
        painter->setRenderHint(QPainter::Antialiasing, false);
    }

    opt.rect.setLeft(opt.rect.left() + LEFT_PADDING);
    opt.rect.setRight(opt.rect.right() - RIGHT_PADDING);

    /// draw icon

    QRect icon_rect = opt.rect;

    icon_rect.setSize(parent()->iconSize());
    icon_rect.moveTop(icon_rect.top() + (opt.rect.bottom() - icon_rect.bottom()) / 2);

    if (isDropTarget) {
        QPixmap pixmap = opt.icon.pixmap(icon_rect.size());

        QPainter p(&pixmap);

        p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        p.fillRect(QRect(QPoint(0, 0), icon_rect.size()), QColor(0, 0, 0, 255 * 0.1));
        p.end();

        painter->drawPixmap(icon_rect, pixmap);
    } else {
        paintIcon(painter, opt.icon, icon_rect, Qt::AlignCenter, isEnabled ? QIcon::Normal : QIcon::Disabled);
    }

    /// draw file additional icon

//    QList<QRectF> cornerGeometryList = getCornerGeometryList(icon_rect, icon_rect.size() / 2);
//    const QList<QIcon> cornerIconList;/* = parent()->additionalIcon(index)*/;

//    for (int i = 0; i < cornerIconList.count(); ++i) {
//        if (cornerIconList.at(i).isNull()) {
//            continue;
//        }
//        cornerIconList.at(i).paint(painter, cornerGeometryList.at(i).toRect());
//    }

    column_x = icon_rect.right() + ICON_SPACING;

    QRect rect = opt.rect;

    rect.setLeft(column_x);

    qint32 totalWidth = opt.rect.width() - column_x;
    //动态计算每个列的大小
    const QList<int> &columnRoleList = index.data(DFMFileViewItem::ItemColumListRole).value<QList<int>>();
    if (columnRoleList.isEmpty())
        return;
    QList<int> widthScaleList = index.data(DFMFileViewItem::ItemColumWidthScaleListRole).value<QList<int>>();
    while (widthScaleList.size() < columnRoleList.size()) {
        widthScaleList << 1;
    }
    int totalWidthScale = 0;
    for (auto scale : widthScaleList) {
        totalWidthScale += scale;
    }


    column_x += widthScaleList.at(0)*1.0/totalWidthScale*totalWidth/* + parent()->columnWidth(0) - 1 - parent()->viewportMargins().left()*/;

    rect.setRight(qMin(column_x, opt.rect.right()));
//    static QList<int> columnRoleList;
//    columnRoleList << DFMFileViewItem::ItemNameRole
//                   << DFMFileViewItem::ItemFileLastModifiedRole
//                   << DFMFileViewItem::ItemFileSizeRole
//                   << DFMFileViewItem::ItemFileMimeTypeRole;


    int role = columnRoleList.at(0);

    if (index != d->editingIndex/* || (role != DFMFileViewItem::ItemNameRole && role != DFMFileViewItem::ItemNameRole)*/) {
        //get colors
//        const QVariantHash &ep = index.data(DFMFileViewItem::ItemNameRole).toHash();
//        const QList<QColor> &colors = qvariant_cast<QList<QColor>>(ep.value("colored"));

//        if (!colors.isEmpty()) {
//            qreal diameter = 10;
//            QRectF tag_boundingRect(0, 0, (colors.size() + 1) * diameter / 2, diameter);

//            tag_boundingRect.moveCenter(rect.center());
//            tag_boundingRect.moveRight(rect.right());

//            paintCircleList(painter, tag_boundingRect, diameter, colors,
//                            (drawBackground || colors.size() > 1) ? Qt::white : QColor(0, 0, 0, 25));
//            rect.setRight(tag_boundingRect.left() - ICON_SPACING);
//        }

        /// draw file name label
        const QVariant &data = index.data(role);
        painter->setPen(opt.palette.color(drawBackground ? QPalette::BrightText : QPalette::Text));
        if (data.canConvert<QString>()) {
            QString file_name;

            do {
//                if (role != DFMFileViewItem::ItemNameRole /*&& role != DFMFileViewItem::ItemNameRole*/) {
//                    break;
//                }

//                if (role == DFMFileViewItem::ItemNameRole) {
//                    const auto fileName = index.data(DFMFileViewItem::ItemNameRole);
//                    const auto file_display_name = index.data(DFMFileViewItem::ItemNameRole);
//                    qInfo() << fileName << file_display_name;
//                    if (fileName != file_display_name) {
//                        break;
//                    }
//                }

                const QString &suffix = "." + index.data(DFMFileViewItem::ItemFileSuffixRole).toString();
//                qInfo() << suffix << QSize(rect.width() - opt.fontMetrics.width(suffix), rect.height());
                if (suffix == ".") {
                    break;
                }

//                file_name = DFMGlobal::elideText(index.data(DFMFileViewItem::ItemNameRole).toString().remove('\n'),
//                                                 QSize(rect.width() - opt.fontMetrics.width(suffix), rect.height()), QTextOption::WrapAtWordBoundaryOrAnywhere,
//                                                 opt.font, Qt::ElideRight,
//                                                 d->textLineHeight);
//                file_name.append(suffix);
            } while (false);

            if (file_name.isEmpty()) {
                file_name = DFMGlobal::elideText(index.data(role).toString().remove('\n'),
                                                 rect.size(), QTextOption::WrapAtWordBoundaryOrAnywhere,
                                                 opt.font, Qt::ElideRight,
                                                 d->textLineHeight);
            }

            painter->drawText(rect, Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt()), file_name);
        } else {
            drawNotStringData(opt, d->textLineHeight, rect, data, drawBackground, painter, 0);
        }
    }

    if (isDragMode) {
        return;
    }

    const DFMFileViewModel *model = qobject_cast<const DFMFileViewModel *>(index.model());
    if (isSelected)
        painter->setPen(opt.palette.color(QPalette::Active, QPalette::HighlightedText));
    else
        painter->setPen(opt.palette.color(QPalette::Inactive, QPalette::Text));

    for (int i = 1; i < columnRoleList.count(); ++i) {
        int column_width = widthScaleList.at(i)*1.0/totalWidthScale*totalWidth/*qobject_cast<DFMFileView *>( parent())->columnWidth(i)*/;

        if (column_width <= 0) {
            continue;
        }

        QRect rec = opt.rect;

        rec.setLeft(column_x + COLUMU_PADDING);

        if (rec.left() >= rec.right()) {
            break;
        }

        column_x += column_width;

        rec.setRight(qMin(column_x, opt.rect.right()));

        int rol = columnRoleList.at(i);

        QModelIndex tmp_index /*= model->createIndex(index.row(), 4, index.internalId())*/;

        const QVariant &data = index.data(rol);

        if (data.canConvert<QString>()) {
            QString strInfo(index.data(rol).toString());
            // 如果是文件路径项
            if(rol == DFMFileViewItem::ItemFilePathRole) {
//                // 如果是保险箱路径,则不显示真实路径
//                if(VaultController::isVaultFile(strInfo))
//                    strInfo = VaultController::localPathToVirtualPath(index.data(rol).toString());
            }
            const QString &text = DFMGlobal::elideText(strInfo, rec.size(),
                                                       QTextOption::NoWrap, opt.font,
                                                       Qt::ElideRight, d->textLineHeight);

            painter->drawText(rec, Qt::Alignment(tmp_index.data(Qt::TextAlignmentRole).toInt()), text);
        } else {
            drawNotStringData(opt, d->textLineHeight, rec, data, drawBackground, painter, i);
        }
    }

//    if (isDropTarget && !drawBackground) {
//        QPen pen;
//        QRectF rec = opt.rect;

//        rec += QMarginsF(-0.5 + LEFT_PADDING, -0.5, -0.5 + RIGHT_PADDING, -0.5);

//        pen.setColor(opt.backgroundBrush.color());

//        painter->setPen(pen);
//        painter->setRenderHint(QPainter::Antialiasing, true);
//        painter->drawRoundedRect(rec, LIST_MODE_RECT_RADIUS, LIST_MODE_RECT_RADIUS);
//        painter->setRenderHint(QPainter::Antialiasing, false);
//        painter->setPen(Qt::black);
//    }

    painter->setOpacity(1);
}


void DFMListItemDelegate::drawNotStringData(const QStyleOptionViewItem &opt, int lineHeight, const QRect &rect, const QVariant &data,
                                          bool drawBackground, QPainter *painter, const int &column) const
{

    const DFMFileViewModel *model = qobject_cast<const DFMFileViewModel *>(parent()->model());
    const DAbstractFileInfoPointer &fileInfo = DFMInfoFactory::instance().create<DFMLocalFileInfo>(const_cast<DFMFileViewModel *>(model)->rootUrl());

//    int sortRole = model->sortRole();
    static QList<int> userColumnRoles {
        DFMFileViewItem::ItemFileLastModifiedRole,
        DFMFileViewItem::ItemFileSizeRole,
        DFMFileViewItem::ItemFileMimeTypeRole
    };
    int sortRoleIndexByColumnChildren = 0;/*userColumnRoles.indexOf(sortRole);*/

    const QColor &active_color = opt.palette.color(QPalette::Active, drawBackground ? QPalette::BrightText : QPalette::Text);
    const QColor &normal_color = opt.palette.color(QPalette::Inactive, drawBackground ? QPalette::BrightText : QPalette::Text);

    if (data.canConvert<QPair<QString, QString>>()) {
        QPair<QString, QString> name_path = qvariant_cast<QPair<QString, QString>>(data);

        const QString &file_name = DFMGlobal::elideText(name_path.first.remove('\n'),
                                                        QSize(rect.width(), rect.height() / 2), QTextOption::NoWrap,
                                                        opt.font, Qt::ElideRight,
                                                        lineHeight);
        painter->setPen(sortRoleIndexByColumnChildren == 0 ? active_color : normal_color);
        painter->drawText(rect.adjusted(0, 0, 0, -rect.height() / 2), Qt::AlignBottom, file_name);

        const QString &file_path = DFMGlobal::elideText(name_path.second.remove('\n'),
                                                        QSize(rect.width(), rect.height() / 2), QTextOption::NoWrap,
                                                        opt.font, Qt::ElideRight,
                                                        lineHeight);

        painter->setPen(sortRoleIndexByColumnChildren == 1 ? active_color : normal_color);
        painter->drawText(rect.adjusted(0, rect.height() / 2, 0, 0), Qt::AlignTop, file_path);
    } else if (data.canConvert<QPair<QString, QPair<QString, QString>>>()) {
        QRect new_rect = rect;

        const QPair<QString, QPair<QString, QString>> &dst = qvariant_cast<QPair<QString, QPair<QString, QString>>>(data);

        const QString &date = DFMGlobal::elideText(dst.first, QSize(rect.width(), rect.height() / 2),
                                                   QTextOption::NoWrap, opt.font,
                                                   Qt::ElideRight, lineHeight);

        painter->setPen(sortRoleIndexByColumnChildren == 0 ? active_color : normal_color);
        painter->drawText(new_rect.adjusted(0, 0, 0, -new_rect.height() / 2), Qt::AlignBottom, date, &new_rect);

        new_rect = QRect(rect.left(), rect.top(), new_rect.width(), rect.height());

        const QString &size = DFMGlobal::elideText(dst.second.first, QSize(new_rect.width() / 2, new_rect.height() / 2),
                                                   QTextOption::NoWrap, opt.font,
                                                   Qt::ElideRight, lineHeight);

        painter->setPen(sortRoleIndexByColumnChildren == 1 ? active_color : normal_color);
        painter->drawText(new_rect.adjusted(0, new_rect.height() / 2, 0, 0), Qt::AlignTop | Qt::AlignLeft, size);

        const QString &type = DFMGlobal::elideText(dst.second.second, QSize(new_rect.width() / 2, new_rect.height() / 2),
                                                   QTextOption::NoWrap, opt.font,
                                                   Qt::ElideLeft, lineHeight);
        painter->setPen(sortRoleIndexByColumnChildren == 2 ? active_color : normal_color);
        painter->drawText(new_rect.adjusted(0, new_rect.height() / 2, 0, 0), Qt::AlignTop | Qt::AlignRight, type);
    }
}


QSize DFMListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
//    if (!parent()) {
//        return DFMStyledItemDelegate::sizeHint(option, index);
//    }
//    const DAbstractFileInfoPointer &file_info = qobject_cast<DFMFileViewModel*>(parent()->model())->fileInfo(index);

//    if (!file_info) {
//        return DFMStyledItemDelegate::sizeHint(option, index);
//    }

//    Q_D(const DFMListItemDelegate);

//    return QSize(d->itemSizeHint.width(), qMax(option.fontMetrics.height() * 2 + 10, d->itemSizeHint.height()));
}

QWidget *DFMListItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_D(const DFMListItemDelegate);

    d->editingIndex = index;

    QLineEdit *edit = new QLineEdit(parent);

    const DAbstractFileInfoPointer &file_info = qobject_cast<DFMFileViewModel*>(this->parent())->fileInfo(index);
    if (file_info->url().scheme() == "search") {
        edit->setFixedHeight(LIST_EDITER_HEIGHT * 2 - 10);
    } else {
        edit->setFixedHeight(LIST_EDITER_HEIGHT);
    }
    edit->setObjectName("DListItemDelegate_Editor");

    connect(edit, &QLineEdit::destroyed, this, [this, d] {
        Q_UNUSED(this);
        d->editingIndex = QModelIndex();
    });

    connect(edit, &QLineEdit::textChanged, this, [this, edit, d] {
        //在此处处理的逻辑是因为默认QAbstractItemView的QLineEdit重命名会被SelectAll
        //const 防止被改变
        const QString srcText = edit->text();
        //得到处理之后的文件名称
        QString dstText = DFMGlobal::preprocessingFileName(srcText);

        //超出长度将不再被支持输入获取当前
        bool donot_show_suffix= false;/*{ DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowedFileSuffixOnRename).toBool() };*/

        //获取当前编辑框支持的最大文字长度
        int textMaxLen = INT_MAX;
        if (donot_show_suffix) {
            const QString &suffix = d->editingIndex.data(DFMFileViewItem::ItemFileSuffixRole).toString();
            edit->setProperty("_d_whether_show_suffix", suffix);
            textMaxLen = MAX_FILE_NAME_CHAR_COUNT - suffix.toLocal8Bit().size() - (suffix.isEmpty() ? 0 : 1);
        } else {
            textMaxLen = MAX_FILE_NAME_CHAR_COUNT;
        }

        //如果存在非法字符且更改了当前的文本文件
        if (srcText != dstText) {
            int currPos = edit->cursorPosition();
            //气泡提示
            if (!this->parent()) {
                return;
            }

            auto view = this->parent();

            if(!view)
                return;

            auto showPoint = view->mapToGlobal( QPoint( edit->pos().x() + edit->width() / 2,
                                                        edit->pos().y() + edit->height() * 2));
            //背板主题一致
            auto color = view->palette().background().color();

            DFMGlobal::showAlertMessage(showPoint,
                                        color,
                                        QObject::tr("\"\'/\\[]:|<>+=;,?* are not allowed"));

            currPos += dstText.length() - srcText.length();
            QSignalBlocker blocker(edit);
            edit->setText(dstText);
            edit->setCursorPosition(currPos);
        }

        int textCurrLen = dstText.toLocal8Bit().size();
        int textRangeOutLen = textCurrLen - textMaxLen;
        //最大输入框字符控制逻辑
        if (textRangeOutLen > 0) {
            // fix bug 69627
            QVector<uint> list = dstText.toUcs4();
            int cursor_pos = edit->cursorPosition();
            while (dstText.toLocal8Bit().size() > textMaxLen && cursor_pos > 0) {
                list.removeAt(--cursor_pos);
                dstText = QString::fromUcs4(list.data(), list.size());
            }
            QSignalBlocker blocker(edit);
            edit->setText(dstText);
            edit->setCursorPosition(cursor_pos);
        }
    });

    edit->setFrame(false);
    edit->setAttribute(Qt::WA_TranslucentBackground);
    edit->setContentsMargins(0, 0, 0, 0);

    return edit;
}

void DFMListItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::updateEditorGeometry(editor,option,index);
//    Q_UNUSED(index);
//    const QSize &icon_size = /*parent()->*/parent()->iconSize();
//    int column_x = 0;

//    /// draw icon

//    const QRect &opt_rect = option.rect + QMargins(-LIST_MODE_LEFT_MARGIN - LEFT_PADDING, 0, -LIST_MODE_RIGHT_MARGIN - RIGHT_PADDING, 0);

//    QRect icon_rect = opt_rect;

//    icon_rect.setSize(icon_size);

//    column_x = icon_rect.right() + ICON_SPACING + 1;

//    QRect rect = opt_rect;

//    rect.setLeft(column_x + LIST_MODE_EDITOR_LEFT_PADDING);

//    column_x = 1000/*parent()->columnWidth(0) - 1 - parent()->fileViewViewportMargins().left()*/;

//    rect.setRight(qMin(column_x, opt_rect.right()));
//    rect.setTop(opt_rect.y() + (opt_rect.height() - editor->height()) / 2);

//    editor->setGeometry(rect);
}

void DFMListItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *edit = qobject_cast<QLineEdit *>(editor);

    if (!edit) {
        return;
    }

    bool donot_show_suffix = false;/*{ DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowedFileSuffixOnRename).toBool() }*/;
    QString text;

    if (donot_show_suffix) {
        edit->setProperty("_d_whether_show_suffix", index.data(DFMFileViewItem::ItemFileSuffixRole));
        text = index.data(DFMFileViewItem::ItemNameRole).toString();
    } else {
        text = index.data(DFMFileViewItem::ItemNameRole).toString();
    }

    edit->setText(text);
}

static int dataWidth(const QStyleOptionViewItem &option, const QModelIndex &index, int role)
{
    const QVariant &data = index.data(role);
    Qt::Alignment alignment = Qt::Alignment(index.data(Qt::TextAlignmentRole).toInt());

    if (data.canConvert<QString>()) {
        return option.fontMetrics.width(data.toString(), -1, alignment);
    }

    if (data.canConvert<QPair<QString, QString>>()) {
        const QPair<QString, QString> &string_string = qvariant_cast<QPair<QString, QString>>(data);

        return qMax(option.fontMetrics.width(string_string.first, -1, alignment), option.fontMetrics.width(string_string.second, -1, alignment));
    }

    if (data.canConvert<QPair<QString, QPair<QString, QString>>>()) {
        const QPair<QString, QPair<QString, QString>> &string_p_string = qvariant_cast<QPair<QString, QPair<QString, QString>>>(data);

        return option.fontMetrics.width(string_p_string.first, -1, alignment);
    }

    return -1;
}

// sizeHintMode为true时，计算列的宽度时计算的为此列真实需要的宽度，而不受实际列宽所限制
QList<QRect> DFMListItemDelegate::paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode) const
{
//    return DFMStyledItemDelegate::paintGeomertys(option,index,sizeHintMode);
    QList<QRect> geomertys;
//    const QList<int> &columnRoleList = parent()->columnRoleList();
    static QList<int> columnRoleList {
        DFMFileViewItem::ItemFileLastModifiedRole,
        DFMFileViewItem::ItemFileSizeRole,
        DFMFileViewItem::ItemFileMimeTypeRole
    };
    int column_x = 0;

    /// draw icon

    const QRect &opt_rect = option.rect + QMargins(-LIST_MODE_LEFT_MARGIN - LEFT_PADDING, 0, -LIST_MODE_RIGHT_MARGIN - RIGHT_PADDING, 0);

    QRect icon_rect = opt_rect;

    icon_rect.setSize(/*parent()->*/parent()->iconSize());

    geomertys << icon_rect;

    column_x = icon_rect.right() + ICON_SPACING;

    QRect rect = opt_rect;

    rect.setLeft(column_x);

    int role = columnRoleList.at(0);

    if (sizeHintMode) {
        rect.setWidth(dataWidth(option, index, role));
        column_x = rect.right();
    } else {
        column_x = 1000/*parent()->columnWidth(0) - 1 - parent()->fileViewViewportMargins().left()*/;

        rect.setRight(qMin(column_x, opt_rect.right()));
        /// draw file name label
        rect.setWidth(qMin(rect.width(), dataWidth(option, index, role)));
    }

    geomertys << rect;

    for (int i = 1; i < columnRoleList.count(); ++i) {
        QRect rec = opt_rect;

        rec.setLeft(column_x + COLUMU_PADDING);

        if (rec.left() >= opt_rect.right()) {
            return geomertys;
        }

        int rol = columnRoleList.at(i);

        if (sizeHintMode) {
            rec.setWidth(dataWidth(option, index, rol));
            column_x += rec.width();
        } else {
            column_x += 1000/*parent()->columnWidth(i) - 1*/;

            rec.setRight(qMin(column_x, opt_rect.right()));
            rec.setWidth(qMin(rec.width(), dataWidth(option, index, rol)));
        }

        geomertys << rec;
    }

    return geomertys;
}

void DFMListItemDelegate::updateItemSizeHint()
{
    Q_D(DFMListItemDelegate);

//    return DFMStyledItemDelegate::updateItemSizeHint();
    d->textLineHeight = /*parent()->*/parent()->fontMetrics().lineSpacing();
    d->itemSizeHint = QSize(-1, qMax(int(/*parent()->*/parent()->iconSize().height() * 1.1), d->textLineHeight));
}

bool DFMListItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    Q_D(DFMListItemDelegate);

//    if (event->type() == QEvent::Show) {
//        QLineEdit *edit = qobject_cast<QLineEdit *>(object);
//        //在此处处理的逻辑是因为默认QAbstractItemView的QLineEdit重命名会被SelectAll
//        //const 防止被改变
//        if (!edit)
//            return false;

//        bool notShowSuffix =false/* DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowedFileSuffixOnRename).toBool()*/;
//        QString srcText;
//        if (notShowSuffix)
//        {
//            srcText = d->editingIndex.data(DFMFileViewItem::ItemNameRole).toString();
//        } else {
//            srcText = d->editingIndex.data(DFMFileViewItem::ItemNameRole).toString();
//        }

//        //得到处理之后的文件名称
//        QString dstText = DFMGlobal::preprocessingFileName(srcText);
//        //如果存在非法字符且更改了当前的文本文件
//        if (srcText != dstText){
//            //气泡提示
//            DFMGlobal::showAlertMessage(edit->mapToGlobal(edit->pos()),
//                                        /*parent()->*/parent()->palette().background().color(),
//                                        QObject::tr("\"\'/\\[]:|<>+=;,?* are not allowed"));

//            //移动坐标
//            auto srcBaseNameLength =  d->editingIndex.data(DFMFileViewItem::ItemNameRole).toString().length();
//            edit->setText(dstText);
//            int movePosCount = dstText.length() - srcText.length();
//            edit->setText(dstText);
//            edit->setCursorPosition(srcBaseNameLength + movePosCount);
//            edit->setSelection(0, srcBaseNameLength + movePosCount);
//        } else {
//            int selectLength = d->editingIndex.data(DFMFileViewItem::ItemNameRole).toString().length();
//            edit->setText(srcText);
//            edit->setCursorPosition(selectLength);
//            edit->setSelection(0, selectLength);
//        }

//    } else if (event->type() == QEvent::KeyPress) {
//        QKeyEvent *e = static_cast<QKeyEvent *>(event);

//        if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) {
//            e->accept();

//            QLineEdit *edit = qobject_cast<QLineEdit *>(object);

//            if (edit) {
//                edit->close();
//                edit->parentWidget()->setFocus();
//            }

//            return true;
//        }
//    }

    return QStyledItemDelegate::eventFilter(object, event);
}

//static void hideTooltipImmediately()
//{
//    QWidgetList qwl = QApplication::topLevelWidgets();
//    for (QWidget *qw : qwl) {
//        if (QStringLiteral("QTipLabel") == qw->metaObject()->className()) {
//            qw->close();
//        }
//    }
//}

bool DFMListItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
//    if (event->type() == QEvent::ToolTip) {
//        const QString tooltip = index.data(Qt::ToolTipRole).toString();

//        if (tooltip.isEmpty()) {
////            QToolTip::hideText();
//            hideTooltipImmediately();
//        } else {
//            int tooltipsize = tooltip.size();
//            const int nlong = 32;
//            int lines = tooltipsize / nlong + 1;
//            QString strtooltip;
//            for (int i = 0; i < lines; ++i) {
//                strtooltip.append(tooltip.mid(i * nlong, nlong));
//                strtooltip.append("\n");
//            }
//            strtooltip.chop(1);
//            QToolTip::showText(event->globalPos(), strtooltip, view);
//        }

//        return true;
//    }

    return DFMStyledItemDelegate::helpEvent(event, view, option, index);
}
