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
#include "private/actiontypemanager_p.h"

#include "dfm-base/dfm_event_defines.h"

#include <dfm-framework/framework.h>

#include <QUrl>
#include <QList>

DFMBASE_BEGIN_NAMESPACE

ActionTypeManagerPrivate::ActionTypeManagerPrivate(ActionTypeManager *qq)
    : q(qq)
{
}

ActionTypeManagerPrivate::~ActionTypeManagerPrivate()
{
}

ActionTypeManager &ActionTypeManager::instance()
{
    static ActionTypeManager ins;
    return ins;
}

/*!
 * \brief ActionTypeManager::registerActionType: register Action by action name.
 * \param actText: the name of action.
 * \return return the action type, failed return -1.
 */
QPair<int, ActionDataContainer> ActionTypeManager::registerActionType(const QString &actText)
{
    QPair<int, ActionDataContainer> temp;
    if (actText.isEmpty()) {
        temp.first = -1;
        temp.second = ActionDataContainer();
        return temp;
    }

    static int tempactType = ActionType::kActCustomBase;
    if (tempactType > ActionType::kActMaxCustom) {
        temp.first = -1;
        temp.second = ActionDataContainer();
        return temp;
    }

    tempactType++;
    ActionDataContainer tempActData(tempactType, actText);
    d->actionTypes[tempactType] = tempActData;
    temp.first = tempactType;
    temp.second = tempActData;
    return temp;
}

/*!
 * \brief ActionTypeManager::actionNameByType: get the action data by action type.
 * \param actType: type of target action.
 * \param defaultAct: returns the default value when the fetch fails.
 * \return action data of target type
 */
ActionDataContainer ActionTypeManager::actionNameByType(const int actType, ActionDataContainer defaultAct)
{
    return d->actionTypes.value(actType, defaultAct);
}

/*!
 * \brief ActionTypeManager::actionNameListByTypes: get the actions data by action types.
 * \param actTypes: types of target actions.
 * \return actions data of target types.
 */
QVector<ActionDataContainer> ActionTypeManager::actionNameListByTypes(const QVector<ActionType> &actTypes)
{
    if (actTypes.isEmpty())
        return {};
    QVector<ActionDataContainer> tempActDataList;
    for (auto actType : actTypes) {
        tempActDataList << actionNameByType(actType);
    }
    return tempActDataList;
}

/*!
 * \brief ActionTypeManager::recycleActionType: recycle the action type when the action is discarded.
 * \param actType: type of action to be recycled
 */
void ActionTypeManager::recycleActionType(int actType)
{
    d->actionTypes.remove(actType);
}

/*!
 * \brief actType bind eventType:
 * \param actType
 * \param eventType
 */
/*!
 * \brief ActionTypeManager::actionGlobleEventBind: bind actType and eventType.
 * \param actType: type of action to be bind.
 * \param eventType: type of event to be bind.
 */
void ActionTypeManager::actionGlobleEventBind(ActionType actType, GlobalEventType eventType)
{
    d->actionTypeToEventType.insert(actType, eventType);
}

/*!
 * \brief ActionTypeManager::actionGlobleEventUnBind: unbind actType and eventType.
 * \param actType: type of action to be bind.
 */
void ActionTypeManager::actionGlobleEventUnBind(ActionType actType)
{
    d->actionTypeToEventType.remove(actType);
}

/*!
 * \brief ActionTypeManager::actionGlobleEvent: return the type of event by action's type.
 * \param actType: the type of action.
 * \return returns the target event type
 */
GlobalEventType ActionTypeManager::actionGlobalEvent(ActionType actType)
{
    // TODO(Lee):
    return d->actionTypeToEventType.value(actType, GlobalEventType::kUnknowType);
}

ActionTypeManager::ActionTypeManager(QObject *parent)
    : QObject(parent), d(new ActionTypeManagerPrivate(this))
{
    initDefaultActionData();
    initDefaultActionEvent();
}

ActionTypeManager::~ActionTypeManager()
{
}

void ActionTypeManager::initDefaultActionData()
{
    ActionDataContainer openAct(ActionType::kActOpen, tr("Open"));
    d->actionTypes[ActionType::kActOpen] = openAct;

    ActionDataContainer newFolderAct(ActionType::kActNewFolder, tr("New folder"));
    d->actionTypes[ActionType::kActNewFolder] = newFolderAct;

    ActionDataContainer newDocumentAct(ActionType::kActNewDocument, tr("New document"));
    d->actionTypes[ActionType::kActNewDocument] = newDocumentAct;

    ActionDataContainer openInNewWindowAct(ActionType::kActOpenInNewWindow, tr("Open in new window"));
    d->actionTypes[ActionType::kActOpenInNewWindow] = openInNewWindowAct;

    ActionDataContainer openInNewTabAct(ActionType::kActOpenInNewTab, tr("Open in new tab"));
    d->actionTypes[ActionType::kActOpenInNewTab] = openInNewTabAct;

    ActionDataContainer displayAsAct(ActionType::kActDisplayAs, tr("Display as"));
    d->actionTypes[ActionType::kActDisplayAs] = displayAsAct;

    ActionDataContainer sortByAct(ActionType::kActSortBy, tr("Sort by"));
    d->actionTypes[ActionType::kActSortBy] = sortByAct;

    ActionDataContainer openWithAct(ActionType::kActOpenWith, tr("Open with"));
    d->actionTypes[ActionType::kActOpenWith] = openWithAct;

    ActionDataContainer openAsAdminAct(ActionType::kActOpenAsAdmin, tr("Open as administrator"));
    d->actionTypes[ActionType::kActOpenAsAdmin] = openAsAdminAct;

    ActionDataContainer openInTerminalAct(ActionType::kActOpenInTerminal, tr("Open in terminal"));
    d->actionTypes[ActionType::kActOpenInTerminal] = openInTerminalAct;

    ActionDataContainer selectAllAct(ActionType::kActSelectAll, tr("Select all"));
    d->actionTypes[ActionType::kActSelectAll] = selectAllAct;

    ActionDataContainer cutAct(ActionType::kActCut, tr("Cut"));
    d->actionTypes[ActionType::kActCut] = cutAct;

    ActionDataContainer copyAct(ActionType::kActCopy, tr("Copy"));
    d->actionTypes[ActionType::kActCopy] = copyAct;

    ActionDataContainer pasteAct(ActionType::kActPaste, tr("Paste"));
    d->actionTypes[ActionType::kActPaste] = pasteAct;

    ActionDataContainer deleteAct(ActionType::kActDelete, tr("Delete"));
    d->actionTypes[ActionType::kActDelete] = deleteAct;

    ActionDataContainer completeDeletionAct(ActionType::kActCompleteDeletion, tr("Delete"));
    d->actionTypes[ActionType::kActCompleteDeletion] = completeDeletionAct;

    ActionDataContainer renameAct(ActionType::kActRename, tr("Rename"));
    d->actionTypes[ActionType::kActRename] = renameAct;

    ActionDataContainer compressAct(ActionType::kActCompress, tr("Compress"));
    d->actionTypes[ActionType::kActCompress] = compressAct;

    ActionDataContainer createSymlinkAct(ActionType::kActCreateSymlink, tr("Create link"));
    d->actionTypes[ActionType::kActCreateSymlink] = createSymlinkAct;

    ActionDataContainer sendToDesktopAct(ActionType::kActSendToDesktop, tr("Send to desktop"));
    d->actionTypes[ActionType::kActSendToDesktop] = sendToDesktopAct;

    ActionDataContainer propertyAct(ActionType::kActProperty, tr("Properties"));
    d->actionTypes[ActionType::kActProperty] = propertyAct;

    ActionDataContainer separatorAct(ActionType::kActSeparator, "");
    d->actionTypes[ActionType::kActSeparator] = separatorAct;

    // TODO(Lee): icon init
}

void ActionTypeManager::initDefaultActionEvent()
{
    // TODO(Lee): actionType-Event
    d->actionTypeToEventType.insert(kActOpen, kOpenFiles);   // 要不只保留 kOpenFiles？
    d->actionTypeToEventType.insert(kActNewFolder, kMkdir);
    d->actionTypeToEventType.insert(kActNewDocument, kTouchFile);
    d->actionTypeToEventType.insert(kActOpenInNewWindow, kOpenNewWindow);
    d->actionTypeToEventType.insert(kActOpenInNewTab, kOpenNewTab);
    d->actionTypeToEventType.insert(kActOpenWith, kOpenFilesByApp);
    d->actionTypeToEventType.insert(kActOpenInTerminal, kOpenInTerminal);
    d->actionTypeToEventType.insert(kActCut, kWriteUrlsToClipboard);   // ?
    d->actionTypeToEventType.insert(kActCopy, kWriteUrlsToClipboard);   // ?
    d->actionTypeToEventType.insert(kActDelete, kMoveToTrash);
    d->actionTypeToEventType.insert(kActRename, kRenameFile);
    d->actionTypeToEventType.insert(kActCreateSymlink, kCreateSymlink);
}

DFMBASE_END_NAMESPACE
