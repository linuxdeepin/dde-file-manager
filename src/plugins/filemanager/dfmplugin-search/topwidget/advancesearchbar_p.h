// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ADVANCESEARCHBAR_P_H
#define ADVANCESEARCHBAR_P_H

#include "dfmplugin_search_global.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/sortfileinfo.h>

#include <dboxwidget.h>

#include <QDateTime>

#include <mutex>

DWIDGET_BEGIN_NAMESPACE
class DCommandLinkButton;
class DLabel;
class DComboBox;
DWIDGET_END_NAMESPACE

QT_BEGIN_NAMESPACE
class QLabel;
class QComboBox;
QT_END_NAMESPACE

DWIDGET_USE_NAMESPACE

using FilterCallback = std::function<bool(DFMBASE_NAMESPACE::SortFileInfo *info, QVariant data)>;

namespace dfmplugin_search {

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
    bool contains(const QUrl &url);
    void saveOptions(QMap<int, QVariant> &options);

    static bool shouldVisiableByFilterRule(DFMBASE_NAMESPACE::SortFileInfo *info, QVariant data);
    static FileFilter parseFilterData(const QMap<int, QVariant> &data);

public:
    QBoxLayout *mainLayout;
    DLabel *asbLabels[kLabelCount];
    DComboBox *asbCombos[kLabelCount];
    DTK_WIDGET_NAMESPACE::DCommandLinkButton *resetBtn;

    using FilterData = QMap<int, QVariant>;
    using FilterInfoCache = QHash<QUrl, FilterData>;
    FilterInfoCache filterInfoCache;
    QUrl currentSearchUrl;

    AdvanceSearchBar *q;
};

}

Q_DECLARE_METATYPE(FilterCallback)

#endif   // ADVANCESEARCHBAR_P_H
