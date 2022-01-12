/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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
#ifndef EXTENDEDCONTROLVIEW_H
#define EXTENDEDCONTROLVIEW_H

#include "dfm_base_global.h"

#include <DArrowLineDrawer>

#include <QFrame>

DFMBASE_BEGIN_NAMESPACE

/*!
 * \brief Property page control extended view interface class
 */
class ExtendedControlView : public QFrame
{
    Q_OBJECT
public:
    explicit ExtendedControlView(QWidget *parent = nullptr);

    /*!
     * \brief Set the URL of the selected file.
     *        Must be rewritten.
     * \param[in] url file url
     */
    virtual void setSelectFileUrl(const QUrl &url);
};

class ExtendedControlDrawerView : public DTK_WIDGET_NAMESPACE::DArrowLineDrawer
{
    Q_OBJECT
public:
    explicit ExtendedControlDrawerView(QWidget *parent = nullptr);

    /*!
     * \brief Set the URL of the selected file.
     *        Must be rewritten.
     * \param[in] url file url
     */
    virtual void setSelectFileUrl(const QUrl &url);

signals:
    void heightChanged(int height);
};
DFMBASE_END_NAMESPACE
#endif   // EXTENDEDCONTROLVIEW_H
