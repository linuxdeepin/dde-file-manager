// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/private/urlpushbutton_p.h"
#include "views/urlpushbutton.h"
#include "views/crumbbar.h"
#include "views/folderlistwidget.h"
#include "utils/crumbinterface.h"
#include "utils/crumbmanager.h"

#include <dfm-io/denumerator.h>
#include <dfm-base/utils/fileutils.h>

#include <DStyle>
#include <DStyleOptionButton>

#include <QStyle>
#include <QStyleOptionViewItem>
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QTimer>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE

static constexpr int kBorderWidth = 4;
static constexpr int kFolderborderRectRadius { 6 };
static constexpr int kFolderButtonRectRadius { 4 };

static QString getIconName(const CrumbData &c)
{
    QString iconName = c.iconName;

    // 如果不是以"dfm_"开头且不包含"-symbolic"，则添加"-symbolic"后缀
    if (!iconName.isEmpty() && !iconName.startsWith("dfm_") && !iconName.contains("-symbolic"))
        iconName.append("-symbolic");

    return iconName;
}

UrlPushButtonPrivate::UrlPushButtonPrivate(UrlPushButton *qq)
    : QObject(qq), q(qq), active(true)
{
    font = q->font();
    initConnect();
}

UrlPushButtonPrivate::~UrlPushButtonPrivate()
{
}

void UrlPushButtonPrivate::initConnect()
{
    connect(q, &UrlPushButton::selectSubDirs, this, &UrlPushButtonPrivate::onSelectSubDirs);
    connect(q, &UrlPushButton::customContextMenuRequested, this, &UrlPushButtonPrivate::onCustomContextMenu);
}

int UrlPushButtonPrivate::arrowWidth() const
{
    int width = 0;
    if (q->icon().isNull()) {
        // 计算箭头宽度
        QFont adjustedFont(q->font());
        adjustedFont.setBold(subDir.isEmpty());
        int fontWidth = QFontMetrics(adjustedFont).horizontalAdvance("/") * 2;
        if (fontWidth < 4) {
            fontWidth = 4;
        }
        width = fontWidth;
    }

    return width;
}

bool UrlPushButtonPrivate::isAboveArrow(int x) const
{
    const bool leftToRight = (q->layoutDirection() == Qt::LeftToRight);
    bool aboveArrow = false;
    if (q->icon().isNull())
        aboveArrow = leftToRight ? (x >= q->width() - arrowWidth() - kBorderWidth) : (x < arrowWidth() + kBorderWidth);
    return aboveArrow;
}

bool UrlPushButtonPrivate::isTextClipped() const
{
    // 计算可用宽度
    int availableWidth = q->width() - 2 * kBorderWidth;
    if (!subDir.isEmpty() || stacked) {
        availableWidth -= arrowWidth() - kBorderWidth;
    }

    QFont adjustedFont(q->font());
    adjustedFont.setBold(subDir.isEmpty());
    return QFontMetrics(adjustedFont).horizontalAdvance(q->text()) > availableWidth;
}

bool UrlPushButtonPrivate::isSubDir(int x) const
{
    bool subDirFlag = false;
    if (subDirVisible && !subDir.isEmpty() && isAboveArrow(x))
        subDirFlag = true;
    return subDirFlag;
}

void UrlPushButtonPrivate::updateWidth()
{
    const int oldMinWidth = q->minimumWidth();
    const int oldMaxWidth = q->maximumWidth();

    int minWidth = 0, maxWidth = 0;
    if (!q->icon().isNull()) {
        // 图标模式下的宽度
        minWidth = 2 * kBorderWidth + q->iconSize().width();
        maxWidth = minWidth;
    } else {
        // 文本模式下的宽度
        QFont adjustedFont(q->font());
        adjustedFont.setBold(subDir.isEmpty());
        int fontWidth = QFontMetrics(adjustedFont).horizontalAdvance(q->text());
        int defaultWinWidth = QFontMetrics(adjustedFont).horizontalAdvance("......");
        if (defaultWinWidth < kFolderMinWidth)
            defaultWinWidth = kFolderMinWidth;
        int buttonSize = 2 * kBorderWidth + fontWidth;
        if (arrowWidth() != 0) {
            buttonSize = buttonSize + arrowWidth() + kBorderWidth;
        }
        if (buttonSize >= defaultWinWidth && !subDir.isEmpty()) {
            minWidth = defaultWinWidth;
        } else {
            minWidth = buttonSize;
        }
        maxWidth = buttonSize;
    }
    if (oldMinWidth != minWidth) {
        q->setMinimumWidth(minWidth);
    }
    if (maxWidth != 0 && oldMaxWidth != maxWidth) {
        q->setMaximumWidth(maxWidth);
    }
}

QColor UrlPushButtonPrivate::foregroundColor() const
{
    QColor color = q->palette().color(q->foregroundRole());

    // 根据活动状态调整透明度
    int alpha = active ? 255 : 128;
    if (!active) {
        alpha -= alpha / 4;
    }
    color.setAlpha(alpha);

    return color;
}

bool UrlPushButtonPrivate::popupVisible() const
{
    bool visible = false;
    if (!menu.isNull() || (folderListWidget && folderListWidget->isVisible()))
        visible = true;
    return visible;
}

void UrlPushButtonPrivate::requestCompleteByUrl(const QUrl &url)
{
    completionStringList.clear();
    if (!crumbController || !crumbController->isSupportedScheme(url.scheme())) {
        if (crumbController) {
            crumbController->cancelCompletionListTransmission();
            crumbController->disconnect();
            crumbController->deleteLater();
        }
        crumbController = CrumbManager::instance()->createControllerByUrl(url);
        // Still not found? Then nothing here...
        if (!crumbController) {
            completionStringList.clear();
            fmDebug() << "Unsupported url / scheme for completion: " << url;
            return;
        }
        crumbController->setParent(q);
        // connections
        connect(crumbController, &CrumbInterface::completionFound, this, &UrlPushButtonPrivate::onCompletionFound);
        connect(crumbController, &CrumbInterface::completionListTransmissionCompleted, this, &UrlPushButtonPrivate::onCompletionCompleted);
    }
    crumbController->requestCompletionList(url);
}

void UrlPushButtonPrivate::activate()
{
    q->setActive(true);
}

void UrlPushButtonPrivate::onCustomContextMenu(const QPoint &point)
{
    if (!parent() || crumbDatas.isEmpty()) {
        fmWarning("No parent exists !!!");
        return;
    }
    CrumbBar *crumbBar = dynamic_cast<CrumbBar *>(q->parent());
    if (!crumbBar) {
        fmWarning("Parent is not a CrumbBar !!!");
        return;
    }

    crumbBar->setPopupVisible(true);
    // 创建并显示自定义上下文菜单
    menu.reset(new QMenu(q));
    crumbBar->customMenu(crumbDatas.last().url, menu.data());
    menu->exec(QCursor::pos());

    crumbBar->setPopupVisible(false);
    menu.reset();
    if (hoverFlag) {
        hoverFlag = false;
    }
    q->update();
}

void UrlPushButtonPrivate::onSelectSubDirs()
{
    if (!parent()) {
        fmWarning("No parent exists !!!");
        return;
    }
    auto crumbBar = dynamic_cast<CrumbBar *>(q->parent());
    if (!crumbBar) {
        fmWarning("Parent is not a CrumbBar !!!");
        return;
    }
    if (folderListWidget && folderListWidget->isVisible()) {
        folderListWidget->hide();
        return;
    }
    if (crumbDatas.isEmpty())
        return;
    crumbBar->setPopupVisible(true);
    const bool leftToRight = (q->layoutDirection() == Qt::LeftToRight);
    const int popupX = (leftToRight && !stacked) ? (q->width() - arrowWidth() - kBorderWidth) : 0;
    const QPoint popupPos = q->parentWidget()->mapToGlobal(q->geometry().bottomLeft() + QPoint(popupX, 0));

    if (!folderListWidget) {
        folderListWidget = new FolderListWidget(q);
        connect(folderListWidget, &FolderListWidget::urlButtonActivated, q, &UrlPushButton::urlButtonActivated);
    }
    QList<CrumbData> childDatas;
    if (!stacked) {
        // 堆叠时显示堆叠目录
        requestCompleteByUrl(crumbDatas.last().url);
    } else {
        folderListWidget->setFolderList(crumbDatas, stacked);
        folderListWidget->move(popupPos);
        folderListWidget->show();
    }

    QEventLoop eventLoop;
    connect(folderListWidget, &FolderListWidget::hidden, &eventLoop, &QEventLoop::quit);
    (void)eventLoop.exec(QEventLoop::DialogExec);

    crumbBar->setPopupVisible(false);
    if (hoverFlag) {
        hoverFlag = false;
        q->update();
    }
}

void UrlPushButtonPrivate::onCompletionFound(const QStringList &stringList)
{
    completionStringList += stringList;
}

void UrlPushButtonPrivate::onCompletionCompleted()
{
    if (!folderListWidget || completionStringList.isEmpty() || crumbDatas.isEmpty())
        return;
    QList<CrumbData> datas;
    for (int i = 0; i < completionStringList.size(); ++i) {
        CrumbData data;
        data.displayText = completionStringList[i];
        data.url = QUrl(crumbDatas.last().url.url() + '/' + completionStringList[i]);
        datas.append(data);
    }
    const bool leftToRight = (q->layoutDirection() == Qt::LeftToRight);
    const int popupX = (leftToRight && !stacked) ? (q->width() - arrowWidth() - kBorderWidth) : 0;
    const QPoint popupPos = q->parentWidget()->mapToGlobal(q->geometry().bottomLeft() + QPoint(popupX, 0));
    folderListWidget->setFolderList(datas, stacked);
    folderListWidget->move(popupPos);
    folderListWidget->show();
}

UrlPushButton::UrlPushButton(QWidget *parent)
    : DPushButton(parent), d(new UrlPushButtonPrivate(this))
{
    setFocusPolicy(Qt::TabFocus);
    setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    setMinimumHeight(24);
    setAttribute(Qt::WA_LayoutUsesWidgetRect);
    setAcceptDrops(true);
    setMouseTracking(true);
    setContextMenuPolicy(Qt::CustomContextMenu);

    setIconSize({ kFolderIconSize, kFolderIconSize });
}

UrlPushButton::~UrlPushButton() = default;

void UrlPushButton::setCrumbDatas(const QList<CrumbData> &datas, bool stacked)
{
    d->crumbDatas = datas;
    d->stacked = stacked;
    if (!icon().isNull()) {
        setIcon(QIcon());
    }
    if (d->stacked || datas.isEmpty()) {
        setText("...");
    } else {
        const CrumbData &data = datas.first();
        // 本地文件显示下拉选项
        d->subDirVisible = (FileUtils::isLocalDevice(data.url) && CrumbManager::instance()->isRegisted(data.url.scheme()));
        if (data.iconName.isEmpty()) {
            setText(data.displayText);
        } else {
            setIcon(QIcon::fromTheme(getIconName(data)));
        }
    }
    d->updateWidth();
}

void UrlPushButton::setActive(bool active)
{
    if (d->active != active) {
        active = active;
        update();
    }
}

bool UrlPushButton::isActive() const
{
    return d->active;
}

QList<CrumbData> UrlPushButton::crumbDatas() const
{
    return d->crumbDatas;
}

void UrlPushButton::focusInEvent(QFocusEvent *event)
{
    DPushButton::focusInEvent(event);
}

void UrlPushButton::focusOutEvent(QFocusEvent *event)
{
    DPushButton::focusOutEvent(event);
}

void UrlPushButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (d->font != font()) {
        d->font = font();
        d->updateWidth();
    }
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制悬停或菜单状态
    if (d->hoverFlag || d->popupVisible()) {
        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(palette().color(QPalette::Button));
        painter.drawRoundedRect(rect(), kFolderborderRectRadius, kFolderborderRectRadius);
        painter.restore();
    }

    if (!icon().isNull() && iconSize().isValid()) {
        // 绘制图标
        const QSize iconSize = this->iconSize();
        const QRect &borderRect = rect();
        QRect iconRect(QPoint(0, 0), iconSize);
        iconRect.moveCenter(borderRect.center());
        iconRect.moveLeft(kBorderWidth);
        icon().paint(&painter, iconRect, Qt::AlignCenter);
        return;
    }

    QFont adjustedFont(font());
    adjustedFont.setBold(d->subDir.isEmpty());
    painter.setFont(adjustedFont);

    const int buttonWidth = width();
    const int buttonHeight = height();

    const QColor fgColor = d->foregroundColor();

    int textWidth = buttonWidth;

    const bool leftToRight = (layoutDirection() == Qt::LeftToRight);

    const int arrowSize = d->arrowWidth();
    // 绘制箭头
    const int arrowX = leftToRight ? (buttonWidth - arrowSize - kBorderWidth) : kBorderWidth;
    const int arrowY = (buttonHeight - arrowSize) / 2;

    QStyleOption option;
    option.initFrom(this);
    option.rect = QRect(arrowX, arrowY, arrowSize, arrowSize);
    if (d->subDirVisible && !d->subDir.isEmpty() && !d->stacked && (d->hoverFlag || d->popupVisible())) {
        painter.save();
        // QColor hoverColor = palette().color(QPalette::HighlightedText);
        QColor hoverColor = palette().color(QPalette::Text);
        hoverColor.setAlphaF(0.2);
        painter.setPen(Qt::NoPen);
        painter.setBrush(hoverColor);
        painter.drawRoundedRect(option.rect.adjusted(-kFolderButtonRectRadius / 2, -kFolderButtonRectRadius / 2,
                                                     kFolderButtonRectRadius / 2, kFolderButtonRectRadius / 2),
                                kFolderButtonRectRadius, kFolderButtonRectRadius);
        painter.restore();
        style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &option, &painter, this);
    } else {
        painter.save();
        if (!d->stacked && d->subDir.isEmpty())
            painter.setPen(palette().highlight().color());
        painter.drawText(option.rect, Qt::AlignCenter, "/");
        painter.restore();
    }
    textWidth -= (option.rect.width() + 2 * kBorderWidth);

    painter.setPen(fgColor);
    const bool clipped = d->isTextClipped();
    int textX = kBorderWidth;
    if (!leftToRight)
        textX += arrowSize;
    const QRect textRect(textX, 0, textWidth, buttonHeight);
    if (clipped) {
        // 文本被裁剪时添加渐变效果
        QColor bgColor = fgColor;
        bgColor.setAlpha(0);
        QLinearGradient gradient(textRect.topLeft(), textRect.topRight());
        if (leftToRight) {
            gradient.setColorAt(0.8, fgColor);
            gradient.setColorAt(1.0, bgColor);
        } else {
            gradient.setColorAt(0.0, bgColor);
            gradient.setColorAt(0.2, fgColor);
        }

        QPen pen;
        pen.setBrush(QBrush(gradient));
        painter.setPen(pen);
    }

    painter.save();
    const int textFlags = Qt::AlignVCenter | Qt::AlignLeft;
    if (!d->stacked && d->subDir.isEmpty())
        painter.setPen(palette().highlight().color());
    painter.setClipRect(textRect);
    painter.drawText(textRect, textFlags, text());
    painter.restore();
}

void UrlPushButton::mousePressEvent(QMouseEvent *event)
{
    DPushButton::mousePressEvent(event);
    if ((d->isSubDir(event->x()) || d->stacked) && (event->button() == Qt::LeftButton)) {
        // 延迟发送选择子目录信号
        QTimer::singleShot(0, this, [this]() {
            Q_EMIT selectSubDirs();
        });
    }
    if (!d->hoverFlag) {
        d->hoverFlag = true;
        update();
    }
}

void UrlPushButton::mouseMoveEvent(QMouseEvent *event)
{
    DPushButton::mouseMoveEvent(event);
    if (!d->hoverFlag) {
        d->hoverFlag = true;
        update();
    }
}

void UrlPushButton::mouseReleaseEvent(QMouseEvent *event)
{
    // 非堆叠时才可以跳转
    if (!d->isSubDir(event->x()) && !d->stacked && (event->button() == Qt::LeftButton) && !d->crumbDatas.isEmpty()) {
        Q_EMIT urlButtonActivated(d->crumbDatas.last().url);
    }
    DPushButton::mouseReleaseEvent(event);
    if (!d->hoverFlag) {
        d->hoverFlag = true;
        update();
    }
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void UrlPushButton::enterEvent(QEnterEvent *event)
#else
void UrlPushButton::enterEvent(QEvent *event)
#endif
{
    DPushButton::enterEvent(event);
    if (d->isTextClipped()) {
        setToolTip(text());
    }
    if (!d->hoverFlag) {
        d->hoverFlag = true;
        update();
    }
}

void UrlPushButton::leaveEvent(QEvent *event)
{
    DPushButton::leaveEvent(event);

    setToolTip(QString());
    if (d->hoverFlag && !d->popupVisible()) {
        d->hoverFlag = false;
    }
    update();
}

void UrlPushButton::setActiveSubDirectory(const QString &subDir)
{
    d->subDir = subDir;
    d->updateWidth();
    updateGeometry();
    update();
}

QString UrlPushButton::activeSubDirectory() const
{
    return d->subDir;
}
