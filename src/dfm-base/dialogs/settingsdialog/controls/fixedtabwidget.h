// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FIXEDTABWIDGET_H
#define FIXEDTABWIDGET_H

#include <QWidget>

class AliasComboBox;
class QLabel;
class QGridLayout;

namespace Dtk {
namespace Core {
class DSettingsOption;
}
}

class FixedTabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FixedTabWidget(QWidget *parent = nullptr);

    void setOption(QObject *opt);

private:
    void initUI();
    void addCustomItem(Dtk::Core::DSettingsOption *opt, const QUrl &url);

    QUrl selectCustomDirectory();
    void handleAddCustomItem(Dtk::Core::DSettingsOption *opt);
    void handleDefaultChanged(Dtk::Core::DSettingsOption *opt, int index);
    void handleOptionChanged(const QVariant &value);
    void updateAddItemLabel(bool enable);
    void clearCustomItems();
    bool removeRow(QWidget *w);

private:
    AliasComboBox *defItemCB { nullptr };
    QLabel *addItemLabel { nullptr };
    QGridLayout *mainLayout { nullptr };

    int lastDefIndex { -1 };
};

#endif   // FIXEDTABWIDGET_H
