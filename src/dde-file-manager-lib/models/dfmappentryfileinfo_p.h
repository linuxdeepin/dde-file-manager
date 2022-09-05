// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
