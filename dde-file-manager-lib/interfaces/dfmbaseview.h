/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#ifndef DFMBASEVIEW_H
#define DFMBASEVIEW_H

#include "dfmglobal.h"
#include "durl.h"

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

DFM_BEGIN_NAMESPACE

class DFMBaseView
{
public:
    DFMBaseView();
    virtual ~DFMBaseView();

    void deleteLater();

    virtual QWidget *widget() const = 0;
    virtual DUrl rootUrl() const = 0;
    virtual bool setRootUrl(const DUrl &url) = 0;
    virtual QList<QAction*> toolBarActionList() const;
    virtual void refresh();

protected:
    void notifyUrlChanged();
};

DFM_END_NAMESPACE

#endif // DFMBASEVIEW_H
