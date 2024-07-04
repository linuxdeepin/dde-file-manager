// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DETAILVIEW_H
#define DETAILVIEW_H

#include "dfmplugin_detailspace_global.h"
#include "filebaseinfoview.h"

#include <DFrame>
#include <QVBoxLayout>

class QScrollArea;

namespace dfmplugin_detailspace {

class DetailView : public DFrame
{
    Q_OBJECT
public:
    explicit DetailView(QWidget *parent = nullptr);
    virtual ~DetailView();

    bool addCustomControl(QWidget *widget);
    bool insertCustomControl(int index, QWidget *widget);

    void removeControl();

public slots:
    void setUrl(const QUrl &url, int widgetFilter);

private slots:
    void initUiForSizeMode();

private:
    void initInfoUI();

    void createHeadUI(const QUrl &url, int widgetFilter);

    void createBasicWidget(const QUrl &url, int widgetFilter);

protected:
    virtual void showEvent(QShowEvent *event);

private:
    QScrollArea *scrollArea { nullptr };
    QFrame *expandFrame { nullptr };
    QVBoxLayout *vLayout { nullptr };
    QVBoxLayout *mainLayout { nullptr };
    FileBaseInfoView *fileBaseInfoView { nullptr };
    QList<QWidget *> expandList {};
    DLabel *iconLabel { nullptr };
};

}

#endif   // DETAILVIEW_H
