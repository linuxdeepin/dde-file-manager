/*
 * Copyright (C) 2021 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *             max-lv<lvwujun@uniontech.com>
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

#ifndef DFMAPPENTRYFILEINFO_P_H
#define DFMAPPENTRYFILEINFO_P_H

#include "dfmrootfileinfo_p.h"
#include "dfmappentryfileinfo.h"

class DFMAppEntryFileInfoPrivate: public DFMRootFileInfoPrivate {
public:
    DFMAppEntryFileInfoPrivate(DFMAppEntryFileInfo *qq)
        : q_ptr(qq){}

private:
    DFMAppEntryFileInfo *q_ptr;
    Q_DECLARE_PUBLIC(DFMRootFileInfo)
};


#endif // DFMAPPENTRYFILEINFO_P_H
