/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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
#include "computeritemdelegate.h"
#include "views/computerview.h"
#include "models/computermodel.h"
#include "utils/computerdatastruct.h"

#include "dfm-base/utils/fileutils.h"
#include "dfm-base/base/application/application.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DPalette>

#include <QLineEdit>
#include <QPainter>

DPCOMPUTER_BEGIN_NAMESPACE

namespace {
const int kLabelTotalSpacing { 6 };
const int kTotalProgressBarSpacing { 4 };
const int kIconLabelSpacing { 10 };

const int kItemTopMargin { 20 };
const int kItemBottomMargin { 20 };
const int kItemLeftMargin { 10 };
const int kItemRightMargin { 20 };

const char *const kRegPattern { "^[^\\.\\\\/\':\\*\\?\"<>|%&][^\\\\/\':\\*\\?\"<>|%&]*" };
}   // namespace

//!
//! \brief 一些特殊的字体和 CESI_*_GB* 的字体在计算机页面重命名时，显示位置偏上
//! 因此针对这些字体使用 top-margin 调整，确保文字垂直居中
//! \return top-margin of lineeditor
//!
static int editorMarginTop(const QString &family)
{
    int margin = 0;
    // TODO(xust)
    //    if (dfm_util::isContains(family, QString("Unifont"), QString("WenQuanYi Micro Hei")))
    //        margin = 4;
    //    else if (family.startsWith("CESI")) {
    //        if (family.endsWith("GB2312") || family.endsWith("GB13000") || family.endsWith("GB18030"))
    //            margin = 4;
    //    }
    return margin;
}

DWIDGET_USE_NAMESPACE

ComputerItemDelegate::ComputerItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
    view = qobject_cast<ComputerView *>(parent);
}

ComputerItemDelegate::~ComputerItemDelegate()
{
}

void ComputerItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setRenderHint(QPainter::RenderHint::Antialiasing);

    ComputerItemData::ShapeType type = ComputerItemData::ShapeType(index.data(ComputerModel::DataRoles::kItemShapeTypeRole).toInt());

    if (type == ComputerItemData::kSplitterItem) {
        paintSplitter(painter, option, index);
    } else if (type == ComputerItemData::kWidgetItem) {
        paintCustomWidget(painter, option, index);
    } else if (type == ComputerItemData::kSmallItem) {
        paintUserDirectory(painter, option, index);
    } else {   // paint devices include local/removable devices and app entry
        paintDevice(painter, option, index);
    }
}

QSize ComputerItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto itemType = ComputerItemData::ShapeType(index.data(ComputerModel::kItemShapeTypeRole).toInt());
    if (itemType == ComputerItemData::kWidgetItem) {
        return static_cast<ComputerItemData *>(index.internalPointer())->widget->size();
    } else if (itemType == ComputerItemData::kSplitterItem) {
        return QSize(view->width() - 12, 45);
    } else if (itemType == ComputerItemData::kSmallItem) {
        int borderLen = view->iconSize().width() * 2 + 24;
        return QSize(borderLen, borderLen);
    } else {
        const int IconSize = view->iconSize().width();
        const int TextMaxWidth = int(IconSize * 3.75);

        QFont fnt(view->font());
        fnt.setPixelSize(int(view->fontInfo().pixelSize() * 0.85));
        int capacityHeight = QFontMetrics(fnt).height();
        int devNameHeight = QFontMetrics(option.fontMetrics).height();
        int progressHeight = 6;
        int itemHeight = progressHeight + capacityHeight + devNameHeight
                + kLabelTotalSpacing + kTotalProgressBarSpacing
                + kItemTopMargin + kItemBottomMargin;
        int itemWidth = kItemLeftMargin + IconSize + kIconLabelSpacing + TextMaxWidth + kItemRightMargin;
        int iconHeight = kItemTopMargin + IconSize + kItemBottomMargin;
        return QSize(itemWidth, qMax(iconHeight, itemHeight));
    }
}

QWidget *ComputerItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    editingIndex = index;
    auto editor = new QLineEdit(parent);
    renameEditor = editor;

    int topMargin = editorMarginTop(option.font.family());
    editor->setFrame(false);
    editor->setTextMargins(0, topMargin, 0, 0);
    editor->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    QRegularExpression regx(kRegPattern);
    QValidator *validator = new QRegularExpressionValidator(regx, editor);
    editor->setValidator(validator);

    int maxLengthWhenRename = index.data(ComputerModel::kDeviceNameMaxLengthRole).toInt();
    connect(editor, &QLineEdit::textChanged, this, [maxLengthWhenRename, editor](const QString &text) {
        if (!editor)
            return;

        if (text.toUtf8().length() > maxLengthWhenRename) {
            QSignalBlocker blocker(editor);
            auto newLabel = text;
            newLabel.chop(1);
            editor->setText(newLabel);
        }
    });
    connect(editor, &QLineEdit::destroyed, this, [this, editor] {
        if (renameEditor == editor)
            editingIndex = QModelIndex();
    });

    return editor;
}

void ComputerItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto currEditor = qobject_cast<QLineEdit *>(editor);
    if (currEditor) {
        currEditor->setText(index.data(Qt::DisplayRole).toString());
        this->view->model()->setData(index, true, ComputerModel::kItemIsEditingRole);
    }
}

void ComputerItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto currEditor = qobject_cast<QLineEdit *>(editor);
    QString originalText = index.data(Qt::DisplayRole).toString();
    QString newText = currEditor->text();
    if (originalText != newText)
        model->setData(index, currEditor->text());
    model->setData(index, false, ComputerModel::kItemIsEditingRole);
}

void ComputerItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);

    if (index.model() && index.data(ComputerModel::kItemShapeTypeRole) == ComputerItemData::kWidgetItem) {
        editor->setGeometry(option.rect);
        return;
    }

    auto textRect = option.rect;
    const int IconSize = view->iconSize().width();
    const int TextMaxWidth = int(IconSize * 3.75);

    textRect.setLeft(option.rect.left() + kItemLeftMargin + IconSize + kIconLabelSpacing + 1);
    textRect.setWidth(TextMaxWidth);
    textRect.setTop(option.rect.top() + kItemTopMargin - 2);
    textRect.setHeight(view->fontInfo().pixelSize() * 2);

    editor->setGeometry(textRect);
}

void ComputerItemDelegate::closeEditor(ComputerView *view)
{
    if (!view || !editingIndex.isValid())
        return;

    QWidget *editor = view->indexWidget(editingIndex);
    if (!editor)
        return;

    QMetaObject::invokeMethod(this, "_q_commitDataAndCloseEditor",
                              Qt::DirectConnection, Q_ARG(QWidget *, editor));
}

void ComputerItemDelegate::paintSplitter(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFont fnt(view->font());
    fnt.setPixelSize(30);
    painter->setFont(fnt);
    painter->setPen(qApp->palette().color(QPalette::ColorRole::Text));
    painter->drawText(option.rect, 0, index.data(Qt::ItemDataRole::DisplayRole).toString());
}

void ComputerItemDelegate::paintCustomWidget(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(index);
}

void ComputerItemDelegate::paintUserDirectory(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    prepareColor(painter, option);

    // draw round rect
    painter->drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 18, 18);

    const int IconSize = view->iconSize().width() * 4 / 3;
    const int TopMargin = IconSize / 8 + 3;
    const int LeftMargin = IconSize / 4 + 12;
    const int LabelTopMargin = IconSize / 4;

    const int TextMaxWidth = option.rect.width() - 24;
    const QString &ElidedText = option.fontMetrics.elidedText(index.data(Qt::DisplayRole).toString(), Qt::ElideMiddle, TextMaxWidth);
    const int LabelWidth = view->fontMetrics().width(ElidedText);

    const auto &icon = index.data(Qt::ItemDataRole::DecorationRole).value<QIcon>();
    painter->drawPixmap(option.rect.x() + LeftMargin, option.rect.y() + TopMargin, icon.pixmap(IconSize));

    painter->setFont(view->font());
    painter->setPen(qApp->palette().color((option.state & QStyle::StateFlag::State_Selected) ? QPalette::ColorRole::HighlightedText : QPalette::ColorRole::Text));
    painter->drawText(option.rect.x() + (option.rect.width() - LabelWidth) / 2, option.rect.y() + TopMargin + IconSize + LabelTopMargin, ElidedText);
}

void ComputerItemDelegate::paintDevice(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    prepareColor(painter, option);

    // draw round rect
    painter->drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 18, 18);

    drawDeviceIcon(painter, option, index);
    drawDeviceLabelAndFs(painter, option, index);
    drawDeviceDetail(painter, option, index);
}

/*!
 * \brief ComputerItemDelegate::prepareColor sets the color of painter and brush.
 * \param painter
 * \param option
 */
void ComputerItemDelegate::prepareColor(QPainter *painter, const QStyleOptionViewItem &option) const
{
    DPalette palette(DApplicationHelper::instance()->palette(option.widget));
    auto baseColor = palette.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);

    auto widgetColor = option.widget->palette().base().color();
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        widgetColor = DGuiApplicationHelper::adjustColor(widgetColor, 0, 0, 5, 0, 0, 0, 0);

    if (option.state & QStyle::StateFlag::State_Selected) {
        baseColor.setAlpha(baseColor.alpha() + 30);
    } else if (option.state & QStyle::StateFlag::State_MouseOver) {
        if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
            baseColor = DGuiApplicationHelper::adjustColor(widgetColor, 0, 0, 5, 0, 0, 0, 0);
        else
            baseColor = baseColor.lighter();
    } else {   // non-selected and non-hover
        baseColor = widgetColor;
    }

    painter->setPen(baseColor);
    painter->setBrush(baseColor);
}

void ComputerItemDelegate::drawDeviceIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const auto &icon = index.data(Qt::ItemDataRole::DecorationRole).value<QIcon>();
    const int IconSize = view->iconSize().width();
    int y = option.rect.y() + (sizeHint(option, index).height() - IconSize) / 2;
    painter->drawPixmap(option.rect.x() + kItemLeftMargin, y, icon.pixmap(IconSize));
}

void ComputerItemDelegate::drawDeviceLabelAndFs(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setPen(qApp->palette().color(QPalette::ColorRole::Text));
    auto fnt = view->font();
    fnt.setWeight(66);
    painter->setFont(fnt);

    QString devName = index.data(Qt::DisplayRole).toString();
    auto fs = index.data(ComputerModel::DataRoles::kFileSystemRole).toString();
    int fsLabelWidth = view->fontMetrics().width(fs.toUpper());

    const int IconSize = view->iconSize().width();
    const int TextMaxWidth = sizeHint(option, index).width() - IconSize - kItemLeftMargin - kIconLabelSpacing - kItemRightMargin;
    // if show-fs is enabled in setting, then add a 2pix spacing, else treat it as 0.
    DFMBASE_USE_NAMESPACE
    bool showFsTag = Application::instance()->genericAttribute(Application::GenericAttribute::kShowFileSystemTagOnDiskIcon).toBool();
    showFsTag &= !fs.isEmpty();
    if (showFsTag)
        fsLabelWidth += 2;
    else
        fsLabelWidth = 0;
    devName = option.fontMetrics.elidedText(devName, Qt::ElideMiddle, TextMaxWidth - fsLabelWidth - 5);

    // draw label
    QRect realPaintedRectForDevName;
    QRect preRectForDevName = option.rect;
    preRectForDevName.setLeft(option.rect.left() + kItemLeftMargin + IconSize + kIconLabelSpacing);
    preRectForDevName.setTop(option.rect.top() + kItemTopMargin);
    preRectForDevName.setHeight(view->fontMetrics().height());
    painter->drawText(preRectForDevName, Qt::TextWrapAnywhere, devName, &realPaintedRectForDevName);

    // draw filesystem tag behind label
    if (showFsTag) {
        fnt.setWeight(12);
        painter->setFont(fnt);

        // sets the paint rect
        auto fsTagRect = realPaintedRectForDevName;
        fsTagRect.setWidth(fsLabelWidth - 2);   // 2 pixel spacing is added above, so remove it.
        const int FontPixSize = view->fontInfo().pixelSize();
        fsTagRect.setHeight(FontPixSize + 4);
        fsTagRect.moveLeft(realPaintedRectForDevName.right() + 12);   // 12 pixel spacing behind real painted rect for device name
        fsTagRect.moveBottom(realPaintedRectForDevName.bottom() - (realPaintedRectForDevName.height() - fsTagRect.height()) / 2);   // keep vertical center with label
        fsTagRect.adjust(-5, 0, 5, 0);

        QColor brushColor, penColor, borderColor;
        fs = fs.toUpper();
        if (fs == "EXT2" || fs == "EXT3" || fs == "EXT4" || fs == "VFAT") {
            brushColor = QColor(0xA1E4FF);
            penColor = QColor(0x0081B2);
            borderColor = QColor(0x73C7EE);
        } else if (fs == "NTFS" || fs == "FAT16" || fs == "FAT32" || fs == "EXFAT") {
            brushColor = QColor(0xFFDDA1);
            penColor = QColor(0x502504);
            borderColor = QColor(0xEEB273);
        } else {   // default
            brushColor = QColor(0xD2D2D2);
            penColor = QColor(0x5D5D5D);
            borderColor = QColor(0xA5A5A5);
        }

        // paint rect and draw text.
        painter->setBrush(brushColor);
        painter->setPen(borderColor);
        const qreal FsTagRectRadius = 7.5;
        painter->drawRoundedRect(fsTagRect, FsTagRectRadius, FsTagRectRadius);
        painter->setPen(penColor);
        painter->drawText(fsTagRect, Qt::AlignCenter, fs);
    }
}

void ComputerItemDelegate::drawDeviceDetail(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto fnt = view->font();
    fnt.setPixelSize(int(view->fontInfo().pixelSize() * 0.85));
    painter->setFont(fnt);
    painter->setPen(DApplicationHelper::instance()->palette(option.widget).color(DPalette::TextTips));

    const int IconSize = view->iconSize().width();
    // this is the rect of device name.
    QRect detailRect = option.rect;
    detailRect.setLeft(detailRect.left() + kItemLeftMargin + kIconLabelSpacing + IconSize);
    detailRect.setTop(detailRect.top() + kItemTopMargin);
    detailRect.setHeight(view->fontMetrics().height());
    // adjust it
    detailRect.setTop(detailRect.bottom() + kLabelTotalSpacing);
    detailRect.setHeight(QFontMetrics(fnt).height());

    qint64 sizeUsage = 0, sizeTotal = 0;
    bool totalSizeVisiable = index.data(ComputerModel::kTotalSizeVisiableRole).toBool();
    bool usedSizeVisiable = index.data(ComputerModel::kUsedSizeVisiableRole).toBool();
    bool showSize = totalSizeVisiable || usedSizeVisiable;
    if (showSize) {
        sizeUsage = index.data(ComputerModel::kSizeUsageRole).toLongLong();
        sizeTotal = index.data(ComputerModel::kSizeTotalRole).toLongLong();
        auto usage = DFMBASE_NAMESPACE::FileUtils::formatSize(sizeUsage);
        auto total = DFMBASE_NAMESPACE::FileUtils::formatSize(sizeTotal);
        QString sizeText;
        if (totalSizeVisiable && usedSizeVisiable)
            sizeText = QString("%1/%2").arg(usage).arg(total);
        else
            sizeText = total;
        painter->drawText(detailRect, Qt::AlignLeft, sizeText);
    }

    // paint progress bar
    bool progressVisiable = index.data(ComputerModel::kProgressVisiableRole).toBool();
    if (progressVisiable) {
        const int TextMaxWidth = sizeHint(option, index).width() - IconSize - kItemLeftMargin - kIconLabelSpacing - kItemRightMargin;
        QRect progressBarRect(QPoint(detailRect.x(), detailRect.bottom() + kTotalProgressBarSpacing), QSize(TextMaxWidth, 6));

        // preset the progress bar data
        QStyleOptionProgressBar progressBar;
        progressBar.textVisible = false;
        progressBar.rect = progressBarRect;
        progressBar.minimum = 0;
        progressBar.maximum = 10000;
        progressBar.progress = int(10000. * sizeUsage / sizeTotal);
        if (progressBar.progress > progressBar.maximum)
            progressBar.progress = progressBar.maximum;

        QColor usageColor;
        if (progressBar.progress < 7000)
            usageColor = QColor(0xFF0081FF);
        else if (progressBar.progress < 9000)
            usageColor = QColor(0xFFF8AE2C);
        else
            usageColor = QColor(0xFFFF6170);

        // paint progress bar
        progressBar.palette = option.widget ? option.widget->palette() : qApp->palette();
        progressBar.palette.setColor(QPalette::ColorRole::Highlight, usageColor);
        painter->setPen(Qt::PenStyle::NoPen);
        auto style = option.widget && option.widget->style() ? option.widget->style() : qApp->style();
        style->drawControl(QStyle::ControlElement::CE_ProgressBarGroove, &progressBar, painter, option.widget);
        style->drawControl(QStyle::ControlElement::CE_ProgressBarContents, &progressBar, painter, option.widget);
    }

    QString deviceDescription = index.data(ComputerModel::kDeviceDescriptionRole).toString();
    if (!showSize && !progressVisiable && !deviceDescription.isEmpty()) {
        painter->drawText(detailRect, Qt::AlignLeft, deviceDescription);
    }
}

DPCOMPUTER_END_NAMESPACE
