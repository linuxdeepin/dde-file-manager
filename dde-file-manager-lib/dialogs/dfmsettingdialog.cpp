/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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

#include "dfmsettingdialog.h"
#include "dtkcore_global.h"
#include <DSettingsWidgetFactory>
#include <QCheckBox>
#include <QFrame>
#include <QVBoxLayout>
#include <QDebug>
#include <DSettingsOption>

#include "dfmglobal.h"
#include "app/define.h"
#include "singleton.h"
#include "dfmsetting.h"


DWIDGET_USE_NAMESPACE

QCheckBox* DFMSettingDialog::AutoMountCheckBox = nullptr;
QCheckBox* DFMSettingDialog::AutoMountOpenCheckBox = nullptr;

DFMSettingDialog::DFMSettingDialog(QWidget *parent):
    DSettingsDialog(parent)
{
    widgetFactory()->registerWidget("mountCheckBox", &DFMSettingDialog::createAutoMountCheckBox);
    widgetFactory()->registerWidget("openCheckBox", &DFMSettingDialog::createAutoMountOpenCheckBox);
}

QWidget *DFMSettingDialog::createAutoMountCheckBox(QObject *opt)
{
    auto option = qobject_cast<Dtk::Core::DSettingsOption*>(opt);
    QCheckBox* mountCheckBox = new QCheckBox(QObject::tr("Auto mount"));
    DFMSettingDialog::AutoMountCheckBox = mountCheckBox;

    if (globalSetting->isAutoMount()){
        mountCheckBox->setChecked(true);
    }

    QObject::connect(mountCheckBox,
                     &QCheckBox::stateChanged,
                     [=](int state){
        if (state == 0){
            if (DFMSettingDialog::AutoMountOpenCheckBox){
                DFMSettingDialog::AutoMountOpenCheckBox->setDisabled(true);
                DFMSettingDialog::AutoMountOpenCheckBox->setChecked(false);
            }
            option->setValue(false);
        }else if (state == 2){
            if (DFMSettingDialog::AutoMountOpenCheckBox){
                DFMSettingDialog::AutoMountOpenCheckBox->setDisabled(false);
            }

            option->setValue(true);
        }
    });

    QObject::connect(option, &Core::DSettingsOption::valueChanged, [=](QVariant value){
        mountCheckBox->setChecked(value.toBool());
    });

    return DSettingsWidgetFactory::createTwoColumWidget(option, mountCheckBox);
}

QWidget *DFMSettingDialog::createAutoMountOpenCheckBox(QObject *opt)
{
    auto option = qobject_cast<Dtk::Core::DSettingsOption*>(opt);
    QCheckBox* openCheckBox = new QCheckBox(QObject::tr("Open after auto mount"));
    DFMSettingDialog::AutoMountOpenCheckBox = openCheckBox;

    if (globalSetting->isAutoMountAndOpen()){
        openCheckBox->setChecked(true);
        openCheckBox->setDisabled(false);
    }else{
        if (!globalSetting->isAutoMount())
            openCheckBox->setDisabled(true);
    }

    QObject::connect(openCheckBox,
                     &QCheckBox::stateChanged,
                     [=](int state){
        if (state == 0){
            option->setValue(false);
        }else if (state == 2){
            option->setValue(true);
        }
    });

    QObject::connect(option, &Core::DSettingsOption::valueChanged, [=](QVariant value){
        openCheckBox->setChecked(value.toBool());
    });

    return DSettingsWidgetFactory::createTwoColumWidget(option, openCheckBox);
}
