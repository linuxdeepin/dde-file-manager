// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PREVIEWHELPER_H
#define PREVIEWHELPER_H

#include "dfm-base/utils/clipboard.h"

namespace dfmplugin_filepreview{
class PreviewHelper : public QObject
{
    Q_DISABLE_COPY(PreviewHelper)
public:
    static PreviewHelper *instance();

    bool isPreviewEnabled();

private:
    explicit PreviewHelper(QObject *parent = nullptr);
};
}

#endif   // PREVIEWHELPER_H
