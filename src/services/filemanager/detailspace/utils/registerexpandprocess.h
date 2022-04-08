/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

#ifndef REGISTEREXPANDPROCESS_H
#define REGISTEREXPANDPROCESS_H

#include "dfm_filemanager_service_global.h"
#include "detailspace/detailspace_defines.h"

#include <QWidget>

DSB_FM_BEGIN_NAMESPACE
DTSP_BEGIN_NAMESPACE
class RegisterExpandProcess
{
private:
    explicit RegisterExpandProcess();

public:
    static RegisterExpandProcess *instance();

public:
    //! 定义创建控件函数类型
    typedef QWidget *(*createControlViewFunc)(const QUrl &url);

protected:
    //! 创建函数列表
    QHash<int, createControlViewFunc> constructList {};

public:
    bool registerFunction(createControlViewFunc view, int index = -1, QString *errorString = nullptr);

    QMap<int, QWidget *> createControlView(const QUrl &url);
};
DTSP_END_NAMESPACE
DSB_FM_END_NAMESPACE
#endif   //! REGISTEREXPANDPROCESS_H
