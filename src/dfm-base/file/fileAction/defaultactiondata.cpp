/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
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
#include "private/defaultactiondata_p.h"

#include "dfm-base/utils/actiontypemanager.h"

DFMBASE_USE_NAMESPACE

DefaultActionDataPrivate::DefaultActionDataPrivate(DefaultActionData *qq)
    : q(qq)
{
}

DefaultActionDataPrivate::~DefaultActionDataPrivate()
{
    allActionTypeToData.clear();
    allActionPredicateToText.clear();
    allActionPredicateToTypes.clear();
    actionTypeToEventType.clear();
}

DefaultActionData::DefaultActionData(QObject *parent)
    : QObject(parent), d(new DefaultActionDataPrivate(this))
{
    this->initDefaultActionData();
    this->initDefaultActionEvent();
}

DefaultActionData::~DefaultActionData()
{
}

/*!
 * \brief DefaultActionData::actionDataContainerByType: get the default action data by action type.
 * \param actType: type of target action.
 * \param defaultAct: returns the default value when the fetch fails.
 * \return action data of target type
 */
dfmbase::ActionDataContainer DefaultActionData::actionDataContainerByType(const int actType,
                                                                          const dfmbase::ActionDataContainer &defaultAct)
{
    return d->allActionTypeToData.value(actType, defaultAct);
}

bool DefaultActionData::addSubActionType(ActionType parentType, const ActionDataContainer &actionData)
{
    if (d->allActionTypeToData.count(parentType)) {
        d->allActionTypeToData[parentType].addChildrenActionsData(actionData);
        return true;
    }
    return false;
}

bool DefaultActionData::contains(ActionType type)
{
    return d->allActionTypeToData.contains(type);
}

void DefaultActionData::initDefaultActionData()
{
    // regist default actionType

    this->addDefaultActionType(kActOpen, "Open", tr("Open"));
    this->addDefaultActionType(kActNewFolder, "NewFolder", tr("New folder"));

    this->addDefaultActionType(kActNewDocument, "NewDocument", tr("New document"));
    addSubActionType(kActNewDocument, addDefaultActionType(kActNewWord, "NewWord", tr("Office Text")));
    addSubActionType(kActNewDocument, addDefaultActionType(kActNewExcel, "NewExcel", tr("Spreadsheets")));
    addSubActionType(kActNewDocument, addDefaultActionType(kActNewPowerpoint, "NewPowerpoint", tr("Presentation")));
    addSubActionType(kActNewDocument, addDefaultActionType(kActNewText, "NewText", tr("Plain Text")));

    this->addDefaultActionType(kActOpenInNewWindow, "OpenInNewWindow", tr("Open in new window"));
    this->addDefaultActionType(kActOpenInNewTab, "OpenInNewTab", tr("Open in new tab"));
    this->addDefaultActionType(kActDisplayAs, "DisplayAs", tr("Display as"));
    this->addDefaultActionType(kActSortBy, "SortBy", tr("Sort by"));
    this->addDefaultActionType(kActOpenWith, "OpenWith", tr("Open with"));
    this->addDefaultActionType(kActOpenAsAdmin, "OpenAsAdmin", tr("Open as administrator"));
    this->addDefaultActionType(kActOpenInTerminal, "OpenInTerminal", tr("Open in terminal"));
    this->addDefaultActionType(kActSelectAll, "SelectAll", tr("Select all"));
    this->addDefaultActionType(kActCut, "Cut", tr("Cut"));
    this->addDefaultActionType(kActCopy, "Copy", tr("Copy"));
    this->addDefaultActionType(kActPaste, "Paste", tr("Paste"));
    this->addDefaultActionType(kActDelete, "Delete", tr("Delete"));
    this->addDefaultActionType(kActCompleteDeletion, "CompleteDeletion", tr("Delete"));
    this->addDefaultActionType(kActRename, "Rename", tr("Rename"));
    this->addDefaultActionType(kActRefreshView, "RefreshView", tr("Refresh"));
    this->addDefaultActionType(kActCompress, "Compress", tr("Compress"));
    this->addDefaultActionType(kActCreateSymlink, "CreateSymlink", tr("Create link"));
    this->addDefaultActionType(kActSendToDesktop, "SendToDesktop", tr("Send to desktop"));
    this->addDefaultActionType(kActSendToBluetooth, "SendToBluetooth", tr("Bluetooth"));
    this->addDefaultActionType(kActName, "Name", tr("Name"));
    this->addDefaultActionType(kActSize, "Size", tr("Size"));
    this->addDefaultActionType(kActType, "Type", tr("Type"));
    this->addDefaultActionType(kActCreatedDate, "CreatedDate", tr("Time created"));
    this->addDefaultActionType(kActLastModifiedDate, "LastModifiedDate", tr("Time modified"));
    this->addDefaultActionType(kActProperty, "Property", tr("Properties"));
    this->addDefaultActionType(kActClearTrash, "ClearTrash", tr("Empty Trash"));
    this->addDefaultActionType(kActSeparator, "Separator", tr(""));
    this->addDefaultActionType(kActDisplayAsList, "List", tr("List"));
    this->addDefaultActionType(kActDisplayAsIcon, "Icon", tr("Icon"));

    // TODO(Lee): icon init
}

void DefaultActionData::initDefaultActionEvent()
{
    // TODO(Lee,Lym): actionType-Event
    d->actionTypeToEventType.insert(kActOpen, kOpenFiles);   // TODO(Lee,Lym)要不只保留 kOpenFiles？
    d->actionTypeToEventType.insert(kActNewFolder, kMkdir);
    d->actionTypeToEventType.insert(kActNewDocument, kTouchFile);
    d->actionTypeToEventType.insert(kActOpenInNewWindow, kOpenNewWindow);
    d->actionTypeToEventType.insert(kActOpenInNewTab, kOpenNewTab);
    d->actionTypeToEventType.insert(kActOpenWith, kOpenFilesByApp);
    d->actionTypeToEventType.insert(kActOpenInTerminal, kOpenInTerminal);
    d->actionTypeToEventType.insert(kActCut, kWriteUrlsToClipboard);   // TODO(Lee,Lym)?
    d->actionTypeToEventType.insert(kActCopy, kWriteUrlsToClipboard);   // TODO(Lee,Lym)?
    d->actionTypeToEventType.insert(kActDelete, kMoveToTrash);
    d->actionTypeToEventType.insert(kActRename, kRenameFile);
    d->actionTypeToEventType.insert(kActCreateSymlink, kCreateSymlink);
}

ActionDataContainer DefaultActionData::addDefaultActionType(ActionType type, const QString &actPredicate, const QString &actionText)
{
    ActionDataContainer newAct(type, actionText);
    d->allActionTypeToData[type] = newAct;
    d->allActionPredicateToText.insert(actPredicate, tr("Office Text"));
    d->allActionPredicateToTypes.insert(actPredicate, type);
    return newAct;
}
