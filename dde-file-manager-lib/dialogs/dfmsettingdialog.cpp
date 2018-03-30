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
                     option,
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

    QObject::connect(option, &Core::DSettingsOption::valueChanged, mountCheckBox, [=](QVariant value){
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
                     option,
                     [=](int state){
        if (state == 0){
            option->setValue(false);
        }else if (state == 2){
            option->setValue(true);
        }
    });

    QObject::connect(option, &Core::DSettingsOption::valueChanged, openCheckBox, [=](QVariant value){
        openCheckBox->setChecked(value.toBool());
    });

    return DSettingsWidgetFactory::createTwoColumWidget(option, openCheckBox);
}
