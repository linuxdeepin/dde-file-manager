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
        c = c.lighter();
    } else {
        c = base_color;
    }
    painter->setPen(c);
    painter->setBrush(c);

    painter->drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 18, 18);

    const QIcon icon = index.data(Qt::ItemDataRole::DecorationRole).value<QIcon>();

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

    QRect textrect = option.rect;
    QRect otextrect;
    const int iconsize = par->view()->iconSize().width();
    const int topmargin = iconsize / 2 + 2;
    const int leftmargin = iconsize / 3 + 3;
    const int text_max_width = int(iconsize * 3.75);
    const int spacing = iconsize / 3 + 1;
    const int rightmargin = iconsize / 3 + 9;
    const int fontpixelsize = par->fontInfo().pixelSize();
    textrect.setLeft(option.rect.left() + leftmargin + iconsize + spacing + 1);
    textrect.setTop(option.rect.top() + topmargin);
    textrect.setHeight(fontpixelsize * 2);
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

    painter->drawText(textrect, Qt::TextWrapAnywhere, text, &otextrect);

    // 添加对文件系统格式的显示
    if (DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowFileSystemTagOnDiskIcon).toBool() && !fileSysType.isEmpty()) {

        // 使用细线进行绘制
        font.setWeight(12);
        painter->setFont(font);

        otextrect.moveLeft(otextrect.right() + 12);
        otextrect.moveBottom(otextrect.bottom() + 2);
        otextrect.setWidth(fstw);
        otextrect.setHeight(fontpixelsize + 4);
        otextrect.adjust(-5, 0, 5, 0);

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
        painter->drawRoundedRect(otextrect, 7.5, 7.5);
        painter->setPen(penColor);
        painter->drawText(otextrect, Qt::AlignCenter, fileSysType);
    }

    QFont smallf(par->font());
    smallf.setPixelSize(int(fontpixelsize * 0.85));
    // smallf.setPixelSize(int(fontpixelsize * 0.85));
    painter->setFont(smallf);
    textrect.setLeft(option.rect.left() + leftmargin + iconsize + spacing);
    textrect.setRight(option.rect.right() - rightmargin);
    textrect.setTop(option.rect.top() + topmargin + par->fontMetrics().height() + 1);
    textrect.setHeight(fontpixelsize);

    quint64 sizeinuse = index.data(ComputerModel::DataRoles::SizeInUseRole).toULongLong();
    quint64 sizetotal = index.data(ComputerModel::DataRoles::SizeTotalRole).toULongLong();
    QString strVolTag = index.data(ComputerModel::DataRoles::VolumeTagRole).toString();
    painter->setPen(pl.color(DPalette::TextTips));


    // Paint size.
    bool bSizeVisible = index.data(ComputerModel::DataRoles::SizeRole).toBool();
    if (bSizeVisible) {
        QString scheme = index.data(ComputerModel::DataRoles::SchemeRole).toString();
        if (scheme == DFMVAULT_SCHEME) {
            // vault only show size.
            painter->drawText(textrect, Qt::AlignLeft, FileUtils::formatSize(static_cast<qint64>(sizeinuse)));
        } else {
            painter->drawText(textrect, Qt::AlignLeft, FileUtils::diskUsageString(sizeinuse, sizetotal, strVolTag));
        }
    }

    QRect usgplrect(option.rect.topLeft() + QPoint(iconsize + leftmargin + spacing, topmargin + 14 + 2 * fontpixelsize), QSize(text_max_width, 6));
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

    painter->drawPixmap(option.rect.x() + leftmargin, option.rect.y() + topmargin, icon.pixmap(iconsize));
}

QSize ComputerViewItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    ComputerModelItemData::Category cat = ComputerModelItemData::Category(index.data(ComputerModel::DataRoles::ICategoryRole).toInt());
    if (cat == ComputerModelItemData::Category::cat_widget) {
        return static_cast<ComputerModelItemData *>(index.internalPointer())->widget->size();
    } else if (cat == ComputerModelItemData::Category::cat_splitter) {
        return QSize(par->width() - 12, 45);
    } else if (cat == ComputerModelItemData::Category::cat_user_directory) {
        int sz = par->view()->iconSize().width() * 2 + 24;
        return QSize(sz, sz);
    }
    const int iconsize = par->view()->iconSize().width();
    const int topmargin = iconsize / 2 + 2;
    const int leftmargin = iconsize / 3 + 3;
    const int text_max_width = int(iconsize * 3.75);
    const int spacing = iconsize / 3 + 1;
    const int rightmargin = iconsize / 3 + 9;
    return QSize(leftmargin + iconsize + spacing + text_max_width + rightmargin, topmargin + iconsize + topmargin);
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
    QRegExp regx("[^\\\\/\':\\*\\?\"<>|%&]+"); //屏蔽特殊字符
    QValidator *validator = new QRegExpValidator(regx, le);
    le->setValidator(validator);
    le->setMaxLength(index.data(ComputerModel::EditorLengthRole).toInt());

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
}

void ComputerViewItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit *le = qobject_cast<QLineEdit *>(editor);
    model->setData(index, le->text());
}

void ComputerViewItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    if (index.data(ComputerModel::DataRoles::ICategoryRole) == ComputerModelItemData::Category::cat_widget) {
        editor->setGeometry(option.rect);
        return;
    }
    QRect textrect = option.rect;
    const int iconsize = par->view()->iconSize().width();
    const int topmargin = iconsize / 2 + 2;
    const int leftmargin = iconsize / 3 + 3;
    const int text_max_width = int(iconsize * 3.75);
    const int spacing = iconsize / 3 + 1;
    textrect.setLeft(option.rect.left() + leftmargin + iconsize + spacing + 1);
    textrect.setWidth(text_max_width);
    textrect.setTop(option.rect.top() + topmargin - 2);
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


