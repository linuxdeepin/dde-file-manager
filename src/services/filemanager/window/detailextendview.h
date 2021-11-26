/*
 * Copyright (C) 2021 ~ 2022 Deepin Technology Co., Ltd.
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
#ifndef DETAILEXTENDVIEW_H
#define DETAILEXTENDVIEW_H
#include <QWidget>
/*!
 * \brief 用于最右信息控件扩展专用继承类
 */
class DetailExtendView : public QWidget
{
    Q_OBJECT
public:
    explicit DetailExtendView(QWidget *praent);
    virtual ~DetailExtendView();

    virtual void setFileUrl(QUrl &url) = 0;
};

#endif   // DETAILEXTENDVIEW_H
