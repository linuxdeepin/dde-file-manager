// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FILEEVENTPROCESSOR_H
#define FILEEVENTPROCESSOR_H

#include "dfmglobal.h"
#include "dfmabstracteventhandler.h"

DFM_BEGIN_NAMESPACE

class FileEventProcessor : public DFMAbstractEventHandler
{
public:
    FileEventProcessor();

private:
    virtual bool fmEvent(const QSharedPointer<DFMEvent> &event, QVariant *resultData) override;
};

DFM_END_NAMESPACE

#endif // FILEEVENTPROCESSOR_H
