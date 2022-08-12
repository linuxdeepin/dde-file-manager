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

#include "dfm-base/dfm_base_global.h"

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

private:
    [[nodiscard]] static QPair<QWidget *, QWidget *> createAutoMountCheckBox(QObject *opt);
    [[nodiscard]] static QPair<QWidget *, QWidget *> createAutoMountOpenCheckBox(QObject *opt);
    [[nodiscard]] static QPair<QWidget *, QWidget *> createSplitter(QObject *opt);
    static void mountCheckBoxStateChangedHandle(DSettingsOption *option, int state);
    static void autoMountCheckBoxChangedHandle(DSettingsOption *option, int state);

private:
    static QPointer<QCheckBox> kAutoMountCheckBox;
    static QPointer<QCheckBox> kAutoMountOpenCheckBox;
    QPointer<DSettings> dtkSettings;
};
}
#endif   // DFMSETTINGDIALOG_H
