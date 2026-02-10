// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMSETTINGITEMREGISTER_H
#define CUSTOMSETTINGITEMREGISTER_H

#include <QMap>
#include <QPair>

class QWidget;
class QObject;

typedef QPair<QWidget *, QWidget *> (*CustomSettingItemCreator)(QObject *);

namespace dfmbase {
class CustomSettingItemRegister
{
public:
    static CustomSettingItemRegister *instance();
    bool registCustomSettingItemType(const QString &type, const CustomSettingItemCreator &creator);
    const QMap<QString, CustomSettingItemCreator> &getCreators() const;

private:
    CustomSettingItemRegister();
    QMap<QString, CustomSettingItemCreator> creators;
};
}

#endif   // CUSTOMSETTINGITEMREGISTER_H
