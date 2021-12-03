/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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

#ifndef DFMSETTINGDIALOG_H
#define DFMSETTINGDIALOG_H

#include <DSettingsDialog>

class QCheckBox;

DCORE_BEGIN_NAMESPACE
class DSettings;
DCORE_END_NAMESPACE

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class SettingDialog : public DSettingsDialog
{
public:
    explicit SettingDialog(QWidget *parent = nullptr);

private:
    static QPair<QWidget *, QWidget *> createAutoMountCheckBox(QObject *opt);
    static QPair<QWidget *, QWidget *> createAutoMountOpenCheckBox(QObject *opt);
    static void mountCheckBoxStateChangedHandle(DSettingsOption *option, int state);
    static void autoMountCheckBoxChangedHandle(DSettingsOption *option, int state);

private:
    static QPointer<QCheckBox> AutoMountCheckBox;
    static QPointer<QCheckBox> AutoMountOpenCheckBox;
    QPointer<DSettings> m_settings;
};

#endif   // DFMSETTINGDIALOG_H
