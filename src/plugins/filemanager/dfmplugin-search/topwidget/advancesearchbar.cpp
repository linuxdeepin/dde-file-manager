// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "advancesearchbar.h"
#include "advancesearchbar_p.h"
#include "utils/searchhelper.h"
#include "utils/custommanager.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/sortutils.h>

#include <dfm-framework/dpf.h>

#include <QFileInfo>

#include <DCommandLinkButton>
#include <DHorizontalLine>
#include <DLabel>
#include <DComboBox>
#include <QVBoxLayout>
#include <QApplication>
#include <DGuiApplicationHelper>
#include <dtkwidget_global.h>
#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#    include <QGraphicsEffect>
#endif

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
namespace dfmplugin_search {

AdvanceSearchBarPrivate::AdvanceSearchBarPrivate(AdvanceSearchBar *qq)
    : DBoxWidget(QBoxLayout::LeftToRight, qq),
      q(qq)
{
    initUI();
    initConnection();
}

void AdvanceSearchBarPrivate::initUI()
{
    mainLayout = this->layout();
    QGridLayout *formLayout = new QGridLayout;
    auto createLabelCombo = [=](int index, const QString &labelText) {
        asbLabels[index] = new DLabel(labelText);
        asbCombos[index] = new DComboBox(this);
#ifndef ARM_PROCESSOR
        // 针对ARM部分机器上搜索高级选项下拉框不能使用做特殊处理
        asbCombos[index]->setFocusPolicy(Qt::NoFocus);
#endif
        asbLabels[index]->setBuddy(asbCombos[index]);
    };

    createLabelCombo(kSearchRange, tr("Search:"));
    createLabelCombo(kFileType, tr("File Type:"));
    createLabelCombo(kSizeRange, tr("File Size:"));
    createLabelCombo(kDateRange, tr("Time Modified:"));
    createLabelCombo(kAccessDateRange, tr("Time Accessed:"));
    createLabelCombo(kCreateDateRange, tr("Time Created:"));

    resetBtn = new DCommandLinkButton(tr("Reset"), this);
#ifndef ARM_PROCESSOR
    // 针对ARM部分机器上搜索高级选项下拉框不能使用做特殊处理
    resetBtn->setFocusPolicy(Qt::NoFocus);
#endif

    static int labelWidth = 70;
    static int comboMinWidth = 140;
    asbLabels[kSearchRange]->setMinimumWidth(labelWidth);
    asbCombos[kSearchRange]->setMinimumWidth(comboMinWidth);
    asbLabels[kSizeRange]->setMinimumWidth(labelWidth);
    asbCombos[kSizeRange]->setMinimumWidth(comboMinWidth);
    asbLabels[kFileType]->setMinimumWidth(labelWidth);
    asbCombos[kFileType]->setMinimumWidth(comboMinWidth);
    asbLabels[kDateRange]->setMinimumWidth(labelWidth);
    asbCombos[kDateRange]->setMinimumWidth(comboMinWidth);
    asbLabels[kAccessDateRange]->setMinimumWidth(labelWidth);
    asbCombos[kAccessDateRange]->setMinimumWidth(comboMinWidth);
    asbLabels[kCreateDateRange]->setMinimumWidth(labelWidth);
    asbCombos[kCreateDateRange]->setMinimumWidth(comboMinWidth);

    asbCombos[kSearchRange]->addItem(tr("All subdirectories"), QVariant::fromValue(true));
    asbCombos[kSearchRange]->addItem(tr("Current directory"), QVariant::fromValue(false));
    auto addItemToFileTypeCombo = [this](const QString &typeStr) {
        asbCombos[kFileType]->addItem(typeStr, QVariant::fromValue(typeStr));
    };

    asbCombos[kFileType]->addItem("--", QVariant());
    addItemToFileTypeCombo(tr("Application"));
    addItemToFileTypeCombo(tr("Video"));
    addItemToFileTypeCombo(tr("Audio"));
    addItemToFileTypeCombo(tr("Image"));
    addItemToFileTypeCombo(tr("Archive"));
    addItemToFileTypeCombo(tr("Text"));
    addItemToFileTypeCombo(tr("Executable"));
    addItemToFileTypeCombo(tr("Backup file"));

    asbCombos[kSizeRange]->addItem("--", QVariant());
    asbCombos[kSizeRange]->addItem("0 ~ 100 KB", QVariant::fromValue(QPair<quint64, quint64>(0, 100)));
    asbCombos[kSizeRange]->addItem("100 KB ~ 1 MB", QVariant::fromValue(QPair<quint64, quint64>(100, 1024)));
    asbCombos[kSizeRange]->addItem("1 MB ~ 10 MB", QVariant::fromValue(QPair<quint64, quint64>(1024, 10 * 1024)));
    asbCombos[kSizeRange]->addItem("10 MB ~ 100 MB", QVariant::fromValue(QPair<quint64, quint64>(10 * 1024, 100 * 1024)));
    asbCombos[kSizeRange]->addItem("100 MB ~ 1 GB", QVariant::fromValue(QPair<quint64, quint64>(100 * 1024, 1 << 20)));
    asbCombos[kSizeRange]->addItem("> 1 GB", QVariant::fromValue(QPair<quint64, quint64>(1 << 20, 1 << 30)));   // here to 1T

    auto createDateCombos = [=](const LabelIndex index) {
        asbCombos[index]->addItem("--", QVariant());
        asbCombos[index]->addItem(tr("Today"), QVariant::fromValue(1));
        asbCombos[index]->addItem(tr("Yesterday"), QVariant::fromValue(2));
        asbCombos[index]->addItem(tr("This week"), QVariant::fromValue(7));
        asbCombos[index]->addItem(tr("Last week"), QVariant::fromValue(14));
        asbCombos[index]->addItem(tr("This month"), QVariant::fromValue(30));
        asbCombos[index]->addItem(tr("Last month"), QVariant::fromValue(60));
        asbCombos[index]->addItem(tr("This year"), QVariant::fromValue(365));
        asbCombos[index]->addItem(tr("Last year"), QVariant::fromValue(730));
    };
    createDateCombos(kDateRange);
    createDateCombos(kAccessDateRange);
    createDateCombos(kCreateDateRange);

    formLayout->addWidget(asbLabels[kSearchRange], 0, 0);
    formLayout->addWidget(asbCombos[kSearchRange], 0, 1);
    formLayout->addItem(new QSpacerItem(10, 1), 0, 2);
    formLayout->addWidget(asbLabels[kFileType], 0, 3);
    formLayout->addWidget(asbCombos[kFileType], 0, 4);
    formLayout->addItem(new QSpacerItem(10, 1), 0, 5);
    formLayout->addWidget(asbLabels[kAccessDateRange], 0, 6);
    formLayout->addWidget(asbCombos[kAccessDateRange], 0, 7);
    formLayout->addItem(new QSpacerItem(10, 1), 0, 8);
    formLayout->addWidget(resetBtn, 0, 9);

    formLayout->addWidget(asbLabels[kSizeRange], 1, 0);
    formLayout->addWidget(asbCombos[kSizeRange], 1, 1);
    formLayout->addWidget(asbLabels[kDateRange], 1, 3);
    formLayout->addWidget(asbCombos[kDateRange], 1, 4);
    formLayout->addWidget(asbLabels[kCreateDateRange], 1, 6);
    formLayout->addWidget(asbCombos[kCreateDateRange], 1, 7);

    formLayout->setSpacing(6);
    formLayout->setContentsMargins(6, 6, 6, 6);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->addSpacing(10);
    vLayout->addLayout(formLayout);

    mainLayout->addSpacing(20);
    mainLayout->addLayout(vLayout);
    mainLayout->addSpacing(20);

    q->setWidget(this);
    q->setFrameShape(QFrame::NoFrame);
    q->setAutoFillBackground(true);   // 允许自动填充背景
    q->setBackgroundRole(QPalette::Base);
    // 启用横向滚动条
    q->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    // 禁用竖向滚动条
    q->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void AdvanceSearchBarPrivate::initConnection()
{
    connect(resetBtn, &DCommandLinkButton::pressed, q, &AdvanceSearchBar::onResetButtonPressed);

    for (int i = 0; i < kLabelCount; i++) {
        connect(asbCombos[i], static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), q, &AdvanceSearchBar::onOptionChanged);
    }
}

void AdvanceSearchBarPrivate::refreshOptions(const QUrl &url)
{
    if (!contains(url)) {
        fmDebug() << "URL not found in cache, resetting form";
        q->resetForm();
        return;
    }

    static QMap<int, int> dateRangeMap {
        { 1, 1 },
        { 2, 2 },
        { 7, 3 },
        { 14, 4 },
        { 30, 5 },
        { 60, 6 },
        { 365, 7 },
        { 730, 8 }
    };

    static QMap<QPair<quint64, quint64>, int> sizeRangeMap { { QPair<quint64, quint64>(0, 100), 1 },
                                                             { QPair<quint64, quint64>(100, 1024), 2 },
                                                             { QPair<quint64, quint64>(1024, 10 * 1024), 3 },
                                                             { QPair<quint64, quint64>(10 * 1024, 100 * 1024), 4 },
                                                             { QPair<quint64, quint64>(100 * 1024, 1 << 20), 5 },
                                                             { QPair<quint64, quint64>(1 << 20, 1 << 30), 6 } };

    blockSignals(true);
    FilterData filter = filterInfoCache[url];

    fmDebug() << "Applying cached filter data for URL:" << url.toString();

    // 搜索范围
    const auto &searchRange = filter[kSearchRange];
    asbCombos[kSearchRange]->setCurrentIndex(searchRange.toBool() ? 0 : 1);

    // 文件类型
    const auto &fileType = filter[kFileType];
    if (fileType.isValid()) {
        asbCombos[kFileType]->setCurrentText(fileType.toString());
        fmDebug() << "Set file type filter to:" << fileType.toString();
    } else {
        asbCombos[kFileType]->setCurrentIndex(0);
    }

    // 文件大小
    const auto &sizeRange = filter[kSizeRange];
    if (sizeRange.isValid() && sizeRange.canConvert<QPair<quint64, quint64>>()) {
        auto range = sizeRange.value<QPair<quint64, quint64>>();
        asbCombos[kSizeRange]->setCurrentIndex(sizeRangeMap[range]);
        fmDebug() << "Set size range filter:" << range.first << "KB to" << range.second << "KB";
    } else {
        asbCombos[kSizeRange]->setCurrentIndex(0);
    }

    // 修改时间
    const auto &dateRange = filter[kDateRange];
    if (dateRange.isValid()) {
        int days = dateRange.toInt();
        asbCombos[kDateRange]->setCurrentIndex(dateRangeMap.value(days, 0));
        fmDebug() << "Set date range filter to" << days << "days";
    } else {
        asbCombos[kDateRange]->setCurrentIndex(0);
    }

    // 访问时间
    const auto &accessDateRange = filter[kAccessDateRange];
    if (accessDateRange.isValid()) {
        int days = accessDateRange.toInt();
        asbCombos[kAccessDateRange]->setCurrentIndex(dateRangeMap.value(days, 0));
        fmDebug() << "Set access date range filter to" << days << "days";
    } else {
        asbCombos[kAccessDateRange]->setCurrentIndex(0);
    }

    // 创建时间
    const auto &createDateRange = filter[kCreateDateRange];
    if (createDateRange.isValid()) {
        int days = createDateRange.toInt();
        asbCombos[kCreateDateRange]->setCurrentIndex(dateRangeMap.value(days, 0));
        fmDebug() << "Set create date range filter to" << days << "days";
    } else {
        asbCombos[kCreateDateRange]->setCurrentIndex(0);
    }

    blockSignals(false);
    q->onOptionChanged();
}

bool AdvanceSearchBarPrivate::contains(const QUrl &url)
{
    if (filterInfoCache.contains(url)) {
        fmDebug() << "URL found directly in cache:" << url.toString();
        return true;
    }

    // If there is an entry in `filterInfoCache `that is the same as winId and targetUrl in `url`,
    // then it is considered that `filterInfoCache` contains `url`.
    const auto &targetUrl = SearchHelper::searchTargetUrl(url);
    auto winId = SearchHelper::searchWinId(url);
    const auto &keys = filterInfoCache.keys();
    auto iter = std::find_if(keys.begin(), keys.end(), [&targetUrl, winId](const QUrl &urlKey) {
        auto tmpWinId = SearchHelper::searchWinId(urlKey);
        const auto &tmpTargetUrl = SearchHelper::searchTargetUrl(urlKey);
        return (tmpWinId == winId && UniversalUtils::urlEquals(tmpTargetUrl, targetUrl));
    });

    if (iter == keys.end()) {
        fmDebug() << "URL not found in cache with winId:" << winId << "targetUrl:" << targetUrl.toString();
        return false;
    }

    // update and return true
    auto cache = filterInfoCache.take(*iter);
    filterInfoCache.insert(url, cache);
    return true;
}

void AdvanceSearchBarPrivate::saveOptions(QMap<int, QVariant> &options)
{
    auto winId = FMWindowsIns.findWindowId(this);
    auto window = FMWindowsIns.findWindowById(winId);
    Q_ASSERT(window);

    const auto &url = window->currentUrl();
    if (!url.isValid()) {
        fmWarning() << "Cannot save search options: invalid current URL";
        return;
    }

    fmDebug() << "Saving search options for winId:" << winId << "URL:" << url.toString();

    if (!currentSearchUrl.isValid() || !SearchHelper::isSearchFile(currentSearchUrl)) {
        if (!SearchHelper::isSearchFile(url)) {
            const auto &searchUrl = SearchHelper::fromSearchFile(url, "", QString::number(winId));
            options[AdvanceSearchBarPrivate::kCurrentUrl] = searchUrl;
            filterInfoCache[searchUrl] = options;
        }
    }

    currentSearchUrl = url;
    options[AdvanceSearchBarPrivate::kCurrentUrl] = currentSearchUrl;
    filterInfoCache[currentSearchUrl] = options;
}

bool AdvanceSearchBarPrivate::shouldVisiableByFilterRule(SortFileInfo *info, QVariant data)
{
    if (!data.isValid())
        return true;

    auto filterData = data.value<FilterData>();
    // 如果是重置过滤条件，则都显示
    const auto &iter = std::find_if(std::next(filterData.begin()), std::next(filterData.begin(), kLabelCount), [](const QVariant &value) {
        return value.isValid();
    });
    if (filterData[kSearchRange].toBool() && (iter == std::next(filterData.begin(), kLabelCount) || iter == filterData.end()))
        return true;

    if (!info)
        return false;
    // 检查文件是否存在，如果不存在则直接过滤掉
    const auto &fileUrl = info->fileUrl();
    if (fileUrl.isLocalFile() && !QFileInfo(fileUrl.toLocalFile()).exists())
        return false;

    const auto &filter = parseFilterData(filterData);

    // 检查搜索范围过滤
    if (SearchHelper::isSearchFile(filter.currentUrl) && filter.comboValid[kSearchRange] && !filter.includeSubDir) {
        const QUrl &parentUrl = SearchHelper::searchTargetUrl(filter.currentUrl);
        QString parentPath = CustomManager::instance()->redirectedPath(parentUrl);
        if (parentPath.isEmpty())
            parentPath = parentUrl.toLocalFile();

        if (!parentPath.endsWith("/"))
            parentPath += '/';

        QString filePath = info->fileUrl().path();
        int index = filePath.indexOf(parentPath);
        if (index != -1) {
            int indexWithoutParent = index + parentPath.length();
            filePath = filePath.mid(indexWithoutParent);
            if (filePath.contains('/'))
                return false;
        }
    }

    // 检查文件类型过滤
    if (filter.comboValid[kFileType]) {
        QString fileTypeStr = SortUtils::accurateDisplayType(info->fileUrl());
        if (!fileTypeStr.startsWith(filter.typeString))
            return false;
    }

    // 检查文件大小过滤
    if (filter.comboValid[kSizeRange]) {
        // note: FileSizeInKiloByteRole is the size of Byte, not KB!
        quint64 fileSize = static_cast<quint64>(info->fileSize());
        quint32 blockSize = 1 << 10;
        quint64 lower = filter.sizeRange.first * blockSize;
        quint64 upper = filter.sizeRange.second * blockSize;
        // filter file size in Bytes, not Kilobytes
        if (fileSize < lower || fileSize > upper)
            return false;
    }

    // 检查修改时间过滤
    if (filter.comboValid[kDateRange]) {
        QDateTime filemtime = QDateTime::fromSecsSinceEpoch(info->lastModifiedTime());
        if (filemtime < filter.dateRangeStart || filemtime > filter.dateRangeEnd)
            return false;
    }

    // 检查访问时间过滤
    if (filter.comboValid[kAccessDateRange]) {
        QDateTime filemtime = QDateTime::fromSecsSinceEpoch(info->lastReadTime());
        if (filemtime < filter.accessDateRangeStart || filemtime > filter.accessDateRangeEnd)
            return false;
    }

    // 检查创建时间过滤
    if (filter.comboValid[kCreateDateRange]) {
        QDateTime filemtime = QDateTime::fromSecsSinceEpoch(info->createTime());
        if (filemtime < filter.createDateRangeStart || filemtime > filter.createDateRangeEnd)
            return false;
    }

    return true;
}

AdvanceSearchBarPrivate::FileFilter AdvanceSearchBarPrivate::parseFilterData(const QMap<int, QVariant> &data)
{
    FileFilter filter;
    filter.currentUrl = data[kCurrentUrl].toUrl();
    filter.comboValid[kSearchRange] = true;
    filter.includeSubDir = data[kSearchRange].toBool();
    filter.typeString = data[kFileType].toString();
    filter.comboValid[kFileType] = !filter.typeString.isEmpty();
    filter.comboValid[kSizeRange] = data[kSizeRange].canConvert<QPair<quint64, quint64>>();
    if (filter.comboValid[kSizeRange])
        filter.sizeRange = data[kSizeRange].value<QPair<quint64, quint64>>();

    // 计算时间过滤条件
    auto calDateFilter = [&filter, &data](LabelIndex labelIndex, QDateTime &startTime, QDateTime &endTime) {
        int dateRange = data[labelIndex].toInt();
        filter.comboValid[labelIndex] = (dateRange != 0);

        if (filter.comboValid[labelIndex]) {
            int firstDayOfWeek = QLocale::system().firstDayOfWeek();
            QDate today = QDate::currentDate();
            QDate tomorrow = QDate::currentDate().addDays(+1);
            int dayDist = today.dayOfWeek() - firstDayOfWeek;
            if (dayDist < 0) dayDist += 7;

            switch (dateRange) {
            case 1:
                startTime = today.startOfDay();
                endTime = tomorrow.startOfDay();
                break;
            case 2:
                startTime = today.addDays(-1).startOfDay();
                endTime = today.startOfDay();
                break;
            case 7:
                startTime = today.addDays(0 - dayDist).startOfDay();
                endTime = tomorrow.startOfDay();
                break;
            case 14:
                startTime = today.addDays(-7 - dayDist).startOfDay();
                endTime = today.addDays(0 - dayDist).startOfDay();
                break;
            case 30:
                startTime = QDate(today.year(), today.month(), 1).startOfDay();
                endTime = tomorrow.startOfDay();
                break;
            case 60:
                startTime = QDate(today.year(), today.month(), 1).addMonths(-1).startOfDay();
                endTime = QDate(today.year(), today.month(), 1).startOfDay();
                break;
            case 365:
                startTime = QDate(today.year(), 1, 1).startOfDay();
                endTime = tomorrow.startOfDay();
                break;
            case 730:
                startTime = QDate(today.year(), 1, 1).addYears(-1).startOfDay();
                endTime = QDate(today.year(), 1, 1).startOfDay();
                break;
            default:
                break;
            }
        }
    };

    calDateFilter(kDateRange, filter.dateRangeStart, filter.dateRangeEnd);
    calDateFilter(kAccessDateRange, filter.accessDateRangeStart, filter.accessDateRangeEnd);
    calDateFilter(kCreateDateRange, filter.createDateRangeStart, filter.createDateRangeEnd);

    return filter;
}

AdvanceSearchBar::AdvanceSearchBar(QWidget *parent)
    : QScrollArea(parent),
      d(new AdvanceSearchBarPrivate(this))
{
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setColor(QColor(0, 0, 0, 5));
    effect->setOffset(0, 4);
    effect->setBlurRadius(20);
    setGraphicsEffect(effect);
#ifdef DTKWIDGET_CLASS_DSizeMode
    setFixedHeight(DSizeModeHelper::element(83, 110));
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, &AdvanceSearchBar::initUiForSizeMode);
#endif
}

void AdvanceSearchBar::initUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    setFixedHeight(DSizeModeHelper::element(83, 110));
#endif
}

void AdvanceSearchBar::resetForm()
{
    bool changed = false;
    for (int i = 0; i < AdvanceSearchBarPrivate::kLabelCount; ++i) {
        if (d->asbCombos[i]->currentIndex() != 0)
            changed = true;
        QSignalBlocker blocker(d->asbCombos[i]);
        d->asbCombos[i]->setCurrentIndex(0);
    }

    if (changed)
        onOptionChanged();
}

void AdvanceSearchBar::refreshOptions(const QUrl &url)
{
    d->refreshOptions(url);
}

void AdvanceSearchBar::onOptionChanged()
{
    QMap<int, QVariant> formData;
    formData[AdvanceSearchBarPrivate::kSearchRange] = d->asbCombos[AdvanceSearchBarPrivate::kSearchRange]->currentData();
    formData[AdvanceSearchBarPrivate::kFileType] = d->asbCombos[AdvanceSearchBarPrivate::kFileType]->currentData();
    formData[AdvanceSearchBarPrivate::kSizeRange] = d->asbCombos[AdvanceSearchBarPrivate::kSizeRange]->currentData();
    formData[AdvanceSearchBarPrivate::kDateRange] = d->asbCombos[AdvanceSearchBarPrivate::kDateRange]->currentData();
    formData[AdvanceSearchBarPrivate::kAccessDateRange] = d->asbCombos[AdvanceSearchBarPrivate::kAccessDateRange]->currentData();
    formData[AdvanceSearchBarPrivate::kCreateDateRange] = d->asbCombos[AdvanceSearchBarPrivate::kCreateDateRange]->currentData();

    d->saveOptions(formData);

    auto winId = FMWindowsIns.findWindowId(this);
    dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetCustomFilterData", winId, d->currentSearchUrl, QVariant::fromValue(formData));

    FilterCallback callback { AdvanceSearchBarPrivate::shouldVisiableByFilterRule };
    dpfSlotChannel->push("dfmplugin_workspace", "slot_Model_SetCustomFilterCallback", winId, d->currentSearchUrl, QVariant::fromValue(callback));
}

void AdvanceSearchBar::onResetButtonPressed()
{
    resetForm();
}

void AdvanceSearchBar::hideEvent(QHideEvent *event)
{
    fmDebug() << "AdvanceSearchBar hidden, cleaning up resources";

    auto winId = FMWindowsIns.findWindowId(this);
    auto window = FMWindowsIns.findWindowById(winId);
    if (window && !window->isMinimized()) {
        fmDebug() << "Resetting form and clearing cache on hide";
        resetForm();
        d->filterInfoCache.clear();
        d->currentSearchUrl = QUrl();
    } else {
        fmDebug() << "Window minimized or not found, skipping cleanup";
    }

    QScrollArea::hideEvent(event);
}

}
