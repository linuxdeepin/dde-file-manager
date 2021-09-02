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

#include "private/crumbbar_p.h"
//#include "dfmcrumbinterface.h"
#include "crumbbar.h"
#include "crumbmodel.h"

#include "dfm-base/base/singleton.hpp"
#include "dfm-base/base/standardpaths.h"
#include "dfm-base/base/application.h"
#include "dfm-base/base/dfmsettings.h"

#include <DListView>

#include <QHBoxLayout>
#include <QPainter>
#include <QScrollBar>
#include <QApplication>
#include <QMenu>
#include <QDebug>
#include <QGuiApplication>
#include <QClipboard>
#include <QPushButton>
#include <QMouseEvent>

DWIDGET_USE_NAMESPACE

DSB_FM_BEGIN_NAMESPACE

DFMCrumbBarPrivate::DFMCrumbBarPrivate(DFMCrumbBar *qq)
    : q_ptr(qq)
{
    initData();
    initUI();
    initConnections();
}

DFMCrumbBarPrivate::~DFMCrumbBarPrivate()
{

}

/*!
 * \brief Remove all crumbs List items inside crumb listView.
 */
void DFMCrumbBarPrivate::clearCrumbs()
{
    m_leftArrow.hide();
    m_rightArrow.hide();

    if (m_crumbModel)
        m_crumbModel->removeAll();
}

void DFMCrumbBarPrivate::checkArrowVisiable()
{
    QScrollBar *scrollBar = m_crumbView.horizontalScrollBar();
    if (!scrollBar)
        return;

    m_leftArrow.setVisible(scrollBar->maximum() > 0);
    m_rightArrow.setVisible(scrollBar->maximum() > 0);

    m_leftArrow.setEnabled(scrollBar->value() != scrollBar->minimum());
    m_rightArrow.setEnabled(scrollBar->value() != scrollBar->maximum());
}

/*!
 * \brief Update the crumb controller hold by the crumb bar.
 *
 * \param url The url which controller should supported.
 */

void DFMCrumbBarPrivate::setClickableAreaEnabled(bool enabled)
{
    Q_Q(DFMCrumbBar);

    if (m_clickableAreaEnabled == enabled) return;

    m_clickableAreaEnabled = enabled;

    q->update();
}

void DFMCrumbBarPrivate::initUI()
{
    Q_Q(DFMCrumbBar);

    // Crumbbar Widget
    //q->setFixedHeight(24);

    // Arrows
    QSize size(24, 24), iconSize(16, 16);

    m_leftArrow.setFocusPolicy(Qt::NoFocus);
    m_leftArrow.setIcon(QIcon::fromTheme("go-previous"));
    m_rightArrow.setIcon(QIcon::fromTheme("go-next"));
    m_rightArrow.setFocusPolicy(Qt::NoFocus);

    m_leftArrow.setFixedSize(size);
    m_leftArrow.setIconSize(iconSize);
    m_rightArrow.setFixedSize(size);
    m_rightArrow.setIconSize(iconSize);
    m_leftArrow.setFlat(true);
    m_rightArrow.setFlat(true);
    m_leftArrow.hide();
    m_rightArrow.hide();

    // Crumb List Layout
    m_crumbView.setObjectName("DCrumbListScrollArea");

    m_crumbView.setItemSpacing(10);
    m_crumbView.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_crumbView.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_crumbView.setFocusPolicy(Qt::NoFocus);
    m_crumbView.setContentsMargins(0, 0, 0, 0);
    m_crumbView.setSizeAdjustPolicy(QAbstractScrollArea::AdjustIgnored);
    m_crumbView.setIconSize({16, 16});
    m_crumbView.setHorizontalScrollMode(QAbstractItemView::ScrollPerItem);
    m_crumbView.setOrientation(QListView::LeftToRight, false);
    m_crumbView.setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_crumbView.setDragDropMode(QAbstractItemView::DragDropMode::NoDragDrop);

    m_crumbModel = new DFMCrumbModel(q);
    m_crumbView.setModel(m_crumbModel);
    m_crumbView.setContextMenuPolicy(Qt::CustomContextMenu);

    // 点击listview空白可拖动窗口
    m_crumbView.viewport()->installEventFilter(q);

    // for first icon item icon AlignCenter...
    class IconItemDelegate : public DStyledItemDelegate
    {
    public:
        explicit IconItemDelegate(QAbstractItemView *parent = nullptr): DStyledItemDelegate(parent)
        {
            setItemSpacing(10);
        }

        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
        {
            QStyleOptionViewItem opt = option;
            opt.decorationAlignment = Qt::AlignCenter;
            DStyledItemDelegate::paint(painter, opt, index);
        }
    };
    m_crumbView.setItemDelegateForRow(0, new IconItemDelegate(&m_crumbView));

    // Crumb Bar Layout
    m_crumbBarLayout = new QHBoxLayout(q);
    m_crumbBarLayout->addWidget(&m_leftArrow);
    m_crumbBarLayout->addWidget(&m_crumbView);
    m_crumbBarLayout->addWidget(&m_rightArrow);
    m_crumbBarLayout->setContentsMargins(0, 0, 0, 0);
    m_crumbBarLayout->setSpacing(0);
    q->setLayout(m_crumbBarLayout);

    return;
}

void DFMCrumbBarPrivate::initData()
{
    m_clickableAreaEnabled = Application::instance()->
            genericAttribute(Application::GA_ShowCsdCrumbBarClickableArea)
            .toBool();
}

void DFMCrumbBarPrivate::initConnections()
{
    Q_Q(DFMCrumbBar);

    QObject::connect(&m_crumbView, &QListView::customContextMenuRequested,
                     q, &DFMCrumbBar::onCustomContextMenu);

    QObject::connect(&m_crumbView, &QListView::clicked,
                     q, [=](const QModelIndex & index)
    {
        if (index.isValid()) {
            qInfo() << "sig send selectedUrl: " << index.data(DFMCrumbModel::FileUrlRole).toUrl();
            emit q->selectedUrl(index.data(DFMCrumbModel::FileUrlRole).toUrl());
        }
    });

    q->connect(&m_leftArrow, &QPushButton::clicked,
               q, [=]()
    {
        m_crumbView.horizontalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
    });

    q->connect(&m_rightArrow, &QPushButton::clicked,
               q, [=]()
    {
        m_crumbView.horizontalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
    });

    q->connect(m_crumbView.horizontalScrollBar(), &QScrollBar::valueChanged,
               q, [=]()
    {
        checkArrowVisiable();
    });

    if (Application::instance()) {
        q->connect(Application::instance(),
                   &Application::csdClickableAreaAttributeChanged,
                   q, [=](bool enabled)
        {
            setClickableAreaEnabled(enabled);
        });
    }
}

/*!
 * \class DFMCrumbBar
 * \inmodule dde-file-manager-lib
 *
 * \brief DFMCrumbBar is the crumb bar widget of Deepin File Manager
 *
 * DFMCrumbBar is the crumb bar widget of Deepin File Manager, provide the interface to manage
 * crumb bar state.
 *
 * \sa DFMCrumbInterface, DFMCrumbManager
 */

DFMCrumbBar::DFMCrumbBar(QWidget *parent)
    : QFrame(parent)
    , d_ptr(new DFMCrumbBarPrivate(this))
{
    setFrameShape(QFrame::NoFrame);
    setFixedHeight(36);
}

DFMCrumbBar::~DFMCrumbBar()
{

}

void DFMCrumbBar::setRootUrl(const QUrl &url)
{
    Q_D(DFMCrumbBar);

    d->clearCrumbs();

    const QIcon firstIcon = UrlRoute::schemeIcon(url.scheme());

    QString scheme = url.scheme();
    QString path = url.path();
    QStringList pathList = path.split("/");

    QStandardItem* firstItem = new QStandardItem(firstIcon, QString(""));
    //    firstItem->setData(QSize({36,36}),Qt::SizeHintRole);
    for (int nodeCount = pathList.size(); nodeCount -- ; nodeCount > 0)
    {
        if (pathList.at(nodeCount).isEmpty())
            continue;

        auto currNodeItem = new QStandardItem(pathList.at(nodeCount));
        QStringList currNodeList;
        for (int index = 0 ; index <= nodeCount ; index ++)
        {
            currNodeList.append(pathList.at(index));
        }
        auto currNodeUrl = UrlRoute::pathToUrl(currNodeList.join("/"));
        currNodeItem->setData(currNodeUrl,DFMCrumbModel::Roles::FileUrlRole);

        d->m_crumbModel->insertRow(0, currNodeItem);
    }
    d->m_crumbModel->insertRow(0,firstItem);

    if (d->m_crumbView.selectionModel() && d->m_crumbModel)
        d->m_crumbView.selectionModel()->select(d->m_crumbModel->lastIndex(), QItemSelectionModel::Select);

    d->checkArrowVisiable();
    d->m_crumbView.horizontalScrollBar()->triggerAction(QScrollBar::SliderToMaximum);
}

void DFMCrumbBar::mousePressEvent(QMouseEvent *event)
{
    Q_D(DFMCrumbBar);
    d->m_clickedPos = event->globalPos();

    if (event->button() == Qt::RightButton && d->m_clickableAreaEnabled) {
        event->accept();
        return;
    }

    QModelIndex index = d->m_crumbView.indexAt(event->pos());
    if (event->button() != Qt::RightButton || !index.isValid()) {
        QFrame::mousePressEvent(event);
    }
}

void DFMCrumbBar::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(DFMCrumbBar);

    if (!d->m_clickableAreaEnabled) {
        return QFrame::mouseReleaseEvent(event);;
    }

    QFrame::mouseReleaseEvent(event);
}

void DFMCrumbBar::resizeEvent(QResizeEvent *event)
{
    Q_D(DFMCrumbBar);

    d->checkArrowVisiable();

    return QFrame::resizeEvent(event);
}

void DFMCrumbBar::showEvent(QShowEvent *event)
{
    Q_D(DFMCrumbBar);

    //d->crumbListScrollArea.horizontalScrollBar()->setPageStep(d->crumbListHolder->width());
    d->m_crumbView.horizontalScrollBar()->triggerAction(QScrollBar::SliderToMaximum);

    d->checkArrowVisiable();

    return QFrame::showEvent(event);
}

bool DFMCrumbBar::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(DFMCrumbBar);
    QMouseEvent *me = nullptr;
    if (watched && watched->parent() == &d->m_crumbView && (me = dynamic_cast<QMouseEvent *>(event))) {
        QEvent::Type type = event->type();
        bool isMousePressed = type == QEvent::MouseButtonPress || type == QEvent::MouseButtonDblClick;
        static QPoint pos;
        if (isMousePressed) {
            pos = QCursor::pos();
        }

        bool isIgnore = isMousePressed ||  type == QEvent::MouseMove;
        if (isIgnore) {
            event->ignore();
            return true;
        }

        bool isDragging = (pos - QCursor::pos()).manhattanLength() > QApplication::startDragDistance();
        if (type == QEvent::MouseButtonRelease && me->button() == Qt::LeftButton && !isDragging) {
            QModelIndex index = d->m_crumbView.indexAt(me->pos());
            if (index.isValid() && index != d->m_crumbView.currentIndex()) {
                d->m_crumbView.clicked(index);
                return true;
            }
        }
    }

    return QFrame::eventFilter(watched, event);
}

void DFMCrumbBar::onCustomContextMenu(const QPoint &point)
{
    Q_D(DFMCrumbBar);
    QModelIndex index = d->m_crumbView.indexAt(point);
    if (!index.isValid())
        return;
    qInfo() << "need add mouse right clicked menu show logic";
}

DSB_FM_END_NAMESPACE
