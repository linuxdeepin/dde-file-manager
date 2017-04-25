/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
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
    virtual bool fmEvent(const QSharedPointer<DFMEvent> &event, QVariant *resultData) Q_DECL_OVERRIDE;
};

DFM_END_NAMESPACE

#endif // FILEEVENTPROCESSOR_H
