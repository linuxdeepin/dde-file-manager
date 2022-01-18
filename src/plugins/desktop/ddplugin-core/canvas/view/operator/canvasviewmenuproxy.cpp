/*
 * Copyright (C) 2022 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#include "canvasviewmenuproxy.h"
#include "displayconfig.h"
#include "canvas/grid/canvasgrid.h"
#include "canvas/view/canvasview.h"
#include "canvas/view/canvasmodel.h"
#include "canvas/view/operator/fileoperaterproxy.h"
#include "dfm-base/utils/clipboard.h"
#include "dfm-base/interfaces/abstractfileinfo.h"
#include "dfm-base/dfm_global_defines.h"

#include <QMenu>
#include <QtDebug>

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSB_D_USE_NAMESPACE

CanvasViewMenuProxy::CanvasViewMenuProxy(CanvasView *parent)
    : QObject(parent)
    , view(parent)
{

}

CanvasViewMenuProxy::~CanvasViewMenuProxy()
{

}

void CanvasViewMenuProxy::showEmptyAreaMenu(const Qt::ItemFlags &indexFlags)
{
    Q_UNUSED(indexFlags)
    // todo menu

    // test code
    QMenu *tstMenu = new QMenu;
    QAction *tstAction = nullptr;

    tstAction = tstMenu->addAction(tr("Create file"));
    connect(tstAction, &QAction::triggered, this, [=](){
        FileOperaterProxyIns->touchFile(view, kCreateFileTypeText);
    });

    tstAction = tstMenu->addAction(tr("Create folder"));
    connect(tstAction, &QAction::triggered, this, [=](){
        FileOperaterProxyIns->touchFolder(view);
    });

    tstAction = tstMenu->addAction(tr("Paste"));
    connect(tstAction, &QAction::triggered, this, [=](){
        FileOperaterProxyIns->pasteFiles(view);
    });

    auto sortByRole = [=](const dfmbase::AbstractFileInfo::SortKey role)->bool{
        Qt::SortOrder order = view->model()->sortOrder();
        if (role == view->model()->sortRole()) {
            order = order == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
        }
        view->model()->setSortRole(role, order);
        view->model()->sort();
    };

    QMenu *subMenu = tstMenu->addMenu(tr("Sort by"));
    tstAction = subMenu->addAction(tr("Name"));
    connect(tstAction, &QAction::triggered, this, [=](){
        sortByRole(AbstractFileInfo::kSortByFileName);
    });

    tstAction = subMenu->addAction(tr("Time modified"));
    connect(tstAction, &QAction::triggered, this, [=](){
        sortByRole(AbstractFileInfo::kSortByModified);
    });

    tstAction = subMenu->addAction(tr("Size"));
    connect(tstAction, &QAction::triggered, this, [=](){
        sortByRole(AbstractFileInfo::kSortByFileSize);
    });

    tstAction = subMenu->addAction(tr("Type"));
    connect(tstAction, &QAction::triggered, this, [=](){
        // todo(wangcl):now, not type func
    });

    tstAction = tstMenu->addAction(tr("Auto arrange"));
    connect(tstAction, &QAction::triggered, this, [=](){
        auto align = DispalyIns->autoAlign();
        align = !align;
        DispalyIns->setAutoAlign(align);

        if (align) {
            GridIns->setMode(CanvasGrid::Mode::Align);
            GridIns->setItems(GridIns->items());
        } else {
            GridIns->setMode(CanvasGrid::Mode::Custom);
        }
    });

    tstMenu->exec(QCursor::pos());
    delete tstMenu;
}

void CanvasViewMenuProxy::showNormalMenu(const QModelIndex &index, const Qt::ItemFlags &indexFlags)
{
    Q_UNUSED(index)
    Q_UNUSED(indexFlags)
    // todo menu

    // test code
    QMenu *tstMenu = new QMenu;
    QAction *tstAction = nullptr;

    tstAction = tstMenu->addAction(tr("Open"));
    connect(tstAction, &QAction::triggered, this, [=](){
        FileOperaterProxyIns->openFiles(view);
    });

    tstAction = tstMenu->addAction(tr("Delete"));
    connect(tstAction, &QAction::triggered, this, [=](){
        FileOperaterProxyIns->moveToTrash(view);
    });

    tstAction = tstMenu->addAction(tr("Delete forever"));
    connect(tstAction, &QAction::triggered, this, [=](){
        FileOperaterProxyIns->deleteFiles(view);
    });

    tstAction = tstMenu->addAction(tr("Copy"));
    connect(tstAction, &QAction::triggered, this, [=](){
        FileOperaterProxyIns->copyFiles(view);
    });

    tstAction = tstMenu->addAction(tr("Cut"));
    connect(tstAction, &QAction::triggered, this, [=](){
        FileOperaterProxyIns->cutFiles(view);
    });

    tstMenu->exec(QCursor::pos());
    delete tstMenu;
}

