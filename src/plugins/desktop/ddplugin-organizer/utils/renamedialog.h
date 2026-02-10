// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RENAMEDIALOG_H
#define RENAMEDIALOG_H

#include "ddplugin_organizer_global.h"

#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

#include <QSharedPointer>
#include <DDialog>

namespace ddplugin_organizer {

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
    QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> getAddContent() const;
    QPair<QString, QString> getCustomContent() const;

private:
    void initUi();

private:
    RenameDialog(const RenameDialog &other) = delete;
    RenameDialog &operator=(const RenameDialog &other) = delete;

private:
    QSharedPointer<RenameDialogPrivate> d;
};

}
#endif   // RENAMEDIALOG_H
