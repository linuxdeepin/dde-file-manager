// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CHECKBOXWIDTHSEMANTICINDEX_H
#define CHECKBOXWIDTHSEMANTICINDEX_H

#include "indexstatuscheckbox.h"

namespace dfmplugin_search {

class CheckBoxWidthSemanticIndex : public IndexStatusCheckBox
{
    Q_OBJECT

public:
    explicit CheckBoxWidthSemanticIndex(QWidget *parent = nullptr);

    void initStatusBar();

private:
    void setDisabledByFileIndex(bool fileIndexEnabled);

private:
    bool m_fileIndexEnabled { true };
};

}   // namespace dfmplugin_search

#endif   // CHECKBOXWIDTHSEMANTICINDEX_H
