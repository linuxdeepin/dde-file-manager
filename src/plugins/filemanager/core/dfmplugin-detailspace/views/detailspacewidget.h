// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILSPACEWIDGET_H
#define DETAILSPACEWIDGET_H

#include "dfmplugin_detailspace_global.h"
#include <dfm-base/interfaces/abstractframe.h>

#include <QUrl>

namespace dfmplugin_detailspace {

class DetailView;
class DetailSpaceWidget : public DFMBASE_NAMESPACE::AbstractFrame
{
    Q_OBJECT
    Q_DISABLE_COPY(DetailSpaceWidget)

public:
    explicit DetailSpaceWidget(QFrame *parent = nullptr);
    void setCurrentUrl(const QUrl &url) override;
    void setCurrentUrl(const QUrl &url, int widgetFilter);
    QUrl currentUrl() const override;

    bool insterExpandControl(const int &index, QWidget *widget);

    void removeControls();

private slots:
    void initUiForSizeMode();

private:
    void initializeUi();
    void initConnect();

private:
    QUrl detailSpaceUrl;
    DetailView *detailView { nullptr };
};

}

#endif   // DETAILSPACEWIDGET_H
