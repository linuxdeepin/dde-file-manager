/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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

#ifndef TRASHPROPERTYDIALOG_H
#define TRASHPROPERTYDIALOG_H

#include "dfm_common_service_global.h"
#include "dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h"

#include <DDialog>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
DSC_BEGIN_NAMESPACE
class TrashPropertyDialog : public DDialog
{
public:
    explicit TrashPropertyDialog(QWidget *parent = nullptr);

private:
    void initUI();

private:
    DLabel *trashNameLabel { nullptr };
    DLabel *trashIconLabel { nullptr };
    KeyValueLabel *fileCountAndFileSize { nullptr };
};
DSC_END_NAMESPACE
#endif   // TRASHPROPERTYDIALOG_H
