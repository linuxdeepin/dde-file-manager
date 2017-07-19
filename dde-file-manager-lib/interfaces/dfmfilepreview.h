/**
 * Copyright (C) 2017 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/
#ifndef DFMFILEPREVIEWDIALOG_H
#define DFMFILEPREVIEWDIALOG_H

#include <QObject>

#include "dfmglobal.h"

class DUrl;

DFM_BEGIN_NAMESPACE

class DFMFilePreview : public QObject
{
    Q_OBJECT

public:
    explicit DFMFilePreview(QObject *parent = 0);

    virtual void initialize(QWidget *window, QWidget *statusBar);
    virtual bool setFileUrl(const DUrl &url) = 0;

    virtual QWidget *contentWidget() const = 0;
    virtual QWidget *statusBarWidget() const;
    virtual Qt::Alignment statusBarWidgetAlignment() const;

    virtual QString title() const;
    virtual bool showStatusBarSeparator() const;

signals:
    void titleChanged();
};

DFM_END_NAMESPACE

#endif // DFMFILEPREVIEWDIALOG_H
