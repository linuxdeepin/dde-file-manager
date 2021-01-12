/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "policykithelper.h"

bool PolicyKitHelper::checkAuthorization(const QString& actionId, qint64 applicationPid)
{
    Authority::Result result;
    result = Authority::instance()->checkAuthorizationSync(actionId, UnixProcessSubject(applicationPid), /// 第一个参数是需要验证的action，和规则文件写的保持一致
             Authority::AllowUserInteraction);
    if (result == Authority::Yes) {
        return true;
    }else {
        return false;
    }
}

PolicyKitHelper::PolicyKitHelper()
{

}

PolicyKitHelper::~PolicyKitHelper()
{

}
