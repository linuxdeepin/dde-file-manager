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
                      << ShortcutItem(tr("Select all"),tr("Ctrl + a "))
                      << ShortcutItem(tr("Copy"),tr("Ctrl + c "))
                      << ShortcutItem(tr("Cut"),tr("Ctrl + x "))
                      << ShortcutItem(tr("Paste"),tr("Ctrl + v "))
                      << ShortcutItem(tr("Rename"),tr("F2 "));
    group2.groupName = tr("New/Search");
    group2.groupItems << ShortcutItem(tr("New window"),tr("Ctrl + n "))
                      << ShortcutItem(tr("New folder"),tr("Ctrl + Shift + n "))
                      << ShortcutItem(tr("Search"),tr("Ctrl + f"))
                      << ShortcutItem(tr("New tab"),tr("Ctrl + t "));
    group3.groupName = tr("View");
    group3.groupItems << ShortcutItem(tr("Item information"),tr("Ctrl + i "))
                      << ShortcutItem(tr("Help"),tr("F1 "))
                      << ShortcutItem(tr("Keyboard shortcuts"),tr("Ctrl + Shift + / "));
    group4.groupName = tr("Switch display status");
    group4.groupItems << ShortcutItem(tr("Hide item"),tr("Ctrl + h "))
                      << ShortcutItem(tr("Input in address bar"),tr("Ctrl + l "));
    group5.groupName = tr("Others");
    group5.groupItems << ShortcutItem(tr("Close"),tr("Alt + F4 "))
                      << ShortcutItem(tr("Close current tab"), tr("Ctrl + w"))
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
