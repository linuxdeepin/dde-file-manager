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
QT_END_NAMESPACE

DPSEARCH_BEGIN_NAMESPACE

class AdvanceSearchBar;
class AdvanceSearchBarPrivate : public DTK_WIDGET_NAMESPACE::DBoxWidget
{
    Q_OBJECT
public:
    enum LabelIndex {
        kSearchRange,
        kFileType,
        kSizeRange,
        kDateRange,
        kAccessDateRange,
        kCreateDateRange,
        kLabelCount,
        kCurrentUrl
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
        QUrl currentUrl;
        bool includeSubDir;
        bool comboValid[kLabelCount];
    } FileFilter;

    explicit AdvanceSearchBarPrivate(AdvanceSearchBar *qq);
    void initUI();
    void initConnection();
    void refreshOptions(const QUrl &url);

    static bool shouldVisiableByFilterRule(DFMBASE_NAMESPACE::AbstractFileInfo *info, QVariant data);
    static FileFilter parseFilterData(const QMap<int, QVariant> &data);

    QBoxLayout *mainLayout;
    QLabel *asbLabels[kLabelCount];
    QComboBox *asbCombos[kLabelCount];
    DTK_WIDGET_NAMESPACE::DCommandLinkButton *resetBtn;

    using FilterData = QMap<int, QVariant>;
    using FilterInfoCache = QHash<QUrl, FilterData>;
    FilterInfoCache filterInfoCache;

    AdvanceSearchBar *q;
};

DPSEARCH_END_NAMESPACE

#endif   // ADVANCESEARCHBAR_P_H
