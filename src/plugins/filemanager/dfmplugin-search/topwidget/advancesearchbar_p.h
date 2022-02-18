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
#ifndef ADVANCESEARCHBAR_P_H
#define ADVANCESEARCHBAR_P_H

#include "dfmplugin_search_global.h"

#include "dfm-base/interfaces/abstractfileinfo.h"

#include <dboxwidget.h>

#include <QDateTime>

#include <mutex>

DWIDGET_BEGIN_NAMESPACE
class DCommandLinkButton;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QLabel;
class QComboBox;
class QVBoxLayout;
QT_END_NAMESPACE

DPSEARCH_BEGIN_NAMESPACE

class AdvanceSearchBar;
class AdvanceSearchBarPrivate : public DTK_WIDGET_NAMESPACE::DBoxWidget
{
    Q_OBJECT
public:
    enum LabelIndex {
        SEARCH_RANGE,
        FILE_TYPE,
        SIZE_RANGE,
        DATE_RANGE,
        ACCESS_DATE_RANGE,
        CREATE_DATE_RANGE,
        LABEL_COUNT
    };

    typedef struct fileFilter
    {
        QPair<quint64, quint64> sizeRange;
        QDateTime dateRangeStart;
        QDateTime dateRangeEnd;
        QDateTime accessDateRangeStart;
        QDateTime accessDateRangeEnd;
        QDateTime createDateRangeStart;
        QDateTime createDateRangeEnd;
        QString typeString;
        bool includeSubDir;
        bool comboValid[LABEL_COUNT];
    } FileFilter;

    explicit AdvanceSearchBarPrivate(AdvanceSearchBar *qq);
    void initUI();
    void initConnection();

    static bool shouldVisiableByFilterRule(dfmbase::AbstractFileInfo *info, QVariant data);
    static FileFilter parseFilterData(const QMap<int, QVariant> &data);

    QBoxLayout *mainLayout;
    QLabel *asbLabels[LABEL_COUNT];
    QComboBox *asbCombos[LABEL_COUNT];
    DTK_WIDGET_NAMESPACE::DCommandLinkButton *resetBtn;
    bool needSearchAgain = true;
    bool allowUpdateView = true;

    AdvanceSearchBar *q;
};

DPSEARCH_END_NAMESPACE

#endif   // ADVANCESEARCHBAR_P_H
