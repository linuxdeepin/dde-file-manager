/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#ifndef ADVANCESEARCHBAR_H
#define ADVANCESEARCHBAR_H

#include "dfmplugin_search_global.h"

#include <dboxwidget.h>

#include <QScrollArea>

DPSEARCH_BEGIN_NAMESPACE

class AdvanceSearchBarPrivate;
class AdvanceSearchBar : public QScrollArea
{
    Q_OBJECT
public:
    explicit AdvanceSearchBar(QWidget *parent = nullptr);
    void resetForm();
    void refreshOptions(const QUrl &url);

public slots:
    void onOptionChanged();
    void onResetButtonPressed();

protected:
    void hideEvent(QHideEvent *event) override;

private:
    AdvanceSearchBarPrivate *d;
};

DPSEARCH_END_NAMESPACE

#endif   // ADVANCESEARCHBAR_H
