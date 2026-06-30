// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef CHECKBOXWITHOCRINDEX_H
#define CHECKBOXWITHOCRINDEX_H

#include "indexstatuscheckbox.h"

namespace dfmplugin_search {

class IndexStatusController;

class CheckBoxWithOcrIndex : public IndexStatusCheckBox
{
    Q_OBJECT

public:
    explicit CheckBoxWithOcrIndex(QWidget *parent = nullptr);
    void connectToBackend();
    void initStatusBar();

private:
    IndexStatusController *m_controller { nullptr };
};

}   // namespace dfmplugin_search
#endif   // CHECKBOXWITHOCRINDEX_H
