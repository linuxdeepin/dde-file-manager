// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHECKBOXWITHSEMANTICINDEX_H
#define CHECKBOXWITHSEMANTICINDEX_H

#include "indexstatuscheckbox.h"

namespace dfmplugin_search {

class CheckBoxWithSemanticIndex : public IndexStatusCheckBox
{
    Q_OBJECT

public:
    explicit CheckBoxWithSemanticIndex(QWidget *parent = nullptr);

    void initStatusBar();

private:
    void setDisabledByFileIndex(bool fileIndexEnabled);

private:
    bool m_fileIndexEnabled { true };
};

}   // namespace dfmplugin_search

#endif   // CHECKBOXWITHSEMANTICINDEX_H
