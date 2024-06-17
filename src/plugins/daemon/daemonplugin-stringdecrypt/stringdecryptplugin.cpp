// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stringdecryptplugin.h"
#include "opensslhandler.h"
#include "stringdecryptdbus.h"

Q_DECLARE_METATYPE(QString *)

using namespace daemonplugin_stringdecrypt;

bool StringDecryptPlugin::start()
{
    OpenSSLHandler::instance()->initKeyPairs();
    mng.reset(new StringDecryptDBus(this));
    bindEvents();
    return true;
}

void StringDecryptPlugin::bindEvents()
{
    dpfSlotChannel->connect("daemonplugin_stringdecrypt", "slot_OpenSSL_DecryptString",
                            OpenSSLHandler::instance(), &OpenSSLHandler::decrypt);
}
