/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef DETAILVIEW_H
#define DETAILVIEW_H

#include "dfmplugin_detailspace_global.h"
#include "filebaseinfoview.h"

#include <QFrame>
#include <QVBoxLayout>

class QScrollArea;

DPDETAILSPACE_BEGIN_NAMESPACE

class DetailView : public QFrame
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

private:
    void initInfoUI();

    void createHeadUI(const QUrl &url, int widgetFilter);

    void createBasicWidget(const QUrl &url, int widgetFilter);

protected:
    virtual void showEvent(QShowEvent *event);

private:
    QScrollArea *scrollArea { nullptr };
    QFrame *expandFrame { nullptr };
    QVBoxLayout *mainLayout { nullptr };
    FileBaseInfoView *fileBaseInfoView { nullptr };
    QList<QWidget *> expandList {};
    QLabel *iconLabel { nullptr };
};

DPDETAILSPACE_END_NAMESPACE

#endif   // DETAILVIEW_H
