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

#include "registerexpandprocess.h"
#include "dfm-base/utils/finallyutil.h"

#include <QDebug>

DSB_FM_USE_NAMESPACE
DTSP_USE_NAMESPACE
RegisterExpandProcess::RegisterExpandProcess()
{
}

RegisterExpandProcess *RegisterExpandProcess::instance()
{
    static RegisterExpandProcess process;
    return &process;
}

bool RegisterExpandProcess::registerFunction(RegisterExpandProcess::createControlViewFunc view, int index, QString *errorString)
{
    QString error;
    DFMBASE_NAMESPACE::FinallyUtil finally([&]() { if (errorString) *errorString = error; });

    if (constructList.keys().contains(index)) {
        error = QObject::tr("The current index has registered "
                            "the associated construction class");
        qInfo() << error;
        return false;
    }

    constructList.insert(index, view);
    finally.dismiss();
    return true;
}

QMap<int, QWidget *> RegisterExpandProcess::createControlView(const QUrl &url)
{
    QMap<int, QWidget *> temp {};
    for (createControlViewFunc &func : constructList.values()) {
        int index = constructList.key(func);
        QWidget *g = func(url);
        if (g != nullptr)
            temp.insert(index, g);
    }
    return temp;
}
