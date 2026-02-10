// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CUSTOMTABSETTINGWIDGET_H
#define CUSTOMTABSETTINGWIDGET_H

#include "dfmplugin_titlebar_global.h"

#include <DToolButton>

#include <QWidget>

class QLabel;
class QGridLayout;

namespace Dtk {
namespace Core {
class DSettingsOption;
}
}
namespace dfmplugin_titlebar {

class CustomTabSettingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CustomTabSettingWidget(QWidget *parent = nullptr);

    void setOption(QObject *opt);

private:
    void initUI();
    void addCustomItem(Dtk::Core::DSettingsOption *opt, const QUrl &url);

    QUrl selectCustomDirectory();
    void handleAddCustomItem(Dtk::Core::DSettingsOption *opt);
    void handleOptionChanged(const QVariant &value);
    void clearCustomItems();
    bool removeRow(QWidget *w);

private:
    DTK_WIDGET_NAMESPACE::DToolButton *addItemBtn { nullptr };
    QGridLayout *mainLayout { nullptr };
};
}

#endif   // CUSTOMTABSETTINGWIDGET_H
