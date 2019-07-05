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

#include <QAction>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QTimer>
#include <dboxwidget.h>

DWIDGET_USE_NAMESPACE

DFM_BEGIN_NAMESPACE

DFMAdvanceSearchBar::DFMAdvanceSearchBar(QWidget *parent)
    : DBoxWidget(QBoxLayout::LeftToRight, parent)
{
    initUI();
    initConnection();
}

void DFMAdvanceSearchBar::resetForm()
{
    for (int i = 0; i < LABEL_COUNT; i++) {
        asbCombos[i]->setCurrentIndex(0);
    }
}

void DFMAdvanceSearchBar::onOptionChanged()
{
    QMap<int, QVariant> formData;

    bool searchRangeChanged = formData[SEARCH_RANGE] != asbCombos[SEARCH_RANGE]->currentData();

    formData[SEARCH_RANGE] = asbCombos[SEARCH_RANGE]->currentData();
    formData[FILE_TYPE] = asbCombos[FILE_TYPE]->currentData();
    formData[SIZE_RANGE] = asbCombos[SIZE_RANGE]->currentData();
    formData[DATE_RANGE] = asbCombos[DATE_RANGE]->currentData();

    bool triggerSearch = false;

    if (needSearchAgain) {
        needSearchAgain = false;
        triggerSearch = true;
    }

    if (searchRangeChanged) {
        triggerSearch = true;
    }

    formData[TRIGGER_SEARCH] = QVariant::fromValue(triggerSearch);

    emit optionChanged(formData);
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

    auto createLabelCombo = [ = ](int index, const QString& labelText) {
        asbLabels[index] = new QLabel(labelText);
        asbCombos[index] = new QComboBox(this);
        asbCombos[index]->setFocusPolicy(Qt::NoFocus);
        asbLabels[index]->setBuddy(asbCombos[index]);
    };

    createLabelCombo(SEARCH_RANGE, qApp->translate("DFMAdvanceSearchBar", "Search:"));
    createLabelCombo(FILE_TYPE, qApp->translate("DFMAdvanceSearchBar", "File Type:"));
    createLabelCombo(SIZE_RANGE, qApp->translate("DFMAdvanceSearchBar", "File Size:"));
    createLabelCombo(DATE_RANGE, qApp->translate("DFMAdvanceSearchBar", "Time Modified:"));

    resetBtn = new DLinkButton(qApp->translate("DFMAdvanceSearchBar", "Reset"), this);
    resetBtn->setFocusPolicy(Qt::NoFocus);

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

    asbCombos[SEARCH_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "All subdirectories"), QVariant::fromValue(true));
    asbCombos[SEARCH_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "Current directory"), QVariant::fromValue(false));

    auto addItemToFileTypeCombo = [this](const QString& typeStr) {
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

    asbCombos[SIZE_RANGE]->addItem("--", QVariant());
    asbCombos[SIZE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "0 ~ 100 KB"), QVariant::fromValue(QPair<quint64, quint64>(0, 100)));
    asbCombos[SIZE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "100 KB ~ 1 MB"), QVariant::fromValue(QPair<quint64, quint64>(100, 1024)));
    asbCombos[SIZE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "1 MB ~ 10 MB"), QVariant::fromValue(QPair<quint64, quint64>(1024, 10 * 1024)));
    asbCombos[SIZE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "10 MB ~ 100 MB"), QVariant::fromValue(QPair<quint64, quint64>(10 * 1024, 100 * 1024)));
    asbCombos[SIZE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "100 MB ~ 1 GB"), QVariant::fromValue(QPair<quint64, quint64>(100 * 1024, 1 << 20)));
    asbCombos[SIZE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "> 1 GB"), QVariant::fromValue(QPair<quint64, quint64>(1 << 20, 1 << 30))); // here to 1T

    asbCombos[DATE_RANGE]->addItem("--", QVariant());
    asbCombos[DATE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "Today"), QVariant::fromValue(1));
    asbCombos[DATE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "Yesterday"), QVariant::fromValue(2));
    asbCombos[DATE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "This week"), QVariant::fromValue(7));
    asbCombos[DATE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "Last week"), QVariant::fromValue(14));
    asbCombos[DATE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "This month"), QVariant::fromValue(30));
    asbCombos[DATE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "Last month"), QVariant::fromValue(60));
    asbCombos[DATE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "This year"), QVariant::fromValue(365));
    asbCombos[DATE_RANGE]->addItem(qApp->translate("DFMAdvanceSearchBar", "Last year"), QVariant::fromValue(730));

//    QTimer::singleShot(1000, this, [this] {
//       qDebug() << asbCombos[0]->sizeHint() << asbCombos[0]->size() << asbCombos[0]->minimumSizeHint();
//    });

    formLayout->addWidget(asbLabels[SEARCH_RANGE], 0, 0);
    formLayout->addWidget(asbCombos[SEARCH_RANGE], 0, 1);
    formLayout->addItem(new QSpacerItem(14, 1), 0, 2);
    formLayout->addWidget(asbLabels[FILE_TYPE], 0, 3);
    formLayout->addWidget(asbCombos[FILE_TYPE], 0, 4);
    formLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding), 0, 5);
    formLayout->addWidget(resetBtn, 0, 6);

    formLayout->addWidget(asbLabels[SIZE_RANGE], 1, 0);
    formLayout->addWidget(asbCombos[SIZE_RANGE], 1, 1);
    formLayout->addWidget(asbLabels[DATE_RANGE], 1, 3);
    formLayout->addWidget(asbCombos[DATE_RANGE], 1, 4);

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
    connect(resetBtn, &DLinkButton::pressed, this, &DFMAdvanceSearchBar::onResetButtonPressed);

    for (int i = 0; i < LABEL_COUNT; i++) {
        connect(asbCombos[i], static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DFMAdvanceSearchBar::onOptionChanged);
    }
}

DFM_END_NAMESPACE
