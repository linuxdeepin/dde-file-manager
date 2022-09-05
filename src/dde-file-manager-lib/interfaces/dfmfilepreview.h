// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    virtual DUrl fileUrl() const = 0;

    virtual QWidget *contentWidget() const = 0;
    virtual QWidget *statusBarWidget() const;
    virtual Qt::Alignment statusBarWidgetAlignment() const;

    virtual QString title() const;
    virtual bool showStatusBarSeparator() const;

    /**
     * @brief DoneCurrent 释放surface
     */
    virtual void DoneCurrent();

    virtual void play(); /*play media if file is music or video*/
    virtual void pause(); /*pause playing if file is music or video*/
    virtual void stop(); /*stop playing if file is music or video*/

    virtual void copyFile() const;

signals:
    void titleChanged();
};

DFM_END_NAMESPACE

#endif // DFMFILEPREVIEWDIALOG_H
