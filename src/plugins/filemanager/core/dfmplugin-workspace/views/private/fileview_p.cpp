/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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

#include "fileview_p.h"
#include "views/headerview.h"
#include "views/fileviewstatusbar.h"
#include "models/fileviewmodel.h"
#include "models/filesortfilterproxymodel.h"
#include "events/workspaceeventcaller.h"
#include "utils/workspacehelper.h"
#include "utils/dragdrophelper.h"
#include "utils/viewdrawhelper.h"
#include "utils/selecthelper.h"
#include "utils/shortcuthelper.h"
#include "utils/fileoperatorhelper.h"
#include "utils/fileviewmenuhelper.h"

#include "dfm-base/base/application/application.h"
#include "dfm-base/base/application/settings.h"
#include "dfm-base/base/schemefactory.h"

#include <QScrollBar>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_workspace;

FileViewPrivate::FileViewPrivate(FileView *qq)
    : q(qq)
{
    dragDropHelper = new DragDropHelper(qq);
    viewDrawHelper = new ViewDrawHelper(qq);
    selectHelper = new SelectHelper(qq);
    shortcutHelper = new ShortcutHelper(qq);
    viewMenuHelper = new FileViewMenuHelper(qq);

    enabledSelectionModes << FileView::NoSelection << FileView::SingleSelection
                          << FileView::MultiSelection << FileView::ExtendedSelection
                          << FileView::ContiguousSelection;

    allowedAdjustColumnSize = Application::instance()->appAttribute(Application::kViewSizeAdjustable).toBool();
}

int FileViewPrivate::iconModeColumnCount(int itemWidth) const
{
    int horizontalMargin = 0;

    int contentWidth = q->maximumViewportSize().width();

    if (itemWidth <= 0)
        itemWidth = q->itemSizeHint().width() + q->spacing() * 2;

    return qMax((contentWidth - horizontalMargin - 1) / itemWidth, 1);
}

QUrl FileViewPrivate::modelIndexUrl(const QModelIndex &index) const
{
    return index.data().toUrl();
}

void FileViewPrivate::initIconModeView()
{
    if (headerView) {
        headerView->disconnect();
        q->removeHeaderWidget(0);
        delete headerView;
        headerView = nullptr;
    }

    if (statusBar) {
        statusBar->setScalingVisible(true);
        q->setIconSize(QSize(iconSizeList()[currentIconSizeLevel],
                             iconSizeList()[currentIconSizeLevel]));
        statusBar->scalingSlider()->setValue(currentIconSizeLevel);
    }
}

void FileViewPrivate::initListModeView()
{
    if (!headerView) {
        headerView = new HeaderView(Qt::Orientation::Horizontal, q);

        headerView->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        headerView->setFixedHeight(kListViewIconSize);
        headerView->setMinimumSectionSize(GlobalPrivate::kListViewMinimumWidth);

        headerView->setModel(q->model());
        if (q->selectionModel()) {
            headerView->setSelectionModel(q->selectionModel());
        }
    }

    q->addHeaderWidget(headerView);

    QObject::connect(headerView, &HeaderView::mouseReleased, q, &FileView::onHeaderViewMouseReleased);
    QObject::connect(headerView, &HeaderView::sectionResized, q, &FileView::onHeaderSectionResized);
    QObject::connect(headerView, &HeaderView::sortIndicatorChanged, q, &FileView::onSortIndicatorChanged);
    QObject::connect(headerView, &HeaderView::sectionMoved, q, &FileView::onHeaderSectionMoved);
    QObject::connect(headerView, &HeaderView::sectionHandleDoubleClicked, q, &FileView::onSectionHandleDoubleClicked);
    QObject::connect(headerView, &HeaderView::hiddenSectionChanged, q, &FileView::onHeaderHiddenChanged);
    QObject::connect(q->horizontalScrollBar(), &QScrollBar::valueChanged, headerView, [=](int value) {
        headerView->move(-value, headerView->y());
    });

    q->setIconSize(QSize(kListViewIconSize, kListViewIconSize));

    if (statusBar)
        statusBar->setScalingVisible(false);
}

QModelIndexList FileViewPrivate::selectedDraggableIndexes()
{
    QModelIndexList indexes = q->selectedIndexes();

    auto isNotDragEnabled = [=](const QModelIndex &index) {
        return !(q->model()->flags(index) & Qt::ItemIsDragEnabled);
    };

    indexes.erase(std::remove_if(indexes.begin(), indexes.end(), isNotDragEnabled),
                  indexes.end());

    return indexes;
}

void FileViewPrivate::initContentLabel()
{
    if (!contentLabel) {
        contentLabel = new QLabel(q);

        QPalette palette = contentLabel->palette();
        QStyleOption opt;
        opt.initFrom(contentLabel);
        QColor color = opt.palette.color(QPalette::Inactive, QPalette::Text);
        palette.setColor(QPalette::Text, color);
        contentLabel->setPalette(palette);

        auto font = contentLabel->font();
        font.setFamily("SourceHanSansSC-Light");
        font.setPixelSize(20);
        contentLabel->setFont(font);

        contentLabel.setCenterIn(q);
        contentLabel->setStyleSheet(q->styleSheet());
        contentLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        contentLabel->show();
    }
}

void FileViewPrivate::updateHorizontalScrollBarPosition()
{
    QWidget *widget = qobject_cast<QWidget *>(q->horizontalScrollBar()->parentWidget());

    widget->move(widget->x(), q->height() - statusBar->height() - widget->height());
}

void FileViewPrivate::pureResizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)

    if (currentViewMode == Global::ViewMode::kListMode) {
        if (q->width() >= headerView->width())
            adjustFileNameColumn = true;

        if (adjustFileNameColumn)
            headerView->doFileNameColumnResize(q->width());
    }
}

void FileViewPrivate::loadViewMode(const QUrl &url)
{
    int defaultViewMode = static_cast<int>(WorkspaceHelper::instance()->findViewMode(url.scheme()));
    int savedViewMode = fileViewStateValue(url, "viewMode", -1).toInt();
    int parentViewMode = -1;

    if (savedViewMode == -1
        && Application::appObtuselySetting()->value("ApplicationAttribute", "UseParentViewMode", false).toBool()) {
        auto info = InfoFactory::create<AbstractFileInfo>(url);
        QList<QUrl> parentUrlList {};
        info->isAncestorsUrl(QUrl(), &parentUrlList);

        for (const QUrl &parentUrl : parentUrlList) {
            parentViewMode = fileViewStateValue(parentUrl, "viewMode", -1).toInt();
            break;
        }
    }

    if (parentViewMode != -1) {
        currentViewMode = static_cast<Global::ViewMode>(parentViewMode);
    } else if (savedViewMode != -1) {
        currentViewMode = static_cast<Global::ViewMode>(savedViewMode);
    } else {
        currentViewMode = static_cast<Global::ViewMode>(defaultViewMode);
    }
}

QVariant FileViewPrivate::fileViewStateValue(const QUrl &url, const QString &key, const QVariant &defalutValue)
{
    QUrl realUrl = url;
    // reset root url to make them be same.
    if (UrlRoute::isRootUrl(url))
        realUrl = UrlRoute::rootUrl(url.scheme());

    QMap<QString, QVariant> valueMap = Application::appObtuselySetting()->value("FileViewState", realUrl).toMap();
    return valueMap.value(key, defalutValue);
}
