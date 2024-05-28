// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shortcut.h"

namespace dfmbase {

Shortcut::Shortcut(QObject *parent)
    : QObject(parent)
{
    ShortcutGroup group1;
    ShortcutGroup group2;
    ShortcutGroup group3;
    ShortcutGroup group4;
    ShortcutGroup group5;
    group1.groupName = tr("Item");
    group1.groupItems << ShortcutItem(tr("Select to the first item"), "Shift + Home ")
                      << ShortcutItem(tr("Select to the last item"), "Shift + End ")
                      << ShortcutItem(tr("Select leftwards"), "Shift + Left ")
                      << ShortcutItem(tr("Select rightwards"), "Shift + Right ")
                      << ShortcutItem(tr("Select to upper row"), "Shift + Up ")
                      << ShortcutItem(tr("Select to lower row"), "Shift + Down ")
                      << ShortcutItem(tr("Reverse selection"), "Ctrl + Shift + I ")
                      << ShortcutItem(tr("Open"), "Ctrl + Down " + QObject::tr("or") +  " Alt + Down ")
                      << ShortcutItem(tr("To parent directory"), "Ctrl + Up ")
                      << ShortcutItem(tr("Permanently delete"), "Shift + Delete ")
                      << ShortcutItem(tr("Delete file"), "Delete " + QObject::tr("or") +  " Ctrl + D ")
                      << ShortcutItem(tr("Select all"), "Ctrl + A ")
                      << ShortcutItem(tr("Copy"), "Ctrl + C ")
                      << ShortcutItem(tr("Cut"), "Ctrl + X ")
                      << ShortcutItem(tr("Paste"), "Ctrl + V ")
                      << ShortcutItem(tr("Rename"), "F2 ")
                      << ShortcutItem(tr("Open in terminal"), "Shift + T ")
                      << ShortcutItem(tr("Undo"), "Ctrl + Z ")
                      << ShortcutItem(tr("Redo"), "Ctrl + Y ");

    group2.groupName = tr("New/Search");
    group2.groupItems << ShortcutItem(tr("New window"), "Ctrl + N ")
                      << ShortcutItem(tr("New folder"), "Ctrl + Shift + N ")
                      << ShortcutItem(tr("Search"), "Ctrl + F")
                      << ShortcutItem(tr("New tab"), "Ctrl + T ");
    group3.groupName = tr("View");
    group3.groupItems << ShortcutItem(tr("Item information"), "Ctrl + I ")
                      << ShortcutItem(tr("Help"), "F1 ")
                      << ShortcutItem(tr("Keyboard shortcuts"), "Ctrl + Shift + / ");
    group4.groupName = tr("Switch display status");
    group4.groupItems << ShortcutItem(tr("Hide item"), "Ctrl + H ")
                      << ShortcutItem(tr("Input in address bar"), "Ctrl + L ")
                      << ShortcutItem(tr("Switch to icon view"), "Ctrl + 1 ")
                      << ShortcutItem(tr("Switch to list view"), "Ctrl + 2 ")
                      << ShortcutItem(tr("Switch to tree view"), "Ctrl + 3 ");
    group5.groupName = tr("Others");
    group5.groupItems << ShortcutItem(tr("Close"), "Alt + F4 ")
                      << ShortcutItem(tr("Close current tab"), "Ctrl + W")
                      << ShortcutItem(tr("Back"), "Alt + Left ")
                      << ShortcutItem(tr("Forward"), "Alt + Right ")
                      << ShortcutItem(tr("Switch to next tab"), "Ctrl + Tab ")
                      << ShortcutItem(tr("Switch to previous tab"), "Ctrl + Shift + Tab ")
                      << ShortcutItem(tr("Next file"), "Tab ")
                      << ShortcutItem(tr("Previous file"), "Shift + Tab ")
                      << ShortcutItem(tr("Switch tab by specified number between 1 to 8"), "Alt + [1-8] ");

    shortcutGroups << group1 << group2 << group3 << group4 << group5;

    //convert to json object
    QJsonArray jsonGroups;
    for (auto scg : shortcutGroups) {
        QJsonObject jsonGroup;
        jsonGroup.insert("groupName", scg.groupName);
        QJsonArray jsonGroupItems;
        for (auto sci : scg.groupItems) {
            QJsonObject jsonItem;
            jsonItem.insert("name", sci.name);
            jsonItem.insert("value", sci.value);
            jsonGroupItems.append(jsonItem);
        }
        jsonGroup.insert("groupItems", jsonGroupItems);
        jsonGroups.append(jsonGroup);
    }
    shortcutObj.insert("shortcut", jsonGroups);
}
QString Shortcut::toStr()
{
    QJsonDocument doc(shortcutObj);
    return doc.toJson().data();
}

}
