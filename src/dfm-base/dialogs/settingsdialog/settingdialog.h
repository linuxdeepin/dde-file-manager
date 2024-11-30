// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFMSETTINGDIALOG_H
#define DFMSETTINGDIALOG_H

#include <dfm-base/dfm_base_global.h>

#include <DSettingsDialog>

class QCheckBox;

DCORE_BEGIN_NAMESPACE
class DSettings;
DCORE_END_NAMESPACE

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

namespace dfmbase {
class SettingDialog : public DSettingsDialog
{
public:
    explicit SettingDialog(QWidget *parent = nullptr);
    void initialze();
    static void setItemVisiable(const QString &key, bool visiable);

private:
    [[nodiscard]] static QPair<QWidget *, QWidget *> createAutoMountCheckBox(QObject *opt);
    [[nodiscard]] static QPair<QWidget *, QWidget *> createAutoMountOpenCheckBox(QObject *opt);
    [[nodiscard]] static QPair<QWidget *, QWidget *> createCheckBoxWithMessage(QObject *opt);
    [[nodiscard]] static QPair<QWidget *, QWidget *> createPushButton(QObject *opt);

    static void mountCheckBoxStateChangedHandle(DSettingsOption *option, int state);
    static void autoMountCheckBoxChangedHandle(DSettingsOption *option, int state);

    void settingFilter(QByteArray &data);
    void loadSettings(const QString &templateFile);
    static bool needHide(const QString &key);

private:
    static QPointer<QCheckBox> kAutoMountCheckBox;
    static QPointer<QCheckBox> kAutoMountOpenCheckBox;
    static QSet<QString> kHiddenSettingItems;
    QPointer<DSettings> dtkSettings;
    static quint64 parentWid;
};
}
#endif   // DFMSETTINGDIALOG_H
