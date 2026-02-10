// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ABSTRACTFRAME_H
#define ABSTRACTFRAME_H

#include <dfm-base/dfm_base_global.h>

#include <QFrame>
#include <QUrl>

namespace dfmbase {

class AbstractFrame : public QFrame
{
    Q_OBJECT
public:
    explicit AbstractFrame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~AbstractFrame();

    virtual void setCurrentUrl(const QUrl &url) = 0;
    virtual QUrl currentUrl() const = 0;
};

}

#endif   // ABSTRACTFRAME_H
