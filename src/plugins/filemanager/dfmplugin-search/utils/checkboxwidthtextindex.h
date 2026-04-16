// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef CHECKBOXWIDTHTEXTINDEX_H
#define CHECKBOXWIDTHTEXTINDEX_H

#include "indexstatuscheckbox.h"

namespace dfmplugin_search {

class IndexStatusController;

class CheckBoxWidthTextIndex : public IndexStatusCheckBox
{
    Q_OBJECT

public:
    explicit CheckBoxWidthTextIndex(QWidget *parent = nullptr);
    void connectToBackend();
    void initStatusBar();

private:
    IndexStatusController *m_controller { nullptr };
};

}   // namespace dfmplugin_search
#endif   // CHECKBOXWIDTHTEXTINDEX_H
