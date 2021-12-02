/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "docutfilesworker.h"

#include <QUrl>
#include <QDebug>

DSC_USE_NAMESPACE
DoCutFilesWorker::DoCutFilesWorker(QObject *parent)
    : AbstractWorker(parent)
{
}

DoCutFilesWorker::~DoCutFilesWorker()
{
}

void DoCutFilesWorker::doWork()
{
    // ToDo::执行剪切的业务逻辑
}

void DoCutFilesWorker::stop()
{
    // ToDo::停止剪切的业务逻辑
}

void DoCutFilesWorker::pause()
{
    // ToDo::暂停剪切的业务逻辑
}
