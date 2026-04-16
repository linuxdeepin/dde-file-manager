// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef CHECKBOXWIDTHOCRINDEX_H
#define CHECKBOXWIDTHOCRINDEX_H

#include "indexstatuscheckbox.h"

namespace dfmplugin_search {

class IndexStatusController;

class CheckBoxWidthOcrIndex : public IndexStatusCheckBox
{
    Q_OBJECT

public:
    explicit CheckBoxWidthOcrIndex(QWidget *parent = nullptr);
    void connectToBackend();
    void initStatusBar();

private:
    IndexStatusController *m_controller { nullptr };
};

}   // namespace dfmplugin_search
#endif   // CHECKBOXWIDTHOCRINDEX_H
