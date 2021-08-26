/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
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
#include "dfmglobaleventdispatcher.h"
#include <QFuture>
#include <QtConcurrent>
#include <QWidget>

Q_GLOBAL_STATIC(QList<DFMAbstractEventHandler*>,_handers);

int DFMGlobalEventDispatcher::installHandler(DFMAbstractEventHandler * const handler) {
    _handers->insert(0,handler);
    return _handers->indexOf(handler);
}

void DFMGlobalEventDispatcher::setMaxThreadCount(int count)
{
    const int threadCount = QThreadPool::globalInstance()->maxThreadCount();
    QThreadPool::globalInstance()->setMaxThreadCount(qMax(count, 2 * threadCount));
}

int DFMGlobalEventDispatcher::maxThreadCount(){
    return QThreadPool::globalInstance()->maxThreadCount();
}

void DFMGlobalEventDispatcher::sendEvent(const DFMEventPointer &event)
{
    for (auto handler: *_handers) {
        //如果当前可以异步执行且handler与界面类无关，则加入线程运行
        //此处!dynamic_cast<QWidget*>(handler)为优化代码，
        //但是这只能保证当前类不是界面类,而如果其使用widget成员变量会导致界面假死
        if (handler->canAsynProcess() ||
                !dynamic_cast<QWidget*>(handler)) {
            QtConcurrent::run(handler, &DFMAbstractEventHandler::event, event);
        } else { //直接调用
            handler->event(event);
            //事件被接收则直接退出循环
            if (event->isAccepted()) {
                return;
            }
        }
    }
}
