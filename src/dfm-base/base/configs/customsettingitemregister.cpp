// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/settingdialog/customsettingitemregister.h>
#include <dfm-base/dfm_base_global.h>

#include <QDebug>

using namespace dfmbase;

CustomSettingItemRegister::CustomSettingItemRegister()
{
}

const QMap<QString, CustomSettingItemCreator> &CustomSettingItemRegister::getCreators() const
{
    return creators;
}

CustomSettingItemRegister *CustomSettingItemRegister::instance()
{
    static CustomSettingItemRegister ins;
    return &ins;
}

bool CustomSettingItemRegister::registCustomSettingItemType(const QString &type, const CustomSettingItemCreator &creator)
{
    if (creators.contains(type)) {
        qCWarning(logDFMBase) << type << "is already registered...";
        return false;
    }
    creators.insert(type, creator);
    return true;
}
