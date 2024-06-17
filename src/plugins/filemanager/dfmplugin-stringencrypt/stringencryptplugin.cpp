// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stringencryptplugin.h"
#include "opensslhandler.h"

Q_DECLARE_METATYPE(QString *);

using namespace dfmplugin_stringencrypt;
DFM_LOG_REISGER_CATEGORY(dfmplugin_stringencrypt)

bool StringEncryptPlugin::start()
{
    bindEvents();
    return true;
}

void StringEncryptPlugin::bindEvents()
{
    dpfSlotChannel->connect("dfmplugin_stringencrypt", "slot_OpenSSL_EncryptString",
                            OpenSSLHandler::instance(), &OpenSSLHandler::encrypt);
}
