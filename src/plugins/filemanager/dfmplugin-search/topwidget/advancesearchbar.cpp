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
#include "advancesearchbar.h"
#include "advancesearchbar_p.h"
#include "fileinfo/searchfileinfo.h"
#include "utils/searchhelper.h"

#include "services/filemanager/workspace/workspaceservice.h"
#include "services/filemanager/windows/windowsservice.h"

#include <DCommandLinkButton>
#include <DHorizontalLine>

#include <QLabel>
#include <QComboBox>
#include <QVBoxLayout>
#include <QApplication>

DSB_FM_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
DPSEARCH_BEGIN_NAMESPACE

namespace GlobalPrivate {
static WindowsService *winServ { nullptr };
static WorkspaceService *workspaceServ { nullptr };
}   // namespace GlobalPrivate

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
        asbLabels[index] = new QLabel(labelText);
        asbCombos[index] = new QComboBox(this);
#ifndef ARM_PROCESSOR
        // 针对ARM部分机器上搜索高级选项下拉框不能使用做特殊处理
        asbCombos[index]->setFocusPolicy(Qt::NoFocus);
#endif
        asbLabels[index]->setBuddy(asbCombos[index]);
    };

    createLabelCombo(SEARCH_RANGE, tr("Search:"));
    createLabelCombo(FILE_TYPE, tr("File Type:"));
    createLabelCombo(SIZE_RANGE, tr("File Size:"));
    createLabelCombo(DATE_RANGE, tr("Time Modified:"));
    createLabelCombo(ACCESS_DATE_RANGE, tr("Time Accessed:"));
    createLabelCombo(CREATE_DATE_RANGE, tr("Time Created:"));

    resetBtn = new DCommandLinkButton(tr("Reset"), this);
#ifndef ARM_PROCESSOR
    // 针对ARM部分机器上搜索高级选项下拉框不能使用做特殊处理
    resetBtn->setFocusPolicy(Qt::NoFocus);
#endif

    static int labelWidth = 70;
    static int comboMinWidth = 120;
    asbLabels[SEARCH_RANGE]->setMinimumWidth(labelWidth);
    asbCombos[SEARCH_RANGE]->setMinimumWidth(comboMinWidth);
    asbLabels[SIZE_RANGE]->setMinimumWidth(labelWidth);
    asbCombos[SIZE_RANGE]->setMinimumWidth(comboMinWidth);
    asbLabels[FILE_TYPE]->setMinimumWidth(labelWidth);
    asbCombos[FILE_TYPE]->setMinimumWidth(comboMinWidth);
    asbLabels[DATE_RANGE]->setMinimumWidth(labelWidth);
    asbCombos[DATE_RANGE]->setMinimumWidth(comboMinWidth);
    asbLabels[ACCESS_DATE_RANGE]->setMinimumWidth(labelWidth);
    asbCombos[ACCESS_DATE_RANGE]->setMinimumWidth(comboMinWidth);
    asbLabels[CREATE_DATE_RANGE]->setMinimumWidth(labelWidth);
    asbCombos[CREATE_DATE_RANGE]->setMinimumWidth(comboMinWidth);

    asbCombos[SEARCH_RANGE]->addItem(tr("All subdirectories"), QVariant::fromValue(true));
    asbCombos[SEARCH_RANGE]->addItem(tr("Current directory"), QVariant::fromValue(false));
    auto addItemToFileTypeCombo = [this](const QString &typeStr) {
        asbCombos[FILE_TYPE]->addItem(typeStr, QVariant::fromValue(typeStr));
    };

    asbCombos[FILE_TYPE]->addItem("--", QVariant());
    addItemToFileTypeCombo(tr("Application"));
    addItemToFileTypeCombo(tr("Video"));
    addItemToFileTypeCombo(tr("Audio"));
    addItemToFileTypeCombo(tr("Image"));
    addItemToFileTypeCombo(tr("Archive"));
    addItemToFileTypeCombo(tr("Text"));
    addItemToFileTypeCombo(tr("Executable"));
    addItemToFileTypeCombo(tr("Backup file"));

    asbCombos[SIZE_RANGE]->addItem("--", QVariant());
    asbCombos[SIZE_RANGE]->addItem("0 ~ 100 KB", QVariant::fromValue(QPair<quint64, quint64>(0, 100)));
    asbCombos[SIZE_RANGE]->addItem("100 KB ~ 1 MB", QVariant::fromValue(QPair<quint64, quint64>(100, 1024)));
    asbCombos[SIZE_RANGE]->addItem("1 MB ~ 10 MB", QVariant::fromValue(QPair<quint64, quint64>(1024, 10 * 1024)));
    asbCombos[SIZE_RANGE]->addItem("10 MB ~ 100 MB", QVariant::fromValue(QPair<quint64, quint64>(10 * 1024, 100 * 1024)));
    asbCombos[SIZE_RANGE]->addItem("100 MB ~ 1 GB", QVariant::fromValue(QPair<quint64, quint64>(100 * 1024, 1 << 20)));
    asbCombos[SIZE_RANGE]->addItem("> 1 GB", QVariant::fromValue(QPair<quint64, quint64>(1 << 20, 1 << 30)));   // here to 1T

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
    createDateCombos(DATE_RANGE);
    createDateCombos(ACCESS_DATE_RANGE);
    createDateCombos(CREATE_DATE_RANGE);

    formLayout->addWidget(asbLabels[SEARCH_RANGE], 0, 0);
    formLayout->addWidget(asbCombos[SEARCH_RANGE], 0, 1);
    formLayout->addItem(new QSpacerItem(10, 1), 0, 2);
    formLayout->addWidget(asbLabels[FILE_TYPE], 0, 3);
    formLayout->addWidget(asbCombos[FILE_TYPE], 0, 4);
    formLayout->addItem(new QSpacerItem(10, 1), 0, 5);
    formLayout->addWidget(asbLabels[ACCESS_DATE_RANGE], 0, 6);
    formLayout->addWidget(asbCombos[ACCESS_DATE_RANGE], 0, 7);
    formLayout->addItem(new QSpacerItem(10, 1), 0, 8);
    formLayout->addWidget(resetBtn, 0, 9);

    formLayout->addWidget(asbLabels[SIZE_RANGE], 1, 0);
    formLayout->addWidget(asbCombos[SIZE_RANGE], 1, 1);
    formLayout->addWidget(asbLabels[DATE_RANGE], 1, 3);
    formLayout->addWidget(asbCombos[DATE_RANGE], 1, 4);
    formLayout->addWidget(asbLabels[CREATE_DATE_RANGE], 1, 6);
    formLayout->addWidget(asbCombos[CREATE_DATE_RANGE], 1, 7);

    formLayout->setSpacing(6);
    formLayout->setMargin(6);

    mainLayout->addSpacing(20);
    mainLayout->addLayout(formLayout);
    mainLayout->addSpacing(20);

    q->setWidget(this);
    // 禁用横向滚动条
    q->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 禁用竖向滚动条
    q->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

void AdvanceSearchBarPrivate::initConnection()
{
    connect(resetBtn, &DCommandLinkButton::pressed, q, &AdvanceSearchBar::onResetButtonPressed);

    for (int i = 0; i < LABEL_COUNT; i++) {
        connect(asbCombos[i], static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), q, &AdvanceSearchBar::onOptionChanged);
    }
}

bool AdvanceSearchBarPrivate::shouldVisiableByFilterRule(AbstractFileInfo *info, QVariant data)
{
    if (!data.isValid())
        return true;

    auto filterData = data.value<QMap<int, QVariant>>();
    // 如果是重置过滤条件，则都显示
    const auto &iter = std::find_if(filterData.begin() + 1, filterData.end(), [](const QVariant &value) {
        return value.isValid();
    });
    if (filterData[SEARCH_RANGE].toBool() && iter == filterData.end())
        return true;

    SearchFileInfo *fileInfo = dynamic_cast<SearchFileInfo *>(info);
    if (!fileInfo)
        return false;

    const auto &filter = parseFilterData(filterData);
    if (filter.comboValid[SEARCH_RANGE] && !filter.includeSubDir) {
        const QUrl &parentUrl = SearchHelper::searchTargetUrl(fileInfo->url());
        QString filePath = fileInfo->filePath();
        filePath.remove(parentUrl.toLocalFile().endsWith("/") ? parentUrl.toLocalFile() : parentUrl.toLocalFile() + '/');
        if (filePath.contains('/'))
            return false;
    }

    if (filter.comboValid[FILE_TYPE]) {
        QString fileTypeStr = fileInfo->mimeTypeDisplayName();
        if (!fileTypeStr.startsWith(filter.typeString))
            return false;
    }

    if (filter.comboValid[SIZE_RANGE]) {
        // note: FileSizeInKiloByteRole is the size of Byte, not KB!
        quint64 fileSize = fileInfo->size();
        quint32 blockSize = 1 << 10;
        quint64 lower = filter.sizeRange.first * blockSize;
        quint64 upper = filter.sizeRange.second * blockSize;
        // filter file size in Bytes, not Kilobytes
        if (fileSize < lower || fileSize > upper)
            return false;
    }

    if (filter.comboValid[DATE_RANGE]) {
        QDateTime filemtime = fileInfo->lastModified();
        if (filemtime < filter.dateRangeStart || filemtime > filter.dateRangeEnd)
            return false;
    }

    if (filter.comboValid[ACCESS_DATE_RANGE]) {
        QDateTime filemtime = fileInfo->lastRead();
        if (filemtime < filter.accessDateRangeStart || filemtime > filter.accessDateRangeEnd)
            return false;
    }

    if (filter.comboValid[CREATE_DATE_RANGE]) {
        QDateTime filemtime = fileInfo->created();
        if (filemtime < filter.createDateRangeStart || filemtime > filter.createDateRangeEnd)
            return false;
    }

    return true;
}

AdvanceSearchBarPrivate::FileFilter AdvanceSearchBarPrivate::parseFilterData(const QMap<int, QVariant> &data)
{
    FileFilter filter;
    filter.comboValid[SEARCH_RANGE] = true;
    filter.includeSubDir = data[SEARCH_RANGE].toBool();
    filter.typeString = data[FILE_TYPE].toString();
    filter.comboValid[FILE_TYPE] = !filter.typeString.isEmpty();
    filter.comboValid[SIZE_RANGE] = data[SIZE_RANGE].canConvert<QPair<quint64, quint64>>();
    if (filter.comboValid[SIZE_RANGE])
        filter.sizeRange = data[SIZE_RANGE].value<QPair<quint64, quint64>>();

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
                startTime = QDateTime(today);
                endTime = QDateTime(tomorrow);
                break;
            case 2:
                startTime = QDateTime(today).addDays(-1);
                endTime = QDateTime(today);
                break;
            case 7:
                startTime = QDateTime(today).addDays(0 - dayDist);
                endTime = QDateTime(tomorrow);
                break;
            case 14:
                startTime = QDateTime(today).addDays(-7 - dayDist);
                endTime = QDateTime(today).addDays(0 - dayDist);
                break;
            case 30:
                startTime = QDateTime(QDate(today.year(), today.month(), 1));
                endTime = QDateTime(tomorrow);
                break;
            case 60:
                startTime = QDateTime(QDate(today.year(), today.month(), 1)).addMonths(-1);
                endTime = QDateTime(QDate(today.year(), today.month(), 1));
                break;
            case 365:
                startTime = QDateTime(QDate(today.year(), 1, 1));
                endTime = QDateTime(tomorrow);
                break;
            case 730:
                startTime = QDateTime(QDate(today.year(), 1, 1)).addYears(-1);
                endTime = QDateTime(QDate(today.year(), 1, 1));
                break;
            default:
                break;
            }
        }
    };

    calDateFilter(DATE_RANGE, filter.dateRangeStart, filter.dateRangeEnd);
    calDateFilter(ACCESS_DATE_RANGE, filter.accessDateRangeStart, filter.accessDateRangeEnd);
    calDateFilter(CREATE_DATE_RANGE, filter.createDateRangeStart, filter.createDateRangeEnd);

    return filter;
}

AdvanceSearchBar::AdvanceSearchBar(QWidget *parent)
    : QScrollArea(parent),
      d(new AdvanceSearchBarPrivate(this))
{
}

void AdvanceSearchBar::resetForm()
{
    for (int i = 0; i < AdvanceSearchBarPrivate::LABEL_COUNT; ++i) {
        QSignalBlocker blocker(d->asbCombos[i]);
        d->asbCombos[i]->setCurrentIndex(0);
    }
    onOptionChanged();
}

void AdvanceSearchBar::initService()
{
    auto &ctx = dpfInstance.serviceContext();
    GlobalPrivate::winServ = ctx.service<WindowsService>(WindowsService::name());
    Q_ASSERT(GlobalPrivate::winServ);

    GlobalPrivate::workspaceServ = ctx.service<WorkspaceService>(WorkspaceService::name());
    Q_ASSERT(GlobalPrivate::workspaceServ);
}

void AdvanceSearchBar::onOptionChanged()
{
    QMap<int, QVariant> formData;
    formData[AdvanceSearchBarPrivate::SEARCH_RANGE] = d->asbCombos[AdvanceSearchBarPrivate::SEARCH_RANGE]->currentData();
    formData[AdvanceSearchBarPrivate::FILE_TYPE] = d->asbCombos[AdvanceSearchBarPrivate::FILE_TYPE]->currentData();
    formData[AdvanceSearchBarPrivate::SIZE_RANGE] = d->asbCombos[AdvanceSearchBarPrivate::SIZE_RANGE]->currentData();
    formData[AdvanceSearchBarPrivate::DATE_RANGE] = d->asbCombos[AdvanceSearchBarPrivate::DATE_RANGE]->currentData();
    formData[AdvanceSearchBarPrivate::ACCESS_DATE_RANGE] = d->asbCombos[AdvanceSearchBarPrivate::ACCESS_DATE_RANGE]->currentData();
    formData[AdvanceSearchBarPrivate::CREATE_DATE_RANGE] = d->asbCombos[AdvanceSearchBarPrivate::CREATE_DATE_RANGE]->currentData();

    if (!GlobalPrivate::winServ || !GlobalPrivate::workspaceServ)
        initService();

    auto winId = GlobalPrivate::winServ->findWindowId(this);
    auto window = GlobalPrivate::winServ->findWindowById(winId);
    GlobalPrivate::workspaceServ->setFileViewFilterData(winId, window->currentUrl(), QVariant::fromValue(formData));
    GlobalPrivate::workspaceServ->setFileViewFilterCallback(winId, window->currentUrl(), AdvanceSearchBarPrivate::shouldVisiableByFilterRule);
}

void AdvanceSearchBar::onResetButtonPressed()
{
    resetForm();
}

void AdvanceSearchBar::hideEvent(QHideEvent *event)
{
    resetForm();
    QScrollArea::hideEvent(event);
}

DPSEARCH_END_NAMESPACE
