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
#include "listitemdelegate.h"
#include "fileviewmodel.h"
#include "private/styleditemdelegate_p.h"
#include "widgets/dfmfileview/fileviewitem.h"
#include "widgets/dfmfileview/fileview.h"
#include "dfm-base/dfm_base_global.h"

#include <DListView>
#include <DArrowRectangle>
#include <DPalette>
#include <DApplicationHelper>

#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QAbstractItemView>
#include <QLineEdit>
#include <QApplication>
#include <QToolTip>
#include <QPainterPath>
#include <private/qtextengine_p.h>


#include <linux/limits.h>

DFMBASE_BEGIN_NAMESPACE
namespace GlobalPrivate {

const int ICON_SPACING = 16;
const int LIST_MODE_RECT_RADIUS = 8;
const int LIST_EDITER_HEIGHT = 22;
const int LIST_MODE_EDITOR_LEFT_PADDING = -3;
const int LIST_VIEW_ICON_SIZE = 24;
const int LIST_MODE_LEFT_MARGIN = 20;
const int LIST_MODE_RIGHT_MARGIN = 20;
const int COLUMU_PADDING = 10;
const int LEFT_PADDING = 10;
const int RIGHT_PADDING = 10;

static void elideText(QTextLayout *layout, const QSizeF &size,
                      QTextOption::WrapMode wordWrap,
                      Qt::TextElideMode mode, qreal lineHeight,
                      int flags = 0, QStringList *lines = nullptr,
                      QPainter *painter = nullptr, QPointF offset = QPoint(0, 0),
                      const QColor &shadowColor = QColor(),
                      const QPointF &shadowOffset = QPointF(0, 1),
                      const QBrush &background = QBrush(Qt::NoBrush),
                      qreal backgroundRadius = 4,
                      QList<QRectF> *boundingRegion = nullptr)
{
    qreal height = 0;
    bool drawBackground = background.style() != Qt::NoBrush;
    bool drawShadow = shadowColor.isValid();

    QString text = layout->engine()->hasFormats() ? layout->engine()->block.text() : layout->text();
    QTextOption &text_option = *const_cast<QTextOption *>(&layout->textOption());

    text_option.setWrapMode(wordWrap);

    if (flags & Qt::AlignRight)
        text_option.setAlignment(Qt::AlignRight);
    else if (flags & Qt::AlignHCenter)
        text_option.setAlignment(Qt::AlignHCenter);

    if (painter) {
        text_option.setTextDirection(painter->layoutDirection());
        layout->setFont(painter->font());
    } else {
        // dont paint
        layout->engine()->ignoreBidi = true;
    }

    auto naturalTextRect = [&](const QRectF rect) {
        QRectF new_rect = rect;

        new_rect.setHeight(lineHeight);

        return new_rect;
    };

    auto drawShadowFun = [&](const QTextLine & line) {
        const QPen pen = painter->pen();

        painter->setPen(shadowColor);
        line.draw(painter, shadowOffset);

        // restore
        painter->setPen(pen);
    };

    layout->beginLayout();

    QTextLine line = layout->createLine();
    QRectF lastLineRect;

    while (line.isValid()) {
        height += lineHeight;
        if (height + lineHeight > size.height()) {
            const QString &end_str = layout->engine()->elidedText(mode, qRound(size.width()), flags, line.textStart());

            layout->endLayout();
            layout->setText(end_str);

            if (layout->engine()->block.docHandle()) {
                const_cast<QTextDocument *>(layout->engine()->block.document())->setPlainText(end_str);
            }

            text_option.setWrapMode(QTextOption::NoWrap);
            layout->beginLayout();
            line = layout->createLine();
            line.setLineWidth(size.width() - 1);
            text = end_str;
        } else {
            line.setLineWidth(size.width());
        }

        line.setPosition(offset);

        const QRectF rect = naturalTextRect(line.naturalTextRect());

        if (painter) {
            if (drawBackground) {
                const QMarginsF margins(backgroundRadius, 0, backgroundRadius, 0);
                QRectF backBounding = rect;
                QPainterPath path;

                if (lastLineRect.isValid()) {
                    if (qAbs(rect.width() - lastLineRect.width()) < backgroundRadius * 2) {
                        backBounding.setWidth(lastLineRect.width());
                        backBounding.moveCenter(rect.center());
                        path.moveTo(lastLineRect.x() - backgroundRadius, lastLineRect.bottom() - backgroundRadius);
                        path.lineTo(lastLineRect.x(), lastLineRect.bottom() - 1);
                        path.lineTo(lastLineRect.right(), lastLineRect.bottom() - 1);
                        path.lineTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius);
                        path.lineTo(lastLineRect.right() + backgroundRadius, backBounding.bottom() - backgroundRadius);
                        path.arcTo(backBounding.right() - backgroundRadius, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 0, -90);
                        path.lineTo(backBounding.x(), backBounding.bottom());
                        path.arcTo(backBounding.x() - backgroundRadius, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, -90);
                        lastLineRect = backBounding;
                    } else if (lastLineRect.width() > rect.width()) {
                        backBounding += margins;
                        path.moveTo(backBounding.x() - backgroundRadius, backBounding.y() - 1);
                        path.arcTo(backBounding.x() - backgroundRadius * 2, backBounding.y() - 1, backgroundRadius * 2, backgroundRadius * 2 + 1, 90, -90);
                        path.lineTo(backBounding.x(), backBounding.bottom() - backgroundRadius);
                        path.arcTo(backBounding.x(), backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 180, 90);
                        path.lineTo(backBounding.right() - backgroundRadius, backBounding.bottom());
                        path.arcTo(backBounding.right() - backgroundRadius * 2, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, 90);
                        path.lineTo(backBounding.right(), backBounding.top() + backgroundRadius);
                        path.arcTo(backBounding.right(), backBounding.top() - 1, backgroundRadius * 2, backgroundRadius * 2 + 1, 180, -90);
                        path.closeSubpath();
                        lastLineRect = rect;
                    } else {
                        backBounding += margins;
                        path.moveTo(lastLineRect.x() - backgroundRadius * 2, lastLineRect.bottom());
                        path.arcTo(lastLineRect.x() - backgroundRadius * 3, lastLineRect.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, 90);
                        path.lineTo(lastLineRect.x(), lastLineRect.bottom() - 1);
                        path.lineTo(lastLineRect.right(), lastLineRect.bottom() - 1);
                        path.lineTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius * 2);
                        path.arcTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 180, 90);

                        //                        path.arcTo(lastLineRect.x() - backgroundReaius, lastLineRect.bottom() - backgroundReaius * 2, backgroundReaius * 2, backgroundReaius * 2, 180, 90);
                        //                        path.lineTo(lastLineRect.x() - backgroundReaius * 3, lastLineRect.bottom());
                        //                        path.moveTo(lastLineRect.right(), lastLineRect.bottom());
                        //                        path.arcTo(lastLineRect.right() - backgroundReaius, lastLineRect.bottom() - backgroundReaius * 2, backgroundReaius * 2, backgroundReaius * 2, 270, 90);
                        //                        path.arcTo(lastLineRect.right() + backgroundReaius, lastLineRect.bottom() - backgroundReaius * 2, backgroundReaius * 2, backgroundReaius * 2, 180, 90);
                        //                        path.lineTo(lastLineRect.right(), lastLineRect.bottom());

                        path.addRoundedRect(backBounding, backgroundRadius, backgroundRadius);
                        lastLineRect = rect;
                    }
                } else {
                    lastLineRect = backBounding;
                    path.addRoundedRect(backBounding + margins, backgroundRadius, backgroundRadius);
                }

                bool a = painter->testRenderHint(QPainter::Antialiasing);
                qreal o = painter->opacity();

                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setOpacity(1);
                painter->fillPath(path, background);
                painter->setRenderHint(QPainter::Antialiasing, a);
                painter->setOpacity(o);
            }

            if (drawShadow) {
                drawShadowFun(line);
            }

            line.draw(painter, QPointF(0, 0));
        }

        if (boundingRegion) {
            boundingRegion->append(rect);
        }

        offset.setY(offset.y() + lineHeight);

        //        // find '\n'
        //        int text_length_line = line.textLength();
        //        for (int start = line.textStart(); start < line.textStart() + text_length_line; ++start) {
        //            if (text.at(start) == '\n')
        //                height += lineHeight;
        //        }

        if (lines) {
            lines->append(text.mid(line.textStart(), line.textLength()));
        }

        if (height + lineHeight > size.height())
            break;

        line = layout->createLine();
    }

    layout->endLayout();
}

QString elideText(const QString &text, const QSizeF &size,
                  QTextOption::WrapMode wordWrap, const QFont &font,
                  Qt::TextElideMode mode, qreal lineHeight, qreal flags = 0)
{
    QTextLayout textLayout(text);

    textLayout.setFont(font);

    QStringList lines;

    elideText(&textLayout, size, wordWrap, mode, lineHeight, static_cast<int>(flags), &lines);

    return lines.join('\n');
}


QString preprocessingFileName(QString name)
{
    return name;
}

void showAlertMessage(QPoint globalPoint, const QColor &backgroundColor, const QString &text, int duration = 3000)
{
    static DArrowRectangle* tooltip = nullptr;
    if (!tooltip) {
        tooltip = new DArrowRectangle(DArrowRectangle::ArrowTop, nullptr);
        tooltip->setObjectName("AlertTooltip");
        QLabel *label = new QLabel(tooltip);
        label->setWordWrap(true);
        label->setMaximumWidth(500);
        tooltip->setContent(label);
        tooltip->setBackgroundColor(backgroundColor);
        tooltip->setArrowX(15);
        tooltip->setArrowHeight(5);

        QTimer::singleShot(duration, [=] {
            delete tooltip;
            tooltip = nullptr;
        });

        label->setText(text);
        label->adjustSize();

        tooltip->show(static_cast<int>(globalPoint.x()),static_cast<int>(globalPoint.y()));
    }
}

} // namespace GlobalPrivate


DWIDGET_USE_NAMESPACE

class ListItemDelegatePrivate : public StyledItemDelegatePrivate
{
public:
    explicit ListItemDelegatePrivate(ListItemDelegate *qq)
        : StyledItemDelegatePrivate(qq) {}
};

ListItemDelegate::ListItemDelegate(DListView *parent) :
    StyledItemDelegate(parent)
{
    parent->setIconSize(QSize(GlobalPrivate::LIST_VIEW_ICON_SIZE,
                              GlobalPrivate::LIST_VIEW_ICON_SIZE));
}

void ListItemDelegate::paint(QPainter *painter,
                                const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    Q_D(const ListItemDelegate);
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
    QRect dstRect(itemRect.x() + GlobalPrivate::LIST_MODE_LEFT_MARGIN,
                  itemRect.y(),
                  itemRect.width() - (GlobalPrivate::LIST_MODE_LEFT_MARGIN
                                      + GlobalPrivate::LIST_MODE_RIGHT_MARGIN),
                  itemRect.height());

    //取模设置当前的交替变化
    if (index.row() % 2 == 1) {
        auto blockColorBrush = parent()->palette().background();
        QPainterPath path;
        path.addRoundedRect(dstRect, GlobalPrivate::LIST_MODE_RECT_RADIUS,
                            GlobalPrivate::LIST_MODE_RECT_RADIUS);//圆角8 UI要求
        painter->fillPath(path, adjustItemAlterColor);
    } else {
        painter->setBrush(baseColor);
    }

    //设置hover高亮
    if (option.state & QStyle::StateFlag::State_MouseOver) {
        QPainterPath path;
        path.addRoundedRect(dstRect, GlobalPrivate::LIST_MODE_RECT_RADIUS,
                            GlobalPrivate::LIST_MODE_RECT_RADIUS);
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

    opt.rect += QMargins(- GlobalPrivate::LIST_MODE_LEFT_MARGIN, 0,
                         - GlobalPrivate::LIST_MODE_RIGHT_MARGIN, 0);

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
        path.addRoundedRect(opt.rect, GlobalPrivate::LIST_MODE_RECT_RADIUS,
                            GlobalPrivate::LIST_MODE_RECT_RADIUS);
        painter->save();
        painter->setOpacity(1);
        painter->setRenderHint(QPainter::Antialiasing);
        painter->fillPath(path, option.palette.color(cg, colorRole));
        painter->restore();
    } else if (isDropTarget) {
        QRectF rect = opt.rect;
        QPainterPath path;
        rect += QMarginsF(-0.5, -0.5, -0.5, -0.5);
        path.addRoundedRect(rect, GlobalPrivate::LIST_MODE_RECT_RADIUS,
                            GlobalPrivate::LIST_MODE_RECT_RADIUS);
        painter->setRenderHint(QPainter::Antialiasing, true);
        painter->fillPath(path, option.palette.color(cg, colorRole));
        painter->setRenderHint(QPainter::Antialiasing, false);
    }

    opt.rect.setLeft(opt.rect.left() + GlobalPrivate::LEFT_PADDING);
    opt.rect.setRight(opt.rect.right() - GlobalPrivate::RIGHT_PADDING);

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

    column_x = icon_rect.right() + GlobalPrivate::ICON_SPACING;

    QRect rect = opt.rect;

    rect.setLeft(column_x);

    qint32 totalWidth = opt.rect.width() - column_x;
    //动态计算每个列的大小
    const QList<int> &columnRoleList = index.data(FileViewItem::ItemColumListRole).value<QList<int>>();
    if (columnRoleList.isEmpty())
        return;
    QList<int> widthScaleList = index.data(FileViewItem::ItemColumWidthScaleListRole).value<QList<int>>();
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

                const QString &suffix = "." + index.data(FileViewItem::ItemFileSuffixRole).toString();
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
                file_name = GlobalPrivate::elideText(index.data(role).toString().remove('\n'),
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

    const FileViewModel *model = qobject_cast<const FileViewModel *>(index.model());
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

        rec.setLeft(column_x + GlobalPrivate::COLUMU_PADDING);

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
            if(rol == FileViewItem::ItemFilePathRole) {
                //                // 如果是保险箱路径,则不显示真实路径
                //                if(VaultController::isVaultFile(strInfo))
                //                    strInfo = VaultController::localPathToVirtualPath(index.data(rol).toString());
            }
            const QString &text = GlobalPrivate::elideText(strInfo, rec.size(),
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


void ListItemDelegate::drawNotStringData(const QStyleOptionViewItem &opt, int lineHeight, const QRect &rect, const QVariant &data,
                                            bool drawBackground, QPainter *painter, const int &column) const
{

    const FileViewModel *model = qobject_cast<const FileViewModel *>(parent()->model());
    const AbstractFileInfoPointer &fileInfo = InfoFactory::instance().create<LocalFileInfo>(const_cast<FileViewModel *>(model)->rootUrl());

    //    int sortRole = model->sortRole();
    static QList<int> userColumnRoles {
        FileViewItem::ItemFileLastModifiedRole,
                FileViewItem::ItemFileSizeRole,
                FileViewItem::ItemFileMimeTypeRole
    };
    int sortRoleIndexByColumnChildren = 0;/*userColumnRoles.indexOf(sortRole);*/

    const QColor &active_color = opt.palette.color(QPalette::Active, drawBackground ? QPalette::BrightText : QPalette::Text);
    const QColor &normal_color = opt.palette.color(QPalette::Inactive, drawBackground ? QPalette::BrightText : QPalette::Text);

    if (data.canConvert<QPair<QString, QString>>()) {
        QPair<QString, QString> name_path = qvariant_cast<QPair<QString, QString>>(data);

        const QString &file_name = GlobalPrivate::elideText(name_path.first.remove('\n'),
                                                            QSize(rect.width(), rect.height() / 2), QTextOption::NoWrap,
                                                            opt.font, Qt::ElideRight,
                                                            lineHeight);
        painter->setPen(sortRoleIndexByColumnChildren == 0 ? active_color : normal_color);
        painter->drawText(rect.adjusted(0, 0, 0, -rect.height() / 2), Qt::AlignBottom, file_name);

        const QString &file_path = GlobalPrivate::elideText(name_path.second.remove('\n'),
                                                            QSize(rect.width(), rect.height() / 2), QTextOption::NoWrap,
                                                            opt.font, Qt::ElideRight,
                                                            lineHeight);

        painter->setPen(sortRoleIndexByColumnChildren == 1 ? active_color : normal_color);
        painter->drawText(rect.adjusted(0, rect.height() / 2, 0, 0), Qt::AlignTop, file_path);
    } else if (data.canConvert<QPair<QString, QPair<QString, QString>>>()) {
        QRect new_rect = rect;

        const QPair<QString, QPair<QString, QString>> &dst = qvariant_cast<QPair<QString, QPair<QString, QString>>>(data);

        const QString &date = GlobalPrivate::elideText(dst.first, QSize(rect.width(), rect.height() / 2),
                                                       QTextOption::NoWrap, opt.font,
                                                       Qt::ElideRight, lineHeight);

        painter->setPen(sortRoleIndexByColumnChildren == 0 ? active_color : normal_color);
        painter->drawText(new_rect.adjusted(0, 0, 0, -new_rect.height() / 2), Qt::AlignBottom, date, &new_rect);

        new_rect = QRect(rect.left(), rect.top(), new_rect.width(), rect.height());

        const QString &size = GlobalPrivate::elideText(dst.second.first, QSize(new_rect.width() / 2, new_rect.height() / 2),
                                                       QTextOption::NoWrap, opt.font,
                                                       Qt::ElideRight, lineHeight);

        painter->setPen(sortRoleIndexByColumnChildren == 1 ? active_color : normal_color);
        painter->drawText(new_rect.adjusted(0, new_rect.height() / 2, 0, 0), Qt::AlignTop | Qt::AlignLeft, size);

        const QString &type = GlobalPrivate::elideText(dst.second.second, QSize(new_rect.width() / 2, new_rect.height() / 2),
                                                       QTextOption::NoWrap, opt.font,
                                                       Qt::ElideLeft, lineHeight);
        painter->setPen(sortRoleIndexByColumnChildren == 2 ? active_color : normal_color);
        painter->drawText(new_rect.adjusted(0, new_rect.height() / 2, 0, 0), Qt::AlignTop | Qt::AlignRight, type);
    }
}


QSize ListItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
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

QWidget *ListItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_D(const ListItemDelegate);

    d->editingIndex = index;

    QLineEdit *edit = new QLineEdit(parent);

    const AbstractFileInfoPointer &file_info = qobject_cast<FileViewModel*>(this->parent())->fileInfo(index);
    if (file_info->url().scheme() == "search") {
        edit->setFixedHeight(GlobalPrivate::LIST_EDITER_HEIGHT * 2 - 10);
    } else {
        edit->setFixedHeight(GlobalPrivate::LIST_EDITER_HEIGHT);
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
        QString dstText = GlobalPrivate::preprocessingFileName(srcText);

        //超出长度将不再被支持输入获取当前
        bool donot_show_suffix= false;/*{ DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowedFileSuffixOnRename).toBool() };*/

        //获取当前编辑框支持的最大文字长度
        int textMaxLen = INT_MAX;
        if (donot_show_suffix) {
            const QString &suffix = d->editingIndex.data(FileViewItem::ItemFileSuffixRole).toString();
            edit->setProperty("_d_whether_show_suffix", suffix);
            textMaxLen = NAME_MAX - suffix.toLocal8Bit().size() - (suffix.isEmpty() ? 0 : 1);
        } else {
            textMaxLen = NAME_MAX;
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

            GlobalPrivate::showAlertMessage(showPoint,
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

void ListItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::updateEditorGeometry(editor,option,index);
}

void ListItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *edit = qobject_cast<QLineEdit *>(editor);

    if (!edit) {
        return;
    }

    bool donot_show_suffix = false;/*{ DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowedFileSuffixOnRename).toBool() }*/;
    QString text;

    if (donot_show_suffix) {
        edit->setProperty("_d_whether_show_suffix", index.data(FileViewItem::ItemFileSuffixRole));
        text = index.data(FileViewItem::ItemNameRole).toString();
    } else {
        text = index.data(FileViewItem::ItemNameRole).toString();
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
QList<QRect> ListItemDelegate::paintGeomertys(const QStyleOptionViewItem &option, const QModelIndex &index, bool sizeHintMode) const
{
    //    return DFMStyledItemDelegate::paintGeomertys(option,index,sizeHintMode);
    QList<QRect> geomertys;
    //    const QList<int> &columnRoleList = parent()->columnRoleList();
    static QList<int> columnRoleList {
        FileViewItem::ItemFileLastModifiedRole,
                FileViewItem::ItemFileSizeRole,
                FileViewItem::ItemFileMimeTypeRole
    };
    int column_x = 0;

    /// draw icon

    const QRect &opt_rect = option.rect + QMargins(- GlobalPrivate::LIST_MODE_LEFT_MARGIN - GlobalPrivate::LEFT_PADDING, 0,
                                                   - GlobalPrivate::LIST_MODE_RIGHT_MARGIN - GlobalPrivate::RIGHT_PADDING, 0);

    QRect icon_rect = opt_rect;

    icon_rect.setSize(/*parent()->*/parent()->iconSize());

    geomertys << icon_rect;

    column_x = icon_rect.right() + GlobalPrivate::ICON_SPACING;

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

        rec.setLeft(column_x + GlobalPrivate::COLUMU_PADDING);

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

void ListItemDelegate::updateItemSizeHint()
{
    Q_D(ListItemDelegate);

    d->textLineHeight = /*parent()->*/parent()->fontMetrics().lineSpacing();
    d->itemSizeHint = QSize(-1, qMax(int(/*parent()->*/parent()->iconSize().height() * 1.1), d->textLineHeight));
}

bool ListItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    Q_D(ListItemDelegate);


    return QStyledItemDelegate::eventFilter(object, event);
}


bool ListItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{

    return StyledItemDelegate::helpEvent(event, view, option, index);
}
DFMBASE_END_NAMESPACE
