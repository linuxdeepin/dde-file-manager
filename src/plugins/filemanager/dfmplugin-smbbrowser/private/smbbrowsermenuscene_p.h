// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SMBBROWSERMENUSCENE_P_H
#define SMBBROWSERMENUSCENE_P_H

#include "dfmplugin_smbbrowser_global.h"

#include <dfm-base/interfaces/private/abstractmenuscene_p.h>

namespace dfmplugin_smbbrowser {

class SmbBrowserMenuScene;
class SmbBrowserMenuScenePrivate : public DFMBASE_NAMESPACE::AbstractMenuScenePrivate
{
    Q_OBJECT
    friend class SmbBrowserMenuScene;

public:
    explicit SmbBrowserMenuScenePrivate(DFMBASE_NAMESPACE::AbstractMenuScene *qq);
    void actUnmount();
    void actMount();
    void actProperties();

private:
    QUrl url;
};

}

#endif   // SMBBROWSERMENUSCENE_P_H
