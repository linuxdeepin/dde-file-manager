// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "computeritemdelegate.h"
#include "views/computerview.h"
#include "models/computermodel.h"
#include "utils/computerdatastruct.h"

#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/device/devicealiasmanager.h>

#include <DApplication>
#include <DPaletteHelper>
#include <DPalette>
#include <DPaletteHelper>
#include <DGuiApplicationHelper>

#include <QHelpEvent>
#include <QToolTip>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsBlurEffect>

namespace dfmplugin_computer {

namespace {
const int kIconLabelSpacing { 10 };
const int kIconLeftMargin { 10 };
const int kContentRightMargin { 20 };

const char *const kRegPattern { "^[^\\.\\\\/\':\\*\\?\"<>|%&][^\\\\/\':\\*\\?\"<>|%&]*" };

const int kSplitterLineHeight { 36 };
const int kSmallItemWidth { 108 };
const int kSmallItemHeight { 138 };

const int kLargeItemWidth { 284 };
const int kLargeItemHeight { 84 };
}   // namespace

//!
//! \brief 一些特殊的字体和 CESI_*_GB* 的字体在计算机页面重命名时，显示位置偏上
//! 因此针对这些字体使用 top-margin 调整，确保文字垂直居中
//! \return top-margin of lineeditor
//!
static int editorMarginTop(const QString &family)
{
    int margin = 0;
    // TODO(xust) 一些特殊的字体和 CESI_*_GB* 的字体在计算机页面重命名时，显示位置偏上
    //    if (dfm_util::isContains(family, QString("Unifont"), QString("WenQuanYi Micro Hei")))
    //        margin = 4;
    //    else if (family.startsWith("CESI")) {
    //        if (family.endsWith("GB2312") || family.endsWith("GB13000") || family.endsWith("GB18030"))
    //            margin = 4;
    //    }
    return margin;
}

static QPixmap getScaledPixmap(const QIcon &icon, int size, QPainter *painter)
{
    const qreal pixelRatio = painter->device()->devicePixelRatio();
    QPixmap pm = icon.pixmap(size * pixelRatio);
    pm.setDevicePixelRatio(pixelRatio);
    return pm;
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
    if (!index.data(ComputerModel::kItemVisibleRole).toBool())
        return;

    painter->setRenderHint(QPainter::RenderHint::Antialiasing);

    ComputerItemData::ShapeType type = ComputerItemData::ShapeType(index.data(ComputerModel::DataRoles::kItemShapeTypeRole).toInt());
    switch (type) {
    case ComputerItemData::kSplitterItem:
        paintSplitter(painter, option, index);
        break;
    case ComputerItemData::kSmallItem:
        paintSmallItem(painter, option, index);
        break;
    case ComputerItemData::kLargeItem:
        paintLargeItem(painter, option, index);
        break;
    case ComputerItemData::kWidgetItem:
        paintCustomWidget(painter, option, index);
        break;
    default:
        break;
    }
}

QSize ComputerItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto itemType = ComputerItemData::ShapeType(index.data(ComputerModel::kItemShapeTypeRole).toInt());
    switch (itemType) {
    case ComputerItemData::kSplitterItem:
        return QSize(view->width() - 30, kSplitterLineHeight);
    case ComputerItemData::kSmallItem:
        return QSize(kSmallItemWidth, kSmallItemHeight);
    case ComputerItemData::kLargeItem:
        return QSize(kLargeItemWidth, kLargeItemHeight);
    case ComputerItemData::kWidgetItem:
        return static_cast<ComputerItemData *>(index.internalPointer())->widget->size();
    default:
        return QSize();
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
    // 重新设置调色板颜色值，使得背景色正确渲染，而不是保持透明状态。
    auto p = editor->palette();
    p.setColor(QPalette::Button, p.color(QPalette::Button));
    editor->setPalette(p);

    if (!NPDeviceAliasManager::instance()->canSetAlias(index.data(ComputerModel::kRealUrlRole).toUrl())) {
        QRegularExpression regx(kRegPattern);
        QValidator *validator = new QRegularExpressionValidator(regx, editor);
        editor->setValidator(validator);
    }

    int maxLengthWhenRename = index.data(ComputerModel::kDeviceNameMaxLengthRole).toInt();
    connect(editor, &QLineEdit::textChanged, this, [maxLengthWhenRename, editor](const QString &text) {
        if (!editor)
            return;

        auto newLabel = text;
        if (newLabel.toUtf8().length() <= maxLengthWhenRename)
            return;

        QSignalBlocker blocker(editor);
        while (newLabel.toUtf8().length() > maxLengthWhenRename)
            newLabel.chop(1);
        int cursorPos = editor->cursorPosition();
        editor->setText(newLabel);
        editor->setCursorPosition(cursorPos);
    });
    connect(editor, &QLineEdit::destroyed, this, [this] {
        view->model()->setData(editingIndex, false, ComputerModel::kItemIsEditingRole);
        editingIndex = QModelIndex();
    });

    view->model()->setData(editingIndex, true, ComputerModel::kItemIsEditingRole);

    // this if for avoiding trigger edit event infinitely when mouse click out of lineedit but in the area of item.
    // if item is selected, then press it, the edit event is triggered if `QListView::SelectedClicked` is setted for view.
    view->clearSelection();
    return editor;
}

void ComputerItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto currEditor = qobject_cast<QLineEdit *>(editor);
    if (currEditor)
        currEditor->setText(index.data(ComputerModel::kEditDisplayTextRole).toString());
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

    textRect.setLeft(option.rect.left() + kIconLeftMargin + IconSize + kIconLabelSpacing);
    textRect.setWidth(180);
    textRect.setTop(option.rect.top() + 10);
    textRect.setHeight(view->fontInfo().pixelSize() * 2);

    editor->setGeometry(textRect);
}

bool ComputerItemDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::ToolTip) {
        if (index.data(ComputerModel::DataRoles::kDisplayNameIsElidedRole).toBool())
            QToolTip::showText(event->globalPos(), index.data(Qt::DisplayRole).toString(), view, option.rect);
        else
            QToolTip::hideText();
        return true;
    }

    return QStyledItemDelegate::helpEvent(event, view, option, index);
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
    fnt.setPixelSize(QFontInfo(fnt).pixelSize() + 6);
    fnt.setWeight(QFont::Medium);
    painter->setFont(fnt);
    painter->setPen(qApp->palette().color(QPalette::ColorRole::Text));
    painter->drawText(option.rect, Qt::AlignBottom, index.data(Qt::ItemDataRole::DisplayRole).toString());
}

void ComputerItemDelegate::paintCustomWidget(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(index);
}

void ComputerItemDelegate::paintSmallItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    prepareColor(painter, option, index);

    // draw round rect
    painter->drawRoundedRect(option.rect.adjusted(1, 1, -1, -1), 18, 18);

    const int IconSize = view->iconSize().width();
    const int TopMargin = 16;
    const int LeftMargin = 22;

    const auto &icon = index.data(Qt::ItemDataRole::DecorationRole).value<QIcon>();
    QRect iconRect = option.rect;
    iconRect.setSize(view->iconSize());
    iconRect.moveTopLeft(iconRect.topLeft() + QPoint(LeftMargin, TopMargin));

    auto pm = getScaledPixmap(icon, IconSize, painter);
    //    const int ShadowBlurRadisu = 4;
    //    painter->drawPixmap(tl + QPoint(-ShadowBlurRadisu, -ShadowBlurRadisu + 2), renderBlurShadow(pm, ShadowBlurRadisu));
    painter->drawPixmap(iconRect, pm);

    QFont fnt(view->font());
    fnt.setPixelSize(QFontInfo(fnt).pixelSize());
    fnt.setWeight(QFont::Medium);
    painter->setFont(fnt);

    QFontMetrics fm(fnt);

    const int kTextMaxWidth = option.rect.width();
    const QString &itemName = index.data(Qt::DisplayRole).toString();
    const QString &kElidedText = fm.elidedText(itemName, Qt::ElideMiddle, kTextMaxWidth);
    view->model()->setData(index, kElidedText != itemName, ComputerModel::DataRoles::kDisplayNameIsElidedRole);

    const int kLabelWidth = fm.horizontalAdvance(kElidedText);
    const int kLabelTopMargin = 10;
    auto labelRect = QRect(option.rect.x() + (option.rect.width() - kLabelWidth) / 2, option.rect.y() + TopMargin + IconSize + kLabelTopMargin, kLabelWidth, 40);
    painter->setPen(qApp->palette().color(/*(option.state & QStyle::StateFlag::State_Selected) ? QPalette::ColorRole::BrightText : */ QPalette::ColorRole::Text));   // PO: no highlight
    painter->drawText(labelRect, Qt::AlignTop, kElidedText);
}

void ComputerItemDelegate::paintLargeItem(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    prepareColor(painter, option, index);

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
void ComputerItemDelegate::prepareColor(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    DPalette palette(DPaletteHelper::instance()->palette(option.widget));
    auto baseColor = palette.color(DPalette::ColorGroup::Active, DPalette::ColorType::ItemBackground);

    auto widgetColor = option.widget->palette().base().color();
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        widgetColor = DGuiApplicationHelper::adjustColor(widgetColor, 0, 0, 5, 0, 0, 0, 0);

    // paint a fake selection when editing.
    if ((option.state & QStyle::StateFlag::State_Selected)
        || (index == editingIndex && index.isValid())) {
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
    const int topMargin = (sizeHint(option, index).height() - IconSize) / 2;

    QRect iconRect = option.rect;
    iconRect.setSize(view->iconSize());
    iconRect.moveTopLeft(iconRect.topLeft() + QPoint(kIconLeftMargin, topMargin));

    auto pm = getScaledPixmap(icon, IconSize, painter);
    painter->drawPixmap(iconRect, pm);
}

void ComputerItemDelegate::drawDeviceLabelAndFs(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->setPen(qApp->palette().color(QPalette::ColorRole::Text));
    auto fnt = view->font();
    fnt.setPixelSize(QFontInfo(fnt).pixelSize() + 2);
    fnt.setWeight(QFont::Medium);
    painter->setFont(fnt);
    QFontMetrics fm(fnt);

    QString devName = index.data(Qt::DisplayRole).toString();
    auto fs = index.data(ComputerModel::DataRoles::kFileSystemRole).toString();
    int fsLabelWidth = fm.horizontalAdvance(fs.toUpper());

    const int IconSize = view->iconSize().width();
    const int TextMaxWidth = sizeHint(option, index).width() - IconSize - kIconLeftMargin - kIconLabelSpacing - kContentRightMargin;
    // if show-fs is enabled in setting, then add a 2pix spacing, else treat it as 0.
    DFMBASE_USE_NAMESPACE
    bool showFsTag = Application::instance()->genericAttribute(Application::GenericAttribute::kShowFileSystemTagOnDiskIcon).toBool();
    showFsTag &= !fs.isEmpty();
    if (showFsTag)
        fsLabelWidth += 2;
    else
        fsLabelWidth = 0;
    const auto &elideDevName = fm.elidedText(devName, Qt::ElideMiddle, TextMaxWidth - fsLabelWidth - 5);
    view->model()->setData(index, elideDevName != devName, ComputerModel::DataRoles::kDisplayNameIsElidedRole);

    // draw label
    QRect realPaintedRectForDevName;
    QRect preRectForDevName = option.rect;
    preRectForDevName.setLeft(option.rect.left() + kIconLeftMargin + IconSize + kIconLabelSpacing);
    preRectForDevName.setTop(option.rect.top() + 10);
    preRectForDevName.setHeight(fm.height());
    painter->setPen(qApp->palette().color(/*(option.state & QStyle::StateFlag::State_Selected) ? QPalette::ColorRole::BrightText : */ QPalette::ColorRole::Text));   // PO: no highlight

    int realHeight = fm.boundingRect(elideDevName).height();
    preRectForDevName.adjust(0, fm.height() - realHeight, 0, 0);   // make sure the text will not be clipped.
    painter->drawText(preRectForDevName, Qt::AlignVCenter, elideDevName, &realPaintedRectForDevName);

    // draw filesystem tag behind label
    if (showFsTag) {
        fnt.setWeight(QFont::ExtraLight);
        painter->setFont(fnt);
        QFontMetrics fm(fnt);

        // sets the paint rect
        auto fsTagRect = realPaintedRectForDevName;
        fsTagRect.setWidth(fsLabelWidth - 2);   // 2 pixel spacing is added above, so remove it.
        fsTagRect.setHeight(fm.height() - 4);
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
    fnt.setPixelSize(QFontInfo(fnt).pixelSize() - 2);
    fnt.setWeight(QFont::Normal);
    painter->setFont(fnt);
    painter->setPen(DPaletteHelper::instance()->palette(option.widget).color(DPalette::TextTips));

    const int IconSize = view->iconSize().width();
    // this is the rect of device name.
    QRect detailRect = option.rect;
    detailRect.setLeft(detailRect.left() + kIconLeftMargin + kIconLabelSpacing + IconSize);
    detailRect.setTop(detailRect.top() + 40);
    detailRect.setHeight(view->fontMetrics().height());

    qint64 sizeUsage = 0, sizeTotal = 0;
    bool totalSizeVisiable = index.data(ComputerModel::kTotalSizeVisiableRole).toBool();
    bool usedSizeVisiable = index.data(ComputerModel::kUsedSizeVisiableRole).toBool();
    bool showSize = totalSizeVisiable || usedSizeVisiable;
    if (showSize) {
        sizeUsage = index.data(ComputerModel::kSizeUsageRole).toLongLong();
        sizeTotal = index.data(ComputerModel::kSizeTotalRole).toLongLong();
        if (sizeUsage > sizeTotal) {
            fmWarning() << "size overflow!!!";
            sizeUsage = 0;
        }
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
        const int textMaxWidth = sizeHint(option, index).width() - IconSize - kIconLeftMargin - kIconLabelSpacing - kContentRightMargin;
        double usedRate = (sizeTotal == 0) ? 0 : (sizeUsage * 1.0 / sizeTotal);
        if (usedRate > 1)
            usedRate = 1.0;   // avoid overflow.
        if (usedRate < 0)
            usedRate = 0;

        QRect totalRect(QPoint(detailRect.x(), option.rect.y() + 64), QSize(textMaxWidth, 6));
        QRect usedRect = totalRect;
        usedRect.setRight(usedRect.left() + usedRect.width() * usedRate);

        QColor shadowColor;
        QLinearGradient grad(usedRect.topLeft(), usedRect.topRight());
        if (usedRate < 0.7) {
            grad.setColorAt(0, QColor(0x0081FF));
            grad.setColorAt(0.5, QColor(0x0081FF));
            grad.setColorAt(1, QColor(0x06BEFD));
            shadowColor = QColor(0, 129, 255, 255 * 0.4);
        } else if (usedRate < 0.9) {
            grad.setColorAt(0, QColor(0xFFAE00));
            grad.setColorAt(0.5, QColor(0xFFD007));
            grad.setColorAt(1, QColor(0xF6FF0D));
            shadowColor = QColor(248, 174, 44, 255 * 0.4);
        } else {
            grad.setColorAt(0, QColor(0xFF0000));
            grad.setColorAt(0.5, QColor(0xFF237A));
            grad.setColorAt(1, QColor(0xFF9393));
            shadowColor = QColor(255, 0, 83, 255 * 0.3);
        }

        painter->setPen(Qt::NoPen);

        if (usedRate != 0) {
            const int BlurRadius = 6;
            auto shadowRect = usedRect;
            shadowRect.adjust(-BlurRadius, -BlurRadius, BlurRadius, BlurRadius);
            shadowRect.moveTop(shadowRect.top() + 4);
            painter->drawPixmap(shadowRect, renderBlurShadow(usedRect.size(), shadowColor, BlurRadius));
        }

        painter->setBrush(getProgressTotalColor());
        painter->drawRoundedRect(totalRect, 3, 3);

        if (usedRate != 0) {
            painter->setBrush(grad);
            painter->drawRoundedRect(usedRect, 3, 3);
        }
    }

    QString deviceDescription = index.data(ComputerModel::kDeviceDescriptionRole).toString();
    if (!showSize && !progressVisiable && !deviceDescription.isEmpty()) {
        painter->drawText(detailRect, Qt::AlignLeft, deviceDescription);
    }
}

QPixmap ComputerItemDelegate::renderBlurShadow(const QSize &sz, const QColor &color, int blurRadius) const
{
    QPixmap pm(sz);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawRect(0, 0, sz.width(), sz.height());
    p.end();

    return renderBlurShadow(pm, blurRadius);
}

QPixmap ComputerItemDelegate::renderBlurShadow(const QPixmap &pm, int blurRadius) const
{
    auto size = pm.size();
    size.rwidth() += (2 * blurRadius);
    size.rheight() += (2 * blurRadius);

    QGraphicsBlurEffect *eff = new QGraphicsBlurEffect;
    eff->setBlurRadius(blurRadius);

    QGraphicsScene sc;
    QGraphicsPixmapItem item;
    item.setPixmap(pm);
    item.setGraphicsEffect(eff);
    sc.addItem(&item);

    QPixmap ret(size);
    ret.fill(Qt::transparent);
    QPainter pp(&ret);
    sc.render(&pp, QRectF(), QRectF(-blurRadius, -blurRadius, size.width(), size.height()));
    pp.end();
    delete eff;
    return ret;
}

QColor ComputerItemDelegate::getProgressTotalColor() const
{
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        return QColor(255, 255, 255, 25);
    }
    return QColor(0, 0, 0, 25);
}

}
