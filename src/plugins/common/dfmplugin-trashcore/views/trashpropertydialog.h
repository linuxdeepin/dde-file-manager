// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHPROPERTYDIALOG_H
#define TRASHPROPERTYDIALOG_H

#include "dfmplugin_trashcore_global.h"
#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>
#include <dfm-base/utils/filestatisticsjob.h>

#include <DDialog>

namespace dfmplugin_trashcore {
class TrashPropertyDialog : public DTK_WIDGET_NAMESPACE::DDialog
{
    Q_OBJECT
public:
    explicit TrashPropertyDialog(QWidget *parent = nullptr);

    virtual ~TrashPropertyDialog() override;

private:
    void initUI();
    void updateLeftInfo(const int &count);
    void calculateSize();
    void updateUI(qint64 size, int count);

protected:
    virtual void showEvent(QShowEvent *event) override;

private:
    DTK_WIDGET_NAMESPACE::DLabel *trashNameLabel { nullptr };
    DTK_WIDGET_NAMESPACE::DLabel *trashIconLabel { nullptr };
    DFMBASE_NAMESPACE::KeyValueLabel *fileCountAndFileSize { nullptr };
};
}
#endif   // TRASHPROPERTYDIALOG_H
