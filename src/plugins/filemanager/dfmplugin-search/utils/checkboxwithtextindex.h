// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef CHECKBOXWITHTEXTINDEX_H
#define CHECKBOXWITHTEXTINDEX_H

#include "indexstatuscheckbox.h"

namespace dfmplugin_search {

class IndexStatusController;

class CheckBoxWithTextIndex : public IndexStatusCheckBox
{
    Q_OBJECT

public:
    explicit CheckBoxWithTextIndex(QWidget *parent = nullptr);
    void connectToBackend();
    void initStatusBar();

private:
    IndexStatusController *m_controller { nullptr };
};

}   // namespace dfmplugin_search
#endif   // CHECKBOXWITHTEXTINDEX_H
