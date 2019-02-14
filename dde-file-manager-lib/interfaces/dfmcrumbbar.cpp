/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmcrumbbar.h"
#include "dfmcrumbitem.h"
#include "dfmcrumbmanager.h"

#include <QHBoxLayout>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>
#include <QApplication>

#include "views/dfilemanagerwindow.h"
#include "views/dfmaddressbar.h"
#include "views/themeconfig.h"
#include "dfmcrumbfactory.h"
#include "dfmcrumbinterface.h"
#include "dfmapplication.h"
#include "dfmevent.h"

#include <QButtonGroup>
#include <QDebug>

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

class DFMCrumbBarPrivate
{
    Q_DECLARE_PUBLIC(DFMCrumbBar)

public:
    DFMCrumbBarPrivate(DFMCrumbBar *qq);

    // UI
    QPushButton leftArrow;
    QPushButton rightArrow;
    QScrollArea crumbListScrollArea;
    QWidget *crumbListHolder;
    QHBoxLayout *crumbListLayout;
    QHBoxLayout *crumbBarLayout;
    QButtonGroup buttonGroup;
    QPoint clickedPos;
    DFMAddressBar *addressBar = nullptr;

    // Scheme support
    DFMCrumbInterface* crumbController = nullptr;

    // Misc
    bool clickableAreaEnabled = false;

    DFMCrumbBar *q_ptr = nullptr;

    void clearCrumbs();
    void checkArrowVisiable();
    void addCrumb(DFMCrumbItem* item);
    void updateController(const DUrl &url);
    void setClickableAreaEnabled(bool enabled);

private:
    void initUI();
    void initData();
    void initConnections();
};

DFMCrumbBarPrivate::DFMCrumbBarPrivate(DFMCrumbBar *qq)
    : q_ptr(qq)
{
    initData();
    initUI();
    initConnections();
}

/*!
 * \brief Remove all crumbs inside crumb bar.
 */
void DFMCrumbBarPrivate::clearCrumbs()
{
    leftArrow.hide();
    rightArrow.hide();

    if (crumbListLayout == nullptr) return;

    QList<QAbstractButton *> btns = buttonGroup.buttons();

    for (QAbstractButton* btn : btns) {
        crumbListLayout->removeWidget(btn);
        buttonGroup.removeButton(btn);
        btn->setParent(0);
        btn->close();
        btn->disconnect();
        // blumia: calling btn->deleteLater() wont send the delete event to eventloop
        //         don't know why... so we directly delete it here.
        delete btn;
    }
}

void DFMCrumbBarPrivate::checkArrowVisiable()
{
    if (crumbListHolder->width() >= crumbListScrollArea.width()) {
        if (!addressBar->isVisible()) {
            leftArrow.show();
            rightArrow.show();
        }

        QScrollBar* sb = crumbListScrollArea.horizontalScrollBar();
        leftArrow.setEnabled(sb->value() != sb->minimum());
        rightArrow.setEnabled(sb->value() != sb->maximum());
    } else {
        leftArrow.hide();
        rightArrow.hide();
    }
}

/*!
 * \brief Add crumb item into crumb bar.
 * \param item The item to be added into the crumb bar
 *
 * Notice: This shouldn't be called outside `updateCrumbs`.
 */
void DFMCrumbBarPrivate::addCrumb(DFMCrumbItem *item)
{
    Q_Q(DFMCrumbBar);

    crumbListLayout->addWidget(item);
    item->show();

    crumbListScrollArea.horizontalScrollBar()->setPageStep(crumbListHolder->width());

    checkArrowVisiable();

    q->connect(item, &DFMCrumbItem::clicked, q, [this, q]() {
        // change directory.
        DFMCrumbItem * item = qobject_cast<DFMCrumbItem*>(q->sender());
        Q_CHECK_PTR(item);
        emit q->crumbItemClicked(item);
    });
}

/*!
 * \brief Update the crumb controller hold by the crumb bar.
 *
 * \param url The url which controller should supported.
 */
void DFMCrumbBarPrivate::updateController(const DUrl &url)
{
    Q_Q(DFMCrumbBar);

    if (!crumbController || !crumbController->supportedUrl(url)) {
        if (crumbController) {
            crumbController->deleteLater();
        }
        crumbController = DFMCrumbManager::instance()->createControllerByUrl(url, q);
        // Not found? Then nothing here...
        if (!crumbController) {
            qDebug() << "Unsupported url / scheme: " << url;
        }
    }
}

void DFMCrumbBarPrivate::setClickableAreaEnabled(bool enabled)
{
    Q_Q(DFMCrumbBar);

    if (clickableAreaEnabled == enabled) return;

    clickableAreaEnabled = enabled;
    crumbListHolder->setContentsMargins(0, 0, (enabled ? 30 : 0), 0);

    q->update();
}

void DFMCrumbBarPrivate::initUI()
{
    Q_Q(DFMCrumbBar);

    // Address Bar
    addressBar->hide();

    // Crumbbar Widget
    q->setFixedHeight(24);

    // Arrows
    leftArrow.setObjectName("backButton");
    leftArrow.setFixedWidth(26);
    leftArrow.setFixedHeight(24);
    leftArrow.setFocusPolicy(Qt::NoFocus);
    rightArrow.setObjectName("forwardButton");
    rightArrow.setFixedWidth(26);
    rightArrow.setFixedHeight(24);
    rightArrow.setFocusPolicy(Qt::NoFocus);
    leftArrow.hide();
    rightArrow.hide();

    // Crumb List Layout
    crumbListScrollArea.setObjectName("DCrumbListScrollArea");
    crumbListScrollArea.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    crumbListScrollArea.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    crumbListScrollArea.setFocusPolicy(Qt::NoFocus);
    crumbListScrollArea.setContentsMargins(0, 0, 0, 0);
    crumbListScrollArea.setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);

    int clickableRightMargin = clickableAreaEnabled ? 30 : 0; // right 30 for easier click
    crumbListHolder = new QWidget();
    crumbListHolder->setObjectName("crumbListHolder");
    crumbListHolder->setContentsMargins(0, 0, clickableRightMargin, 0);
    crumbListHolder->setFixedHeight(q->height());
    crumbListHolder->installEventFilter(q);
    crumbListScrollArea.setWidget(crumbListHolder);

    crumbListLayout = new QHBoxLayout;
    crumbListLayout->setMargin(0);
    crumbListLayout->setSpacing(0);
    crumbListLayout->setAlignment(Qt::AlignLeft);
//    crumbListLayout->setSizeConstraint(QLayout::SetMinimumSize);
    crumbListLayout->setContentsMargins(0, 0, 0, 0);
    crumbListHolder->setLayout(crumbListLayout);

    // Crumb Bar Layout
    crumbBarLayout = new QHBoxLayout;
    crumbBarLayout->addWidget(&leftArrow);
    crumbBarLayout->addWidget(&crumbListScrollArea);
    crumbBarLayout->addWidget(&rightArrow);
    crumbBarLayout->setContentsMargins(0,0,0,0);
    crumbBarLayout->setSpacing(0);
    q->setLayout(crumbBarLayout);

    return;
}

void DFMCrumbBarPrivate::initData()
{
    Q_Q(DFMCrumbBar);
    addressBar = new DFMAddressBar(q);
    clickableAreaEnabled = DFMApplication::instance()->genericAttribute(DFMApplication::GA_ShowCsdCrumbBarClickableArea).toBool();
}

void DFMCrumbBarPrivate::initConnections()
{
    Q_Q(DFMCrumbBar);

    q->connect(&leftArrow, &QPushButton::clicked, q, [this]() {
        crumbListScrollArea.horizontalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
    });

    q->connect(&rightArrow, &QPushButton::clicked, q, [this]() {
        crumbListScrollArea.horizontalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
    });

    q->connect(crumbListScrollArea.horizontalScrollBar(), &QScrollBar::valueChanged, q, [this]() {
        checkArrowVisiable();
    });

    q->connect(addressBar, &DFMAddressBar::returnPressed, q, [q, this]() {
        emit q->addressBarContentEntered(addressBar->text());
    });

    q->connect(addressBar, &DFMAddressBar::escKeyPressed, q, [q, this]() {
        if (crumbController) {
            crumbController->processAction(DFMCrumbInterface::EscKeyPressed);
        }
    });

    q->connect(addressBar, &DFMAddressBar::lostFocus, q, [q, this]() {
        if (crumbController && !qobject_cast<DFileManagerWindow*>(q->window())->isAdvanceSearchBarVisible()) {
            crumbController->processAction(DFMCrumbInterface::AddressBarLostFocus);
        }
    });

    q->connect(addressBar, &DFMAddressBar::clearButtonPressed, q, [q, this] {
        if (crumbController) {
            crumbController->processAction(DFMCrumbInterface::ClearButtonPressed);
        }
    });

    q->connect(DFMApplication::instance(), &DFMApplication::csdClickableAreaAttributeChanged, q, [this](bool enabled) {
        setClickableAreaEnabled(enabled);
    });
}

/*!
 * \class DFMCrumbBar
 * \inmodule dde-file-manager-lib
 *
 * \brief DFMCrumbBar is the crumb bar widget of Deepin File Manager
 *
 * DFMCrumbBar is the crumb bar widget of Deepin File Manager, provide the interface to manage
 * crumb bar state. Crumb bar holds a group of DFMCrumbItem s, and crumb bar will be switch to
 * DFMAddressBar when clicking empty space at crumb bar.
 *
 * \sa DFMCrumbInterface, DFMCrumbManager, DFMCrumbItem
 */

DFMCrumbBar::DFMCrumbBar(QWidget *parent)
    : QFrame(parent)
    , d_ptr(new DFMCrumbBarPrivate(this))
{
    setFrameShape(QFrame::NoFrame);
}

DFMCrumbBar::~DFMCrumbBar()
{

}

/*!
 * \brief Toggle and show the address bar.
 *
 * \param text The text in the address bar.
 */
void DFMCrumbBar::showAddressBar(const QString &text)
{
    Q_D(DFMCrumbBar);

    d->leftArrow.hide();
    d->rightArrow.hide();
    d->crumbListScrollArea.hide();

    d->addressBar->show();
    d->addressBar->setText(text);
    //d->addressBar->setSelection(0, text.length());
    d->addressBar->setFocus();

    emit addressBarShown();

    return;
}

/*!
 * \brief Toggle and show the address bar.
 *
 * \param url The url in the address bar.
 */
void DFMCrumbBar::showAddressBar(const DUrl &url)
{
    Q_D(DFMCrumbBar);

    d->leftArrow.hide();
    d->rightArrow.hide();
    d->crumbListScrollArea.hide();

    d->addressBar->show();
    d->addressBar->setCurrentUrl(url);
    d->addressBar->setFocus();

    emit addressBarShown();

    return;
}

/*!
 * \brief Hide the address bar.
 */
void DFMCrumbBar::hideAddressBar()
{
    Q_D(DFMCrumbBar);

    d->addressBar->hide();

    d->crumbListScrollArea.show();
    d->checkArrowVisiable();

    emit addressBarHidden();

    return;
}

/*!
 * \brief Update crumbs in crumb bar by the given \a url
 *
 * \param url The newly switched url.
 *
 * DFMCrumbBar holds an instance of crumb controller (derived from DFMCrumbInterface), and when
 * calling updateCrumb, it will check the current used crumb controller is supporting the given
 * \a url. if isn't, we will create a new crumb controller form the registed controllers or the
 * plugin list. Then we will call DFMCrumbInterface::seprateUrl to seprate the url and call
 * DFMCrumbInterface::createCrumbItem to create the crumb item. Finally, we added the created
 * items to the crumb bar.
 */
void DFMCrumbBar::updateCrumbs(const DUrl &url)
{
    Q_D(DFMCrumbBar);

    d->clearCrumbs();

    if (!d->crumbController) {
        qWarning("No controller found when trying to call DFMCrumbBar::updateCrumbs() !!!");
        qDebug() << "updateCrumbs (no controller) : " << url;
        return;
    }

    QList<CrumbData> crumbDataList = d->crumbController->seprateUrl(url);
    for (const CrumbData& c : crumbDataList) {
        DFMCrumbItem* item = d->crumbController->createCrumbItem(c);
        item->setParent(this);
        d->buttonGroup.addButton(item);
        if (item->url() == url) {
            item->setCheckable(true);
            item->setChecked(true);
        }
        d->addCrumb(item);
    }

    d->crumbListHolder->adjustSize();
    d->checkArrowVisiable();
    d->crumbListScrollArea.horizontalScrollBar()->triggerAction(QScrollBar::SliderToMaximum);
}

void DFMCrumbBar::playAddressBarAnimation()
{
    Q_D(DFMCrumbBar);

    d->addressBar->playAnimation();
}

void DFMCrumbBar::stopAddressBarAnimation()
{
    Q_D(DFMCrumbBar);

    d->addressBar->stopAnimation();
}

/*!
 * \brief Call when toolbar url got changed.
 *
 * \param url Current url which we changed to.
 */
void DFMCrumbBar::updateCurrentUrl(const DUrl &url)
{
    Q_D(DFMCrumbBar);

    d->updateController(url);

    if (d->crumbController) {
        d->crumbController->crumbUrlChangedBehavior(url);
    }
}

void DFMCrumbBar::mousePressEvent(QMouseEvent *event)
{
    Q_D(DFMCrumbBar);
    d->clickedPos = event->globalPos();

    if (event->button() == Qt::RightButton && d->clickableAreaEnabled) {
        event->accept();
        return;
    }

    QFrame::mousePressEvent(event);
}

void DFMCrumbBar::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(DFMCrumbBar);

    if (!d->clickableAreaEnabled) {
        return QFrame::mouseReleaseEvent(event);;
    }

    // blumia: no need to check if it's clicked on other widgets
    //         since this will only happend when clicking empty.
    const QPoint pos_difference = d->clickedPos - event->globalPos();

    if (qAbs(pos_difference.x()) < 2 && qAbs(pos_difference.y()) < 2) {
        showAddressBar(qobject_cast<DFileManagerWindow*>(topLevelWidget())->currentUrl());
    }

    QFrame::mouseReleaseEvent(event);
}

void DFMCrumbBar::resizeEvent(QResizeEvent *event)
{
    Q_D(DFMCrumbBar);

    d->checkArrowVisiable();
    d->addressBar->resize(event->size());

    return QFrame::resizeEvent(event);
}

void DFMCrumbBar::showEvent(QShowEvent *event)
{
    Q_D(DFMCrumbBar);

    d->crumbListScrollArea.horizontalScrollBar()->setPageStep(d->crumbListHolder->width());
    d->crumbListScrollArea.horizontalScrollBar()->triggerAction(QScrollBar::SliderToMaximum);

    d->checkArrowVisiable();

    return QFrame::showEvent(event);
}

bool DFMCrumbBar::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(DFMCrumbBar);

    if (event->type() == QEvent::Wheel && d && watched == d->crumbListHolder) {
        class PublicQWheelEvent : public QWheelEvent
        {
        public:
            friend class dde_file_manager::DFMCrumbBar;
        };

        PublicQWheelEvent *e = static_cast<PublicQWheelEvent*>(event);

        e->modState = Qt::AltModifier;
        e->qt4O = Qt::Horizontal;
    }

    return QFrame::eventFilter(watched, event);
}

void DFMCrumbBar::paintEvent(QPaintEvent *event)
{
    Q_D(DFMCrumbBar);

    QPainter painter(this);
    QColor borderColor = ThemeConfig::instace()->color("CrumbBar.BorderLine", "border-color");
    QColor crumbBarBgColor = ThemeConfig::instace()->color("DFMCrumbBar", "background");
    QPainterPath path;

    QRectF crumbsRect(rect());

    if (!d->clickableAreaEnabled) {
        crumbsRect = d->crumbListHolder->rect();
    }

    painter.setRenderHint(QPainter::Antialiasing);
    path.addRoundedRect(QRectF(crumbsRect).adjusted(0.5, 0.5, -0.5, -0.5), 4, 4);
    QPen pen(borderColor, 1);
    painter.setPen(pen);

    if (d->addressBar->isHidden()) {
        QPainterPath path;

        path.addRoundedRect(QRectF(crumbsRect), 4, 4);
        painter.fillPath(path, crumbBarBgColor);
    }

    painter.drawPath(path);

    QFrame::paintEvent(event);
}

/*!
 * \fn DFMCrumbBar::addressBarShown()
 *
 * \brief Emit when the address bar got shown.
 *
 * \sa DFMAddressBar
 */

/*!
 * \fn DFMCrumbBar::addressBarHidden()
 *
 * \brief Emit when the address bar got hidden.
 *
 * \sa DFMAddressBar
 */

/*!
 * \fn DFMCrumbBar::crumbItemClicked(DFMCrumbItem *item);
 *
 * \brief User clicked the crumb \a item.
 */

DFM_END_NAMESPACE
