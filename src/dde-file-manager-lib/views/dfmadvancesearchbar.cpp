/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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

#include "dfmadvancesearchbar.h"
#include "dfilesystemmodel.h"

#include <QAction>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QTimer>
#include <QCommandLinkButton>

#include <dboxwidget.h>

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

DFMAdvanceSearchBar::DFMAdvanceSearchBar(QWidget *parent)
    : DBoxWidget(QBoxLayout::LeftToRight, parent)
{
    initUI();
    initConnection();
}

void DFMAdvanceSearchBar::resetForm(bool updateView)
{
    allowUpdateView = updateView;
    for (int i = 0; i < LABEL_COUNT; i++) {
        asbCombos[i]->setCurrentIndex(0);
    }
    allowUpdateView = true;
}

void DFMAdvanceSearchBar::updateFilterValue(const FileFilter *filter)
{
    // 防止触发信号
    blockSignals(true);
    if (filter) {
        // 搜索范围
        const auto &searchRange = filter->filterRule[SEARCH_RANGE];
        asbCombos[SEARCH_RANGE]->setCurrentIndex(searchRange.toBool() ? 0 : 1);

        // 文件类型
        const auto &fileType = filter->filterRule[FILE_TYPE];
        if (fileType.isValid()) {
            asbCombos[FILE_TYPE]->setCurrentText(fileType.toString());
        } else {
            asbCombos[FILE_TYPE]->setCurrentIndex(0);
        }

        // 文件大小
        const auto &sizeRange = filter->filterRule[SIZE_RANGE];
        if (sizeRange.isValid() && filter->f_comboValid[SIZE_RANGE]) {
            asbCombos[SIZE_RANGE]->setCurrentIndex(sizeRangeMap[filter->f_sizeRange]);
        } else {
            asbCombos[SIZE_RANGE]->setCurrentIndex(0);
        }

        // 修改时间
        const auto &dateRange = filter->filterRule[DATE_RANGE];
        if (dateRange.isValid()) {
            asbCombos[DATE_RANGE]->setCurrentIndex(dateRangeMap[dateRange]);
        } else {
            asbCombos[DATE_RANGE]->setCurrentIndex(0);
        }

        // 访问时间
        const auto &accessDateRange = filter->filterRule[ACCESS_DATE_RANGE];
        if (accessDateRange.isValid()) {
            asbCombos[ACCESS_DATE_RANGE]->setCurrentIndex(dateRangeMap[accessDateRange]);
        } else {
            asbCombos[ACCESS_DATE_RANGE]->setCurrentIndex(0);
        }

        // 创建时间
        const auto &createDateRange = filter->filterRule[CREATE_DATE_RANGE];
        if (createDateRange.isValid()) {
            asbCombos[CREATE_DATE_RANGE]->setCurrentIndex(dateRangeMap[createDateRange]);
        } else {
            asbCombos[CREATE_DATE_RANGE]->setCurrentIndex(0);
        }

    } else {
        resetForm();
    }
    blockSignals(false);
}

void DFMAdvanceSearchBar::onOptionChanged()
{
    QMap<int, QVariant> formData;

    bool searchRangeChanged = formData[SEARCH_RANGE] != asbCombos[SEARCH_RANGE]->currentData();

    formData[SEARCH_RANGE] = asbCombos[SEARCH_RANGE]->currentData();
    formData[FILE_TYPE] = asbCombos[FILE_TYPE]->currentData();
    formData[SIZE_RANGE] = asbCombos[SIZE_RANGE]->currentData();
    formData[DATE_RANGE] = asbCombos[DATE_RANGE]->currentData();
    formData[ACCESS_DATE_RANGE] = asbCombos[ACCESS_DATE_RANGE]->currentData();
    formData[CREATE_DATE_RANGE] = asbCombos[CREATE_DATE_RANGE]->currentData();

    bool triggerSearch = false;

    if (needSearchAgain) {
        needSearchAgain = false;
        triggerSearch = true;
    }

    if (searchRangeChanged) {
        triggerSearch = true;
    }

    formData[TRIGGER_SEARCH] = QVariant::fromValue(triggerSearch);

    emit optionChanged(formData, allowUpdateView);
}

void DFMAdvanceSearchBar::onResetButtonPressed()
{
    resetForm();
}

void DFMAdvanceSearchBar::hideEvent(QHideEvent *)
{
    needSearchAgain = true;
}

void DFMAdvanceSearchBar::initUI()
{
    mainLayout = this->layout();

    const int LEFT_COMBO_MIN_WIDTH = 140;
    const int RIGHT_COMBO_MIN_WIDTH = 100;

    QGridLayout *formLayout = new QGridLayout;

    auto createLabelCombo = [ = ](int index, const QString & labelText) {
        asbLabels[index] = new QLabel(labelText);
        asbCombos[index] = new QComboBox(this);
#ifndef __arm__
        /*针对ARM部分机器上搜索高级选项下拉框不能使用做特殊处理*/
        asbCombos[index]->setFocusPolicy(Qt::NoFocus);
#endif
        asbLabels[index]->setBuddy(asbCombos[index]);
    };

    createLabelCombo(SEARCH_RANGE, qApp->translate("DFMAdvanceSearchBar", "Search:"));
    createLabelCombo(FILE_TYPE, qApp->translate("DFMAdvanceSearchBar", "File Type:"));
    createLabelCombo(SIZE_RANGE, qApp->translate("DFMAdvanceSearchBar", "File Size:"));
    createLabelCombo(DATE_RANGE, qApp->translate("DFMAdvanceSearchBar", "Time Modified:"));
    createLabelCombo(ACCESS_DATE_RANGE, qApp->translate("DFMAdvanceSearchBar", "Time Accessed:"));
    createLabelCombo(CREATE_DATE_RANGE, qApp->translate("DFMAdvanceSearchBar", "Time Created:"));

    resetBtn = new DCommandLinkButton(qApp->translate("DFMAdvanceSearchBar", "Reset"), this);
#ifndef __arm__
    /*针对ARM部分机器上搜索高级选项下拉框不能使用做特殊处理*/
    resetBtn->setFocusPolicy(Qt::NoFocus);
#endif

    int leftLebelWidth = 80; //  qMin(asbLabels[SEARCH_RANGE]->width(), asbLabels[SIZE_RANGE]->width())
    int rightLebelWidth = 90;
    asbLabels[SEARCH_RANGE]->setMinimumWidth(leftLebelWidth);
    asbCombos[SEARCH_RANGE]->setMinimumWidth(LEFT_COMBO_MIN_WIDTH);
    asbLabels[SIZE_RANGE]->setMinimumWidth(leftLebelWidth);
    asbCombos[SIZE_RANGE]->setMinimumWidth(LEFT_COMBO_MIN_WIDTH);
    asbLabels[FILE_TYPE]->setMinimumWidth(rightLebelWidth);
    asbCombos[FILE_TYPE]->setMinimumWidth(RIGHT_COMBO_MIN_WIDTH);
    asbLabels[DATE_RANGE]->setMinimumWidth(rightLebelWidth);
    asbCombos[DATE_RANGE]->setMinimumWidth(RIGHT_COMBO_MIN_WIDTH);
    asbCombos[ACCESS_DATE_RANGE]->setMinimumWidth(RIGHT_COMBO_MIN_WIDTH);
    asbCombos[CREATE_DATE_RANGE]->setMinimumWidth(RIGHT_COMBO_MIN_WIDTH);

    asbCombos[SEARCH_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "All subdirectories"), QVariant::fromValue(true));
    asbCombos[SEARCH_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "Current directory"), QVariant::fromValue(false));

    auto addItemToFileTypeCombo = [this](const QString & typeStr) {
        asbCombos[FILE_TYPE]->addItem(typeStr, QVariant::fromValue(typeStr));
    };

    asbCombos[FILE_TYPE]->addItem("--", QVariant());
    addItemToFileTypeCombo(qApp->translate("MimeTypeDisplayManager", "Application"));
    addItemToFileTypeCombo(qApp->translate("MimeTypeDisplayManager", "Video"));
    addItemToFileTypeCombo(qApp->translate("MimeTypeDisplayManager", "Audio"));
    addItemToFileTypeCombo(qApp->translate("MimeTypeDisplayManager", "Image"));
    addItemToFileTypeCombo(qApp->translate("MimeTypeDisplayManager", "Archive"));
    addItemToFileTypeCombo(qApp->translate("MimeTypeDisplayManager", "Text"));
    addItemToFileTypeCombo(qApp->translate("MimeTypeDisplayManager", "Executable"));
    addItemToFileTypeCombo(qApp->translate("MimeTypeDisplayManager", "Backup file"));

    sizeRangeMap[QPair<quint64, quint64>(0, 100)] = 1;
    sizeRangeMap[QPair<quint64, quint64>(100, 1024)] = 2;
    sizeRangeMap[QPair<quint64, quint64>(1024, 10 * 1024)] = 3;
    sizeRangeMap[QPair<quint64, quint64>(10 * 1024, 100 * 1024)] = 4;
    sizeRangeMap[QPair<quint64, quint64>(100 * 1024, 1 << 20)] = 5;
    sizeRangeMap[QPair<quint64, quint64>(1 << 20, 1 << 30)] = 6;

    asbCombos[SIZE_RANGE]->addItem("--", QVariant());
    asbCombos[SIZE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "0 ~ 100 KB"), QVariant::fromValue(QPair<quint64, quint64>(0, 100)));
    asbCombos[SIZE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "100 KB ~ 1 MB"), QVariant::fromValue(QPair<quint64, quint64>(100, 1024)));
    asbCombos[SIZE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "1 MB ~ 10 MB"), QVariant::fromValue(QPair<quint64, quint64>(1024, 10 * 1024)));
    asbCombos[SIZE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "10 MB ~ 100 MB"), QVariant::fromValue(QPair<quint64, quint64>(10 * 1024, 100 * 1024)));
    asbCombos[SIZE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "100 MB ~ 1 GB"), QVariant::fromValue(QPair<quint64, quint64>(100 * 1024, 1 << 20)));
    asbCombos[SIZE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "> 1 GB"), QVariant::fromValue(QPair<quint64, quint64>(1 << 20, 1 << 30))); // here to 1T

    dateRangeMap[QVariant::fromValue(1)] = 1;
    dateRangeMap[QVariant::fromValue(2)] = 2;
    dateRangeMap[QVariant::fromValue(7)] = 3;
    dateRangeMap[QVariant::fromValue(14)] = 4;
    dateRangeMap[QVariant::fromValue(30)] = 5;
    dateRangeMap[QVariant::fromValue(60)] = 6;
    dateRangeMap[QVariant::fromValue(365)] = 7;
    dateRangeMap[QVariant::fromValue(730)] = 8;

    auto createDateCombos = [ = ](const LabelIndex index) {
        asbCombos[index]->addItem("--", QVariant());
        asbCombos[index]->addItem(qApp->translate("DFMAdvanceSearchBar", "Today"), QVariant::fromValue(1));
        asbCombos[index]->addItem(qApp->translate("DFMAdvanceSearchBar", "Yesterday"), QVariant::fromValue(2));
        asbCombos[index]->addItem(qApp->translate("DFMAdvanceSearchBar", "This week"), QVariant::fromValue(7));
        asbCombos[index]->addItem(qApp->translate("DFMAdvanceSearchBar", "Last week"), QVariant::fromValue(14));
        asbCombos[index]->addItem(qApp->translate("DFMAdvanceSearchBar", "This month"), QVariant::fromValue(30));
        asbCombos[index]->addItem(qApp->translate("DFMAdvanceSearchBar", "Last month"), QVariant::fromValue(60));
        asbCombos[index]->addItem(qApp->translate("DFMAdvanceSearchBar", "This year"), QVariant::fromValue(365));
        asbCombos[index]->addItem(qApp->translate("DFMAdvanceSearchBar", "Last year"), QVariant::fromValue(730));
    };
    createDateCombos(DATE_RANGE);
    createDateCombos(ACCESS_DATE_RANGE);
    createDateCombos(CREATE_DATE_RANGE);


//    QTimer::singleShot(1000, this, [this] {
//       qDebug() << asbCombos[0]->sizeHint() << asbCombos[0]->size() << asbCombos[0]->minimumSizeHint();
//    });

    formLayout->addWidget(asbLabels[SEARCH_RANGE], 0, 0);
    formLayout->addWidget(asbCombos[SEARCH_RANGE], 0, 1);
    formLayout->addItem(new QSpacerItem(14, 1), 0, 2);
    formLayout->addWidget(asbLabels[FILE_TYPE], 0, 3);
    formLayout->addWidget(asbCombos[FILE_TYPE], 0, 4);
    formLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding), 0, 7);
    formLayout->addWidget(resetBtn, 0, 8);

    formLayout->addWidget(asbLabels[SIZE_RANGE], 1, 0);
    formLayout->addWidget(asbCombos[SIZE_RANGE], 1, 1);
    formLayout->addWidget(asbLabels[DATE_RANGE], 1, 3);
    formLayout->addWidget(asbCombos[DATE_RANGE], 1, 4);
    formLayout->addWidget(asbLabels[ACCESS_DATE_RANGE], 0, 5);
    formLayout->addWidget(asbCombos[ACCESS_DATE_RANGE], 0, 6);
    formLayout->addWidget(asbLabels[CREATE_DATE_RANGE], 1, 5);
    formLayout->addWidget(asbCombos[CREATE_DATE_RANGE], 1, 6);


    formLayout->setSpacing(6);
    formLayout->setMargin(6);
//    formLayout->setRowMinimumHeight(0, 36);
//    formLayout->setRowMinimumHeight(1, 36);

    mainLayout->addSpacing(20);
    mainLayout->addLayout(formLayout);
    mainLayout->addSpacing(20);
}

void DFMAdvanceSearchBar::initConnection()
{
    connect(resetBtn, &QCommandLinkButton::pressed, this, &DFMAdvanceSearchBar::onResetButtonPressed);

    for (int i = 0; i < LABEL_COUNT; i++) {
        connect(asbCombos[i], static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DFMAdvanceSearchBar::onOptionChanged);
    }
}

DFM_END_NAMESPACE
