// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTINDEXDBUS_P_H
#define TEXTINDEXDBUS_P_H

#include "service_textindex_global.h"
#include "textindexadaptor.h"

class TextIndexDBus;

SERVICETEXTINDEX_BEGIN_NAMESPACE

class TextIndexDBusPrivate
{
    friend class ::TextIndexDBus;

public:
    explicit TextIndexDBusPrivate(TextIndexDBus *qq)
        : q(qq), adapter(new TextIndexAdaptor(qq)) { }
    ~TextIndexDBusPrivate() { }

private:
    TextIndexDBus *q { nullptr };
    TextIndexAdaptor *adapter { nullptr };
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // TEXTINDEXDBUS_P_H 