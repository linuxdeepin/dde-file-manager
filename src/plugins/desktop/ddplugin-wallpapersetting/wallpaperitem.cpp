// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wallpaperitem.h"
#include "wallpaperlist.h"
#include "thumbnailmanager.h"
#include "editlabel.h"

#include <QUrl>
#include <QPushButton>
#include <QEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QGridLayout>
#include <QtSvg/QSvgRenderer>

using namespace ddplugin_wallpapersetting;

WrapperWidget::WrapperWidget(QWidget *parent)
    : QWidget(parent)
{
}

void WrapperWidget::setPixmap(const QPixmap &pix)
{
    pixmap = pix;
}

void WrapperWidget::paintEvent(QPaintEvent *event)
{
    if (pixmap.isNull())
        return;

    QRect pixmapGeometry = pixmapBoxGeometry;

    pixmapGeometry.setSize(pixmap.size());
    pixmapGeometry.moveCenter(pixmapBoxGeometry.center());

    QPainter pa(this);

    pa.setOpacity(opacity());
    pa.drawPixmap(pixmapGeometry.topLeft(), pixmap, pixmapGeometry.united(event->rect()));
    return;
}

WallpaperItem::WallpaperItem(QWidget *parent)
    : QFrame(parent), thumbnailerWatcher(new QFutureWatcher<QPixmap>(this))
{
    init();
}

WallpaperItem::~WallpaperItem()
{
}

QString WallpaperItem::itemData() const
{
    return itData;
}

void WallpaperItem::setItemData(const QString &data)
{
    itData = data;
}

QString WallpaperItem::sketch() const
{
    return sketchPath;
}

void WallpaperItem::setSketch(const QString &url)
{
    sketchPath = url;
}

bool WallpaperItem::enableThumbnail() const
{
    return enablethumbnail;
}

void WallpaperItem::setEnableThumbnail(bool enbale)
{
    enablethumbnail = enbale;
}

bool WallpaperItem::isDeletable() const
{
    return deletable;
}

void WallpaperItem::setDeletable(bool del)
{
    deletable = del;
}

void WallpaperItem::setOpacity(qreal opacity)
{
    const double tempEps = 1e-6;
    if (qAbs(wrapper->opacity() - opacity) < tempEps)
        return;

    wrapper->setOpacity(opacity);
    wrapper->update();
}

void WallpaperItem::slideUp()
{
    if (wrapper->y() < 0 && downAnim->state() == QAbstractAnimation::Stopped)
        return;

    upAnim->setStartValue(QPoint(0, 0));
    upAnim->setEndValue(QPoint(0, -WallpaperList::kItemHeight / 2 * buttonLayout->rowCount()));
    upAnim->start();
    //当按钮浮起，设置按钮可获得焦点
    for (int i = 0; i < buttonLayout->count(); i++) {
        buttonLayout->itemAt(i)->widget()->setFocusPolicy(Qt::StrongFocus);
    }
    //设置第一个按钮为焦点
    focusOnFirstButton();
}

void WallpaperItem::slideDown()
{
    if (wrapper->y() >= 0 && upAnim->state() == QAbstractAnimation::Stopped)
        return;

    downAnim->setStartValue(QPoint(0, -WallpaperList::kItemHeight / 2 * buttonLayout->rowCount()));
    downAnim->setEndValue(QPoint(0, 0));
    downAnim->start();
    //当按钮下沉，设置按钮不可获得焦点
    for (int i = 0; i < buttonLayout->count(); i++) {
        buttonLayout->itemAt(i)->widget()->setFocusPolicy(Qt::NoFocus);
    }
}

void WallpaperItem::renderPixmap()
{
    if (enablethumbnail) {
        refindPixmap();
    } else {
        QIcon icon(sketch());
        // scale to full up
        {
            auto ratio = devicePixelRatioF();
            const int itemWidth = static_cast<int>(WallpaperList::kItemWidth * ratio);
            const int itemHeight = static_cast<int>(WallpaperList::kItemHeight * ratio);

            const QRect r(0, 0, itemWidth, itemHeight);
            auto pix = icon.pixmap(window()->windowHandle(), QSize(WallpaperList::kItemWidth, WallpaperList::kItemHeight));
            pix = pix.scaled(itemWidth, itemHeight, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            pix.setDevicePixelRatio(ratio);
            if (pix.width() > itemWidth || pix.height() > itemHeight)
                pix = pix.copy(QRect(pix.rect().center() - r.center(), QSize(itemWidth, itemHeight)));
            wrapper->setPixmap(pix);
        }

        wrapper->update();
    }
}

QRect WallpaperItem::contentGeometry() const
{
    return wrapper->boxGeometry();
}

QPushButton *WallpaperItem::addButton(const QString &id, const QString &text, const int btnWidth, int row, int column, int rowSpan, int columnSpan)
{
    QPushButton *button = new QPushButton(this);
    button->setFixedWidth(btnWidth);
    auto fts = button->fontMetrics();
    auto elidedText = fts.elidedText(text, Qt::ElideMiddle, button->width() - 16);
    if (elidedText != text)
        button->setToolTip(text);

    button->setText(elidedText);
    button->installEventFilter(this);
    button->setFocusPolicy(Qt::NoFocus);
    buttons.insert(button, id);
    connect(button, &QPushButton::clicked, this, &WallpaperItem::onButtonClicked);

    buttonLayout->addWidget(button, row, column, rowSpan, columnSpan, Qt::AlignHCenter | Qt::AlignTop);

    return button;
}

/*!
 * \brief WallpaperItem::setEntranceIconOfSettings, add Settings entrance icon for custom screensavers
 * \param id, the name of custom screensaver with custom configuration
 */
void WallpaperItem::setEntranceIconOfSettings(const QString &id)
{
    EditLabel *editLabel = new EditLabel(wrapper);
    const QSize fixSize(36, 36);
    editLabel->setFixedSize(fixSize);
    // scale
    {
        auto ratio = devicePixelRatioF();
        QPixmap pmap(fixSize * ratio);
        pmap.fill(Qt::transparent);
        {
            QSvgRenderer svg(QString(":/images/edit.svg"));
            QPainter painter(&pmap);
            svg.render(&painter, QRect(QPoint(0, 0), pmap.size()));
        }

        pmap.setDevicePixelRatio(ratio);
        editLabel->setPixmap(pmap);
    }

    const QSize visibleSize(28, 28);
    editLabel->setHotZoom(QRect(fixSize.width() - visibleSize.width(), 0,
                                visibleSize.width(), visibleSize.height()));   // the edit.svg has 28pix area that is visible.
    editLabel->move(wrapper->width() - fixSize.width(), 0);

    connect(editLabel, &EditLabel::editLabelClicked, this, [this, id] {
        emit buttonClicked(this, id);
    });
}

void WallpaperItem::mousePressEvent(QMouseEvent *event)
{
    if (event->buttons() == Qt::LeftButton)
        emit pressed(this);
}

void WallpaperItem::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up: {
        QWidget *w = focusWidget();
        for (int i = 0; i < buttonLayout->count(); ++i) {
            if (buttonLayout->itemAt(i)->widget() == w) {
                if (0 == i) {
                    focusOnLastButton();   // first,go to last
                } else {
                    buttonLayout->itemAt(--i)->widget()->setFocus();
                }
                break;
            }
        }
        break;
    }
    case Qt::Key_Down: {
        QWidget *w = focusWidget();
        for (int i = 0; i < buttonLayout->count(); ++i) {
            if (buttonLayout->itemAt(i)->widget() == w) {
                if (buttonLayout->count() - 1 == i)
                    focusOnFirstButton();   // last,go to first
                else
                    buttonLayout->itemAt(++i)->widget()->setFocus();
                break;
            }
        }
        break;
    }
    default:
        break;
    }

    QWidget::keyPressEvent(event);
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
void WallpaperItem::enterEvent(QEvent *event)
#else
void WallpaperItem::enterEvent(QEnterEvent *event)
#endif
{
    Q_UNUSED(event);
    emit hoverIn(this);
}

void WallpaperItem::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    emit hoverOut(this);
}

void WallpaperItem::resizeEvent(QResizeEvent *event)
{
    const qreal ratio = devicePixelRatioF();

    const QPoint &offset = QPoint((event->size().width() - WallpaperList::kItemWidth) / 2,
                                  (event->size().height() - WallpaperList::kItemHeight) / 2);

    wrapper->setFixedWidth(width());
    wrapper->setBoxGeometry(QRect(offset * ratio, QSize(static_cast<int>(WallpaperList::kItemWidth * ratio), static_cast<int>(WallpaperList::kItemHeight * ratio))));

    QFrame::resizeEvent(event);
}

bool WallpaperItem::eventFilter(QObject *watched, QEvent *event)
{
    QPushButton *btn = dynamic_cast<QPushButton *>(watched);
    if (btn && buttons.contains(btn)) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            switch (keyEvent->key()) {
            case Qt::Key_Tab:
                emit tab(this);
                return true;
            case Qt::Key_Backtab:
                emit backtab(this);
                return true;
            case Qt::Key_Space:
            case Qt::Key_Enter:
            case Qt::Key_Return:
                emit btn->clicked();
                return true;
            default:
                // button ignore event,and WallpaperItem will handle it
                event->ignore();
                return true;
            }
        }
    }

    return QFrame::eventFilter(watched, event);
}

void WallpaperItem::refindPixmap()
{
    ThumbnailManager *tnm = ThumbnailManager::instance(devicePixelRatioF());

    connect(tnm, &ThumbnailManager::thumbnailFounded, this, &WallpaperItem::onThumbnailFounded, Qt::UniqueConnection);
    connect(tnm, &ThumbnailManager::findAborted, this, &WallpaperItem::onFindAborted, Qt::UniqueConnection);

    tnm->find(thumbnailKey());
}

void WallpaperItem::focusOnLastButton()
{
    if (buttonLayout->count() != 0)
        buttonLayout->itemAt(buttonLayout->count() - 1)->widget()->setFocus();
}

void WallpaperItem::focusOnFirstButton()
{
    if (buttonLayout->count() != 0)
        buttonLayout->itemAt(0)->widget()->setFocus();
}

QString WallpaperItem::thumbnailKey() const
{
    return QUrl::toPercentEncoding(sketch());
}

void WallpaperItem::init()
{
    setAttribute(Qt::WA_TranslucentBackground);
    setFrameShape(QFrame::NoFrame);
    setFocusPolicy(Qt::NoFocus);

    wrapper = new WrapperWidget(this);
    wrapper->setFixedSize(WallpaperList::kItemWidth, WallpaperList::kItemHeight * 2);
    wrapper->setAttribute(Qt::WA_TranslucentBackground);

    QFrame *buttonArea = new QFrame(wrapper);
    buttonArea->setFixedSize(WallpaperList::kItemWidth, WallpaperList::kItemHeight);
    buttonArea->move(0, WallpaperList::kItemHeight);

    buttonLayout = new QGridLayout(buttonArea);
    buttonLayout->setHorizontalSpacing(6);
    buttonLayout->setVerticalSpacing(8);
    buttonLayout->setContentsMargins(4, 10, 4, 10);

    upAnim = new QPropertyAnimation(wrapper, "pos", this);
    upAnim->setDuration(300);

    downAnim = new QPropertyAnimation(wrapper, "pos", this);
    downAnim->setDuration(300);
}

void WallpaperItem::onButtonClicked()
{
    QPushButton *btn = dynamic_cast<QPushButton *>(sender());
    if (btn && buttons.contains(btn)) {
        emit buttonClicked(this, buttons.value(btn));
    }
}

void WallpaperItem::onThumbnailFounded(const QString &key, QPixmap pixmap)
{
    if (key != thumbnailKey())
        return;

    const qreal ratio = devicePixelRatioF();
    pixmap.setDevicePixelRatio(ratio);
    wrapper->setPixmap(pixmap);
    wrapper->update();
}

void WallpaperItem::onFindAborted(const QQueue<QString> &list)
{
    if (list.contains(thumbnailKey())) {
        refindPixmap();
    }
}
