// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHECKBOXWITHFILEINDEX_H
#define CHECKBOXWITHFILEINDEX_H

#include "indexstatuscheckbox.h"

namespace dfmplugin_search {

class IndexStatusController;

class CheckBoxWithFileIndex : public IndexStatusCheckBox
{
    Q_OBJECT

public:
    explicit CheckBoxWithFileIndex(QWidget *parent = nullptr);
    void connectToBackend();
    void initStatusBar();

protected:
    bool acceptCheckStateChange(Qt::CheckState oldState, Qt::CheckState newState) override;

private:
    IndexStatusController *m_controller { nullptr };
    bool confirmDisableFileIndex();
};

}   // namespace dfmplugin_search
#endif   // CHECKBOXWITHFILEINDEX_H
