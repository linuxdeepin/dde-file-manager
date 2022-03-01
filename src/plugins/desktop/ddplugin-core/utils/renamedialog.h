/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef RENAMEDIALOG_H
#define RENAMEDIALOG_H

#include "dfm_desktop_service_global.h"
#include "dfm-base/interfaces/abstractjobhandler.h"
#include "dfm_global_defines.h"

#include <QSharedPointer>
#include <DDialog>

DSB_D_BEGIN_NAMESPACE

class RenameDialogPrivate;
class RenameDialog : public Dtk::Widget::DDialog
{
    Q_OBJECT
public:
    enum ModifyMode {
        kReplace = 0,
        kAdd,
        kCustom
    };

    explicit RenameDialog(int fileCount = 0, QWidget *parent = nullptr);
    ~RenameDialog() = default;

    ModifyMode modifyMode() const;
    QPair<QString, QString> getReplaceContent() const;
    QPair<QString, dfmbase::AbstractJobHandler::FileNameAddFlag> getAddContent() const;
    QPair<QString, QString> getCustomContent() const;

private:
    void initUi();

private:
    RenameDialog(const RenameDialog &other) = delete;
    RenameDialog &operator=(const RenameDialog &other) = delete;

private:
    QSharedPointer<RenameDialogPrivate> d;
};

DSB_D_END_NAMESPACE
#endif   // RENAMEDIALOG_H
