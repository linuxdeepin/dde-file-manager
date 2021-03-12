/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "shortcut.h"

Shortcut::Shortcut(QObject *parent) : QObject(parent)
{
    ShortcutGroup group1;
    ShortcutGroup group2;
    ShortcutGroup group3;
    ShortcutGroup group4;
    ShortcutGroup group5;
    group1.groupName = tr("Item");
    group1.groupItems << ShortcutItem(tr("Select to the first item"),tr("Shift + Home "))
                      << ShortcutItem(tr("Select to the last item"),tr("Shift + End "))
                      << ShortcutItem(tr("Select leftwards"),tr("Shift + Left "))
                      << ShortcutItem(tr("Select rightwards"),tr("Shift + Right "))
                      << ShortcutItem(tr("Select to upper row"),tr("Shift + Up "))
                      << ShortcutItem(tr("Select to lower row"),tr("Shift + Down "))
                      << ShortcutItem(tr("Open"),tr("Ctrl + Down "))
                      << ShortcutItem(tr("To parent directory"),tr("Ctrl + Up "))
                      << ShortcutItem(tr("Permanently delete"),tr("Shift + Delete "))
                      << ShortcutItem(tr("Delete file"),tr("Delete"))
                      << ShortcutItem(tr("Select all"),tr("Ctrl + A "))
                      << ShortcutItem(tr("Copy"),tr("Ctrl + C "))
                      << ShortcutItem(tr("Cut"),tr("Ctrl + X "))
                      << ShortcutItem(tr("Paste"),tr("Ctrl + V "))
                      << ShortcutItem(tr("Rename"),tr("F2 "));
    group2.groupName = tr("New/Search");
    group2.groupItems << ShortcutItem(tr("New window"),tr("Ctrl + N "))
                      << ShortcutItem(tr("New folder"),tr("Ctrl + Shift + N "))
                      << ShortcutItem(tr("Search"),tr("Ctrl + F"))
                      << ShortcutItem(tr("New tab"),tr("Ctrl + T "));
    group3.groupName = tr("View");
    group3.groupItems << ShortcutItem(tr("Item information"),tr("Ctrl + I "))
                      << ShortcutItem(tr("Help"),tr("F1 "))
                      << ShortcutItem(tr("Keyboard shortcuts"),tr("Ctrl + Shift + / "));
    group4.groupName = tr("Switch display status");
    group4.groupItems << ShortcutItem(tr("Hide item"),tr("Ctrl + H "))
                      << ShortcutItem(tr("Input in address bar"),tr("Ctrl + L "))
                      << ShortcutItem(tr("Switch to icon view"),tr("Ctrl + 1 "))
                      << ShortcutItem(tr("Switch to list view"),tr("Ctrl + 2 "));
    group5.groupName = tr("Others");
    group5.groupItems << ShortcutItem(tr("Close"),tr("Alt + F4 "))
                      << ShortcutItem(tr("Close current tab"), tr("Ctrl + W"))
                      << ShortcutItem(tr("Back"),tr("Alt + Left "))
                      << ShortcutItem(tr("Forward"),tr("Alt + Right "))
                      << ShortcutItem(tr("Switch to next tab"),tr("Ctrl + Tab "))
                      << ShortcutItem(tr("Switch to previous tab"),tr("Ctrl + Shift + Tab "))
                      << ShortcutItem(tr("Next file"),tr("Tab "))
                      << ShortcutItem(tr("Previous file"),tr("Shift + Tab "))
                      << ShortcutItem(tr("Switch tab by specified number between 1 to 8"),tr("Alt + [1-8] "));

    m_shortcutGroups << group1 << group2 << group3 << group4 << group5;

    //convert to json object
    QJsonArray jsonGroups;
    for(auto scg:m_shortcutGroups){
        QJsonObject jsonGroup;
        jsonGroup.insert("groupName",scg.groupName);
        QJsonArray jsonGroupItems;
        for(auto sci:scg.groupItems){
            QJsonObject jsonItem;
            jsonItem.insert("name",sci.name);
            jsonItem.insert("value",sci.value);
            jsonGroupItems.append(jsonItem);
        }
        jsonGroup.insert("groupItems",jsonGroupItems);
        jsonGroups.append(jsonGroup);
    }
    m_shortcutObj.insert("shortcut",jsonGroups);
}
QString Shortcut::toStr(){
    QJsonDocument doc(m_shortcutObj);
    return doc.toJson().data();
}
