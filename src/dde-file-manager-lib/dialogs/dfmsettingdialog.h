// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMSETTINGDIALOG_H
#define DFMSETTINGDIALOG_H


#include <DSettingsDialog>
#include <QCheckBox>

DWIDGET_USE_NAMESPACE

DCORE_BEGIN_NAMESPACE
class DSettings;
DCORE_END_NAMESPACE

DCORE_USE_NAMESPACE

class DFMSettingDialog : public DSettingsDialog
{
public:
    explicit DFMSettingDialog(QWidget *parent = nullptr);

    static QPair<QWidget*, QWidget*> createAutoMountCheckBox(QObject* opt);
    static QPair<QWidget *, QWidget *> createAutoMountOpenCheckBox(QObject* opt);

    static QPointer<QCheckBox> AutoMountCheckBox;
    static QPointer<QCheckBox> AutoMountOpenCheckBox;

private:
    QPointer<DSettings> m_settings;
};

#endif // DFMSETTINGDIALOG_H
