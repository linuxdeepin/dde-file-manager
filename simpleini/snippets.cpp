/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// File:    snippets.cpp
// Library: SimpleIni
// Author:  Brodie Thiesfield <code@jellycan.com>
// Source:  http://code.jellycan.com/simpleini/
//
// Snippets that are used on the website

#ifdef _WIN32
# pragma warning(disable: 4786)
#endif

#ifndef _WIN32
# include <unistd.h>
#endif
#include <fstream>

#define SI_SUPPORT_IOSTREAMS
#include "SimpleIni.h"

bool
snippets(
    const char *    a_pszFile,
    bool            a_bIsUtf8,
    bool            a_bUseMultiKey,
    bool            a_bUseMultiLine
    )
{
    // LOADING DATA

    // load from a data file
    CSimpleIniA ini(a_bIsUtf8, a_bUseMultiKey, a_bUseMultiLine);
    SI_Error rc = ini.LoadFile(a_pszFile);
    if (rc < 0) return false;

    // load from a string
    std::string strData;
    rc = ini.LoadData(strData.c_str(), strData.size());
    if (rc < 0) return false;

    // GETTING SECTIONS AND KEYS

    // get all sections
    CSimpleIniA::TNamesDepend sections;
    ini.GetAllSections(sections);

    // get all keys in a section
    CSimpleIniA::TNamesDepend keys;
    ini.GetAllKeys("section-name", keys);

    // GETTING VALUES

    // get the value of a key
    const char * pszValue = ini.GetValue("section-name",
        "key-name", NULL /*default*/);

    // get the value of a key which may have multiple
    // values. If bHasMultipleValues is true, then just
    // one value has been returned
    bool bHasMultipleValues;
    pszValue = ini.GetValue("section-name", "key-name",
        NULL /*default*/, &bHasMultipleValues);

    // get all values of a key with multiple values
    CSimpleIniA::TNamesDepend values;
    ini.GetAllValues("section-name", "key-name", values);

    // sort the values into the original load order
#if defined(_MSC_VER) && _MSC_VER <= 1200
    /** STL of VC6 doesn't allow me to specify my own comparator for list::sort() */
    values.sort();
#else
    values.sort(CSimpleIniA::Entry::LoadOrder());
#endif

    // output all of the items
    CSimpleIniA::TNamesDepend::const_iterator i;
    for (i = values.begin(); i != values.end(); ++i) {
        printf("key-name = '%s'\n", i->pItem);
    }

    // MODIFYING DATA

    // adding a new section
    rc = ini.SetValue("new-section", NULL, NULL);
    if (rc < 0) return false;
    printf("section: %s\n", rc == SI_INSERTED ?
        "inserted" : "updated");

    // adding a new key ("new-section" will be added
    // automatically if it doesn't already exist.
    rc = ini.SetValue("new-section", "new-key", "value");
    if (rc < 0) return false;
    printf("key: %s\n", rc == SI_INSERTED ?
        "inserted" : "updated");

    // changing the value of a key
    rc = ini.SetValue("section", "key", "updated-value");
    if (rc < 0) return false;
    printf("key: %s\n", rc == SI_INSERTED ?
        "inserted" : "updated");

    // DELETING DATA

    // deleting a key with a value from a section.
    // Optionally the entire section may be deleted if
    // it is now empty.
    ini.DeleteValue("section-name", "key-name", "value",
        true /*delete the section if empty*/);

    // deleting a key with any value from a section.
    ini.Delete("section-name", "key-name",
        true /*delete the section if empty*/);

    // deleting an entire section and all keys in it
    ini.Delete("section-name", NULL);

    // SAVING DATA

    // save the data to a string
    rc = ini.Save(strData);
    if (rc < 0) return false;

    // save the data back to the file
    rc = ini.SaveFile(a_pszFile);
    if (rc < 0) return false;

    return true;
}
