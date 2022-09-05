// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EXTENDVIEW_H
#define EXTENDVIEW_H

#include <QFrame>
#include "durl.h"
class QListWidget;
class DDetailView;


class ExtendView : public QFrame
{
    Q_OBJECT
public:
    explicit ExtendView(QWidget *parent = nullptr);
    ~ExtendView();

signals:

public slots:
    void setStartUrl(const DUrl &url);

private:
    void initUI();
    void initConnect();

private:
    QListWidget *m_extendListView = nullptr;
    DDetailView *m_detailView = nullptr;
    DUrl m_startUrl;
    DUrl m_currentUrl;
};

#endif // EXTENDVIEW_H
