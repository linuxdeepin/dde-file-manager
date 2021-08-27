/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
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
#ifndef DFMNAVWIDGET_H
#define DFMNAVWIDGET_H

#include "dfm_filemanager_service_global.h"

#include <DButtonBox>

#include <QUrl>
#include <QWidget>
#include <QHBoxLayout>

DWIDGET_USE_NAMESPACE

DSB_FM_BEGIN_NAMESPACE

class DFMNavWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DFMNavWidget(QWidget *parent = nullptr);

    DButtonBoxButton *navBackButton() const;
    void setNavBackButton(DButtonBoxButton *navBackButton);

    DButtonBoxButton *navForwardButton() const;
    void setNavForwardButton(DButtonBoxButton *navForwardButton);

private:
    DButtonBox *m_buttonBox = nullptr;
    DButtonBoxButton *m_navBackButton = nullptr;
    DButtonBoxButton *m_navForwardButton = nullptr;
    QHBoxLayout *m_hboxLayout = nullptr;

private Q_SLOTS:
   void doButtonClicked();

public Q_SLOTS:
    void appendUrl(const QUrl &url);

Q_SIGNALS:
    void releaseUrl(const QUrl &url);
};

DSB_FM_END_NAMESPACE

#endif // DFMNAVWIDGET_H
