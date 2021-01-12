/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Mike Chen <kegechen@gmail.com>
 *
 * Maintainer: Mike Chen <chenke_cm@deepin.com>
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
#ifndef DFMTAGWIDGET_H
#define DFMTAGWIDGET_H
#include "dfmglobal.h"
#include "dtagactionwidget.h"

#include <QFrame>
#include <QObject>
#include <QSharedDataPointer>
#include <dcrumbedit.h>
DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

class DFMTagWidgetPrivate;
class DFMTagWidget : public QFrame
{
    Q_OBJECT
public:
    DFMTagWidget(DUrl url, QWidget *parent = nullptr);
    ~DFMTagWidget();

    void loadTags(const DUrl &durl);
    QWidget *tagTitle();
    QWidget *tagLeftTitle();
    DTagActionWidget *tagActionWidget();
    DCrumbEdit *tagCrumbEdit();

    static bool shouldShow(const DUrl &url);

protected:
    void initUi();
    void initConnection();
private:
    QScopedPointer<DFMTagWidgetPrivate> d_private;
    Q_DECLARE_PRIVATE_D(d_private, DFMTagWidget)
};

DFM_END_NAMESPACE

#endif // DFMTAGWIDGET_H
