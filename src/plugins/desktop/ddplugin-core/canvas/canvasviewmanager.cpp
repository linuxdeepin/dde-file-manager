/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "canvasviewmanager.h"
#include "canvasmodel.h"
#include "canvasgridmanager.h"
#include "filetreater.h"
#include "private/canvasviewmanager_p.h"

#include "dfm-framework/framework.h"

DSB_D_BEGIN_NAMESPACE

CanvasViewManager::CanvasViewManager(QObject *parent)
    : QObject(parent)
    , d(new CanvasViewManagerPrivate(this))
{
    auto &ctx = dpfInstance.serviceContext();
    // 获取屏幕服务
    d->screenScevice = ctx.service<ScreenService>(ScreenService::name());

    // 获取背景服务
    d->backgroundService = ctx.service<BackgroundService>(BackgroundService::name());

    // 绑定背景信号
    // todo
    initConnect();

    d->canvasModel = new CanvasModel(this);
    CanvasGridManager::instance()->initCoord(d->backgroundService->allBackground().size());
    // 根据屏幕build
    onCanvasViewBuild();
}

void CanvasViewManager::onFileLoadFinish()
{
    if (!d->isDone) {
        qDebug() << "data load done, need to show when view builed done ";
        // 说明视图窗口还未初始化完全,初始化完成后直接去获取数据初始化
        return;
    }
    loadDataAndShow();
}

void CanvasViewManager::initConnect()
{
    // 遍历数据完成后通知更新栅格位置信息
    connect(FileTreaterCt, &FileTreater::fileFinished, this, &CanvasViewManager::onFileLoadFinish, Qt::DirectConnection);

    // 屏幕增删，模式改变
    connect(d->backgroundService, &BackgroundService::sigBackgroundBuilded, this, &CanvasViewManager::onCanvasViewBuild);
}

void CanvasViewManager::loadDataAndShow()
{
    // 初始化栅布局信息
    CanvasGridManager::instance()->initGridItemsInfo();

    // show canvas
    for (const CanvasViewPointer &cv : d->canvasViewMap.values()) {
        cv->show();
    }
}

void CanvasViewManager::onCanvasViewBuild()
{
    d->isDone = false;
    dfmbase::DisplayMode displayMode = d->screenScevice->displayMode();
    auto screenSize = d->screenScevice->screens().size();

    if ((displayMode == dfmbase::DisplayMode::Showonly)
        || (displayMode == dfmbase::DisplayMode::Duplicate)
        || (1 == screenSize)) {
        // 仅显示模式(Showonly)、复制模式(Duplicate)、单屏
        auto primary = d->screenScevice->primaryScreen();
        if (primary == nullptr) {
            //屏幕信息获取失败，清空对应关系
            d->canvasViewMap.clear();
            qCritical() << "get primary screen failed return";
            return;
        }

        CanvasViewPointer mView = d->canvasViewMap.value(primary->name());
        // 清空前次画布信息
        d->canvasViewMap.clear();

        // todo 待调整这部分
        QRect avRect;
        QRect screenAvaRect = primary->availableGeometry();
#ifndef UNUSED_SMARTDOCK
        avRect = d->relativeRect(screenAvaRect, primary->geometry());
#else
        if (primary == ScreenMrg->primaryScreen()) {
            avRect = relativeRect(primary->availableGeometry(), primary->geometry());
        } else {
            avRect = relativeRect(primary->geometry(), primary->geometry());
        }
#endif

        if (mView.get() == nullptr) {
            mView.reset(new CanvasView());
            mView->setScreenName(primary->name());
            mView->setScreenNum(1);
            mView->setModel(d->canvasModel);
            auto background = d->backgroundService->background(primary->name());
            mView->setParent(background.data());
            mView->setGeometry(avRect);
        } else {
            mView->setScreenNum(1);
            mView->setScreenName(primary->name());
            mView->clearSelection();
            mView->setGeometry(avRect);
        }

        d->canvasViewMap.insert(primary->name(), mView);
        d->screenMap.insert(primary->name(), primary);
    } else {
        //扩展模式、自定义模式
        auto currentScreens = d->screenScevice->logicScreens();
        int screenNum = 0;

        //检查新增的屏幕
        for (const dfmbase::ScreenPointer &sp : currentScreens) {
            ++screenNum;

            // todo:似乎可以提成一个函数
            QRect avRect;
            QRect screenAvaRect = sp->availableGeometry();
            avRect = d->relativeRect(screenAvaRect, sp->geometry());
            CanvasViewPointer mView = d->canvasViewMap.value(sp->name());

            //新增
            if (mView.get() == nullptr) {
                mView.reset(new CanvasView());
                mView->setScreenName(sp->name());
                mView->setScreenNum(screenNum);
                mView->setModel(d->canvasModel);

                auto background = d->backgroundService->background(sp->name());
                mView->setParent(background.data());
                mView->setGeometry(avRect);

                d->canvasViewMap.insert(sp->name(), mView);
                d->screenMap.insert(sp->name(), sp);
            } else {
                mView->setScreenNum(screenNum);
                mView->setScreenName(sp->name());
                mView->setGeometry(avRect);
                mView->clearSelection();
            }
        }

        //检查移除的屏幕
        for (const QString &sp : d->canvasViewMap.keys()) {
            if (!d->screenScevice->screen(sp)) {
                d->canvasViewMap.remove(sp);
                auto rmSp = d->screenMap.take(sp);
                qDebug() << "mode " << displayMode << "remove " << sp << "screen geometry " << rmSp->geometry();
            }
        }
    }
    d->isDone = true;
    //检查是否展示的桌面数据已到位
    if (FileTreaterCt->isDone()) {
        loadDataAndShow();
    } else {
        qDebug() << "view builed but no data, need to show when FileTreater done ";
    }
}

DSB_D_END_NAMESPACE
