/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *               2019 ~ 2019 Chris Xiong
 *
 * Author:     Chris Xiong<chirs241097@gmail.com>
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

#include <DApplication>
#include <DApplicationHelper>
#include <DPalette>
#include <sys/stat.h>
#include <QLineEdit>

#include "models/computermodel.h"
#include "shutil/fileutils.h"
#include "computerviewitemdelegate.h"
#include "dfmapplication.h"

//LABEL间的间隔
#define LABEL_TOTAL_SPACING 6
#define TOTAL_PROBAR_SPACING 4
#define ICON_LABEL_SPACING 10

//上下左右间隔
#define ICON_TOP_MARGIN 20
#define ICON_BOTTOM_MARGIN 20
#define ICON_LEFT_MARGIN 10
#define LABEL_RIGHT_MARGIN 20

//!
//! \brief 一些特殊的字体和 CESI_*_GB* 的字体在计算机页面重命名时，显示位置偏上
//! 因此针对这些字体使用 top-margin 调整，确保文字垂直居中
//! \return top-margin of lineeditor
//!
static int editorMarginTop(const QString &family)
{
    int margin = 0;
    if (dfm_util::isContains(family, QString("Unifont"), QString("WenQuanYi Micro Hei")))
        margin = 4;
    else if (family.startsWith("CESI")) {
        if (family.endsWith("GB2312") || family.endsWith("GB13000")|| family.endsWith("GB18030"))
            margin = 4;
    }
    return margin;
}

ComputerViewItemDelegate::ComputerViewItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    par = qobject_cast<ComputerView *>(parent);
}

ComputerViewItemDelegate::~ComputerViewItemDelegate()
{
}

void ComputerViewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setRenderHint(QPainter::RenderHint::Antialiasing);

    ComputerModelItemData::Category cat = ComputerModelItemData::Category(index.data(ComputerModel::DataRoles::ICategoryRole).toInt());
    if (cat == ComputerModelItemData::Category::cat_splitter) {
        QFont fnt(par->font());
        fnt.setPixelSize(30);
        painter->setFont(fnt);
        painter->setPen(qApp->palette().color(QPalette::ColorRole::Text));
        painter->drawText(option.rect, 0, index.data(Qt::ItemDataRole::DisplayRole).toString());
        return;
    }

    if (cat == ComputerModelItemData::Category::cat_widget) {
        return;
    }

    DPalette pl(DApplicationHelper::instance()->palette(option.widget));
    QColor c = pl.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);
    QColor base_color = c;
    if (option.widget) {
        DPalette pa = DApplicationHelper::instance()->palette(option.widget);
        base_color = option.widget->palette().base().color();
        DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(base_color);
        if (ct == DGuiApplicationHelper::DarkType) {
            base_color = DGuiApplicationHelper::adjustColor(base_color, 0, 0, +5, 0, 0, 0, 0);
        }
    }

    if (option.state & QStyle::StateFlag::State_Selected) {
        // c = pl.color(DPalette::ColorGroup::Active, QPalette::ColorRole::Highlight);
        c.setAlpha(c.alpha() + 30);
    } else if (option.state & QStyle::StateFlag::State_MouseOver) {
        DGuiApplicationHelper::ColorType ct = DGuiApplicationHelper::toColorType(base_color);
        if (ct == DGuiApplicationHelper::DarkType) {
            base_color = DGuiApplicationHelper::adjustColor(base_color, 0, 0, +5, 0, 0, 0, 0);
            c = base_color;
        }
        else
            c = c.lighter();
    } else {
        c = base_color;
    }
    painter->setPen(c);
    painter->setBrush(c);

    painter->drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 18, 18);

    const QIcon icon = index.data(Qt::ItemDataRole::DecorationRole).value<QIcon>();

    //目录绘制
    if (cat == ComputerModelItemData::Category::cat_user_directory) {
        const int iconsize = par->view()->iconSize().width() * 4 / 3;
        const int topmargin = iconsize / 8 + 3;
        const int text_max_width = option.rect.width() - 24;
        const QString elided_text = option.fontMetrics.elidedText(index.data(Qt::DisplayRole).toString(), Qt::ElideMiddle, text_max_width);
        const int fstw = par->fontMetrics().width(elided_text);
        const int leftmargin = iconsize / 4 + 12;
        const int text_topmargin = iconsize / 4;
        painter->drawPixmap(option.rect.x() + leftmargin, option.rect.y() + topmargin, icon.pixmap(iconsize));

        painter->setFont(par->font());
        painter->setPen(qApp->palette().color((option.state & QStyle::StateFlag::State_Selected) ? QPalette::ColorRole::HighlightedText : QPalette::ColorRole::Text));
        painter->drawText(option.rect.x() + (option.rect.width() - fstw) / 2, option.rect.y() + topmargin + iconsize + text_topmargin, elided_text);
        return;
    }

    //设备绘制
    QRect nameLabelRect = option.rect;
    const int iconsize = par->view()->iconSize().width();

    const int text_max_width = sizeHint(option,index).width()
            - ICON_LEFT_MARGIN
            - iconsize
            - ICON_LABEL_SPACING
            - LABEL_RIGHT_MARGIN;

    const int fontpixelsize = par->fontInfo().pixelSize();
    nameLabelRect.setLeft(option.rect.left() + ICON_LEFT_MARGIN + iconsize + ICON_LABEL_SPACING);
    nameLabelRect.setTop(option.rect.top() + ICON_TOP_MARGIN);
    nameLabelRect.setHeight(par->fontMetrics().height());
    QFont font = par->font();
    font.setWeight(66);
    painter->setFont(font);
    painter->setPen(qApp->palette().color(QPalette::ColorRole::Text));

    QString fileSysType = index.data(ComputerModel::DataRoles::FileSystemRole).toString();
    int fstw = par->fontMetrics().width(fileSysType);
    QString text;
    if (!fileSysType.isEmpty()) {
        text = option.fontMetrics.elidedText(index.data(Qt::DisplayRole).toString(), Qt::ElideMiddle, text_max_width - fstw - 7);
    } else {
        text = option.fontMetrics.elidedText(index.data(Qt::DisplayRole).toString(), Qt::ElideMiddle, text_max_width - 5);
    }

    QRect nameTextDarwRect; //此处真实的字符绘制Rect非labelRect
    painter->drawText(nameLabelRect, Qt::TextWrapAnywhere, text, &nameTextDarwRect);

    // 添加对文件系统格式的显示
    if (DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowFileSystemTagOnDiskIcon).toBool() && !fileSysType.isEmpty()) {

        // 使用细线进行绘制
        font.setWeight(12);
        painter->setFont(font);
        QRect fsTypeLabelrect = nameTextDarwRect;

        fsTypeLabelrect.setWidth(fstw);
        fsTypeLabelrect.setHeight(fontpixelsize + 4);
        //与真实字符绘制Rect保持左侧12的间距
        fsTypeLabelrect.moveLeft(nameTextDarwRect.right() + 12);
        //与真实字符绘制Rect保持 居中
        fsTypeLabelrect.moveBottom(nameTextDarwRect.bottom() - (nameTextDarwRect.height() - fsTypeLabelrect.height())/2);
        //微调类型绘制范围
        fsTypeLabelrect.adjust(-5, 0, 5, 0);

        QColor brushColor(0xD2D2D2);
        QColor penColor(0x5D5D5D);
        QColor borderColor(0xA5A5A5);

        if (fileSysType == "EXT2"
                || fileSysType == "EXT3"
                || fileSysType == "EXT4"
                || fileSysType == "VFAT") {

            brushColor = QColor(0xA1E4FF);
            penColor = QColor(0x0081B2);
            borderColor = QColor(0x73C7EE);

        } else if (fileSysType == "NTFS"
                   || fileSysType == "FAT16"
                   || fileSysType == "FAT32"
                   || fileSysType == "EXFAT") {

            brushColor = QColor(0xFFDDA1);
            penColor = QColor(0x502504);
            borderColor = QColor(0xEEB273);
        }

        painter->setBrush(brushColor);
        painter->setPen(borderColor);
        painter->drawRoundedRect(fsTypeLabelrect, 7.5, 7.5);
        painter->setPen(penColor);
        painter->drawText(fsTypeLabelrect, Qt::AlignCenter, fileSysType);
    }

    QFont smallf(par->font());
    smallf.setPixelSize(int(fontpixelsize * 0.85));
    painter->setFont(smallf);

    QRect totalLabelRect = nameLabelRect;

    // 修复bug-63543 控制中心修改字体为unifont后，item的格式标签会覆盖住盘符大小标签
    totalLabelRect.setTop(nameTextDarwRect.bottom() + LABEL_TOTAL_SPACING); //添加与真实绘制的盘符名称间隔
    totalLabelRect.setHeight(QFontMetrics(smallf).height()); //字符高度

    quint64 sizeinuse = index.data(ComputerModel::DataRoles::SizeInUseRole).toULongLong();
    quint64 sizetotal = index.data(ComputerModel::DataRoles::SizeTotalRole).toULongLong();
    QString strVolTag = index.data(ComputerModel::DataRoles::VolumeTagRole).toString();
    painter->setPen(pl.color(DPalette::TextTips));

    // Paint size.
    bool bSizeVisible = index.data(ComputerModel::DataRoles::SizeRole).toBool();
    QString scheme = index.data(ComputerModel::DataRoles::SchemeRole).toString();
    if (bSizeVisible) {
        if (scheme == DFMVAULT_SCHEME) {
            // vault only show size.
            painter->drawText(totalLabelRect, Qt::AlignLeft, FileUtils::formatSize(static_cast<qint64>(sizeinuse)));
        } else {
            painter->drawText(totalLabelRect, Qt::AlignLeft, FileUtils::diskUsageString(sizeinuse, sizetotal, strVolTag));
        }
    }

    // for app entry description
    if (scheme == APPENTRY_SCHEME) {
        auto commentRect = totalLabelRect;
        commentRect.setWidth(text_max_width);
        auto fullComment = index.data(ComputerModel::DataRoles::AppEntryDescription).toString();
        QFontMetrics fm(smallf);
        auto firstLine = fm.elidedText(fullComment, Qt::ElideRight, text_max_width);
        if (firstLine.endsWith("…")) {
            firstLine.chop(1);
            painter->drawText(commentRect, Qt::AlignLeft, firstLine);
            // draw second line
            auto secondLine = fullComment.remove(0, firstLine.length());
            secondLine = fm.elidedText(secondLine, Qt::ElideRight, text_max_width);
            commentRect.moveTo(commentRect.x(), commentRect.y() + commentRect.height());
            painter->drawText(commentRect, Qt::AlignLeft, secondLine);
        } else {
            painter->drawText(commentRect, Qt::AlignLeft, firstLine);
        }
    }

    QRect usgplrect(QPoint(totalLabelRect.x(),
                           totalLabelRect.bottom() + TOTAL_PROBAR_SPACING),
                    QSize(text_max_width, 6));
    QStyle *sty = option.widget && option.widget->style() ? option.widget->style() : qApp->style();
    QStyleOptionProgressBar plopt;
    plopt.textVisible = false;
    plopt.rect = usgplrect;
    plopt.minimum = 0;
    plopt.maximum = 10000;
    plopt.progress = sizetotal && ~sizeinuse ? int(10000. * sizeinuse / sizetotal) : 0;
    if (plopt.progress > plopt.maximum) {
        plopt.progress = plopt.maximum;
    }
    QColor plcolor;
    if (plopt.progress < 7000) {
        plcolor = QColor(0xFF0081FF);
    } else if (plopt.progress < 9000) {
        plcolor = QColor(0xFFF8AE2C);
    } else {
        plcolor = QColor(0xFFFF6170);
    }
    plopt.palette = option.widget ? option.widget->palette() : qApp->palette();
    plopt.palette.setColor(QPalette::ColorRole::Highlight, plcolor);
    painter->setPen(Qt::PenStyle::NoPen);

    // Paint progress
    bool bProgressVisible = index.data(ComputerModel::DataRoles::ProgressRole).toBool();

    if (bProgressVisible) {
        sty->drawControl(QStyle::ControlElement::CE_ProgressBarGroove, &plopt, painter, option.widget);
        sty->drawControl(QStyle::ControlElement::CE_ProgressBarContents, &plopt, painter, option.widget);
    }

    //设备盘符图标高度居中
    int deviceIconAligmentY = option.rect.y() + (sizeHint(option,index).height() - iconsize)/2;
    painter->drawPixmap(option.rect.x() + ICON_LEFT_MARGIN, deviceIconAligmentY, icon.pixmap(iconsize));
}

QSize ComputerViewItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    ComputerModelItemData::Category cat = ComputerModelItemData::Category(index.data(ComputerModel::DataRoles::ICategoryRole).toInt());
    if (cat == ComputerModelItemData::Category::cat_widget) {
        return static_cast<ComputerModelItemData *>(index.internalPointer())->widget->size();
    } else if (cat == ComputerModelItemData::Category::cat_splitter) {
        return QSize(par->width() - 12, 45);
    } else if (cat == ComputerModelItemData::Category::cat_user_directory) {
        int sz = par->view()->iconSize().width() * 2 + 24;
        return QSize(sz, sz);
    }

    const int iconSize = par->view()->iconSize().width();
    const int textMaxWidth = int(iconSize * 3.75);

    //右侧真实大小
    int processBarHeight = 6;
    QFont smallf(par->font());
    smallf.setPixelSize(int(par->fontInfo().pixelSize() * 0.85));
    int capacityLabelHeight = QFontMetrics(smallf).height();
    int nameTextLabelHeight = QFontMetrics(option.fontMetrics).height();

    int colLabelsHeight = processBarHeight
            + capacityLabelHeight
            + nameTextLabelHeight
            + LABEL_TOTAL_SPACING
            + TOTAL_PROBAR_SPACING
            + ICON_TOP_MARGIN
            + ICON_BOTTOM_MARGIN;

    return QSize(ICON_LEFT_MARGIN + iconSize + ICON_LABEL_SPACING + textMaxWidth + LABEL_RIGHT_MARGIN,
                 qMax(ICON_TOP_MARGIN + iconSize + ICON_BOTTOM_MARGIN, colLabelsHeight));
}

QWidget *ComputerViewItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    editingIndex = index;
    QLineEdit *le = new QLineEdit(parent);
    editingEditor = le;
    int topMargin = editorMarginTop(option.font.family());
    le->setFrame(false);
    le->setTextMargins(0, topMargin, 0, 0);
    //消除编辑框背后多余的填充色
//    le->setAutoFillBackground(true);
    le->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    QRegExp regx("^[^\\.\\\\/\':\\*\\?\"<>|%&][^\\\\/\':\\*\\?\"<>|%&]*"); //屏蔽特殊字符
    QValidator *validator = new QRegExpValidator(regx, le);
    le->setValidator(validator);

    int maxLenInBytes = index.data(ComputerModel::EditorLengthRole).toInt();
    connect(le, &QLineEdit::textChanged, this, [le, maxLenInBytes](const QString &txt) {
        if (!le)
            return;
        if (txt.toUtf8().length() > maxLenInBytes) {
            const QSignalBlocker blocker(le);
            QString newLabel = txt;
            newLabel.chop(1);
            le->setText(newLabel);
        }
    });

    connect(le, &QLineEdit::destroyed, this, [this, le] {
        if (editingEditor == le)
            editingIndex = QModelIndex();
    });

    return le;
}

void ComputerViewItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit *le = qobject_cast<QLineEdit *>(editor);
    le->setText(index.data(Qt::DisplayRole).toString()); // TODO: implement EditRole
    this->par->view()->model()->setData(index, true, ComputerModel::DataRoles::IsEditingRole);
}

void ComputerViewItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *le = qobject_cast<QLineEdit *>(editor);
    model->setData(index, le->text());
    model->setData(index, false, ComputerModel::DataRoles::IsEditingRole);
}

void ComputerViewItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    if (index.model() && index.data(ComputerModel::DataRoles::ICategoryRole) == ComputerModelItemData::Category::cat_widget) {
        editor->setGeometry(option.rect);
        return;
    }
    QRect textrect = option.rect;
    const int iconsize = par->view()->iconSize().width();
    const int text_max_width = int(iconsize * 3.75);
    textrect.setLeft(option.rect.left() + ICON_LEFT_MARGIN + iconsize + ICON_LABEL_SPACING + 1);
    textrect.setWidth(text_max_width);
    textrect.setTop(option.rect.top() + ICON_TOP_MARGIN - 2);
    textrect.setHeight(par->fontInfo().pixelSize() * 2);
    editor->setGeometry(textrect);
}

void ComputerViewItemDelegate::closeEditingEditor(ComputerListView *view)
{
    if (!view || !editingIndex.isValid())
        return;

    QWidget *editor = view->indexWidget(editingIndex);
    if (!editor)
        return;

    QMetaObject::invokeMethod(this, "_q_commitDataAndCloseEditor",
                              Qt::DirectConnection, Q_ARG(QWidget *, editor));
}


