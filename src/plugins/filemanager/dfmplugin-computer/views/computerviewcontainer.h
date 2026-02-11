// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPUTERVIEWCONTAINER_H
#define COMPUTERVIEWCONTAINER_H

#include "dfmplugin_computer_global.h"
#include <dfm-base/interfaces/abstractbaseview.h>

#include <QWidget>

namespace dfmplugin_computer {
class ComputerView;
class ComputerViewContainer : public QWidget, public DFMBASE_NAMESPACE::AbstractBaseView
{
public:
    explicit ComputerViewContainer(const QUrl &url, QWidget *parent = nullptr);

    virtual QWidget *widget() const override;
    virtual QWidget *contentWidget() const override;
    virtual QUrl rootUrl() const override;
    virtual ViewState viewState() const override;
    virtual bool setRootUrl(const QUrl &url) override;
    virtual QList<QUrl> selectedUrlList() const override;

private:
    ComputerView *view { nullptr };
    QWidget *viewContainer { nullptr };
};
}

#endif   // COMPUTERVIEWCONTAINER_H
