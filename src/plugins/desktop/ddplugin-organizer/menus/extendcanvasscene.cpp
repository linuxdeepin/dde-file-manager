// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extendcanvasscene_p.h"
#include "organizermenu_defines.h"
#include "config/configpresenter.h"
#include "view/collectionview.h"
#include "models/collectionmodel.h"
#include "utils/renamedialog.h"
#include "utils/fileoperator.h"
#include "ddplugin-canvas/menu/canvasmenu_defines.h"

#include "plugins/common/dfmplugin-menu/menuscene/menuutils.h"
#include "plugins/common/dfmplugin-menu/menuscene/action_defines.h"

#include <dfm-framework/dpf.h>

#include <QMenu>
#include <QDebug>

using namespace ddplugin_organizer;
DFMBASE_USE_NAMESPACE

AbstractMenuScene *ExtendCanvasCreator::create()
{
    return new ExtendCanvasScene();
}

ExtendCanvasScenePrivate::ExtendCanvasScenePrivate(ExtendCanvasScene *qq)
    : AbstractMenuScenePrivate(qq), q(qq)
{
}

void ExtendCanvasScenePrivate::emptyMenu(QMenu *parent)
{
    QAction *tempAction = nullptr;
    // tempAction = parent->addAction(predicateName.value(ActionID::kOrganizeEnable));
    // predicateAction[ActionID::kOrganizeEnable] = tempAction;
    // tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeEnable));
    // tempAction->setCheckable(true);
    // tempAction->setChecked(turnOn);

    if (turnOn) {
#ifdef EnableCollectionModeMenu
        tempAction = parent->addAction(predicateName.value(ActionID::kOrganizeBy));
        tempAction->setMenu(organizeBySubActions(parent));
        predicateAction[ActionID::kOrganizeBy] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeBy));
#endif
        if (ConfigPresenter::instance()->organizeOnTriggered()) {
            tempAction = new QAction(predicateName.value(ActionID::kOrganizeTrigger), parent);
            predicateAction[ActionID::kOrganizeTrigger] = tempAction;
            tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeTrigger));
        }
    }

    tempAction = parent->addAction(predicateName.value(ActionID::kOrganizeOptions));
    predicateAction[ActionID::kOrganizeOptions] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeOptions));
}

void ExtendCanvasScenePrivate::normalMenu(QMenu *parent)
{
    if (Q_UNLIKELY(selectFiles.isEmpty())) {
        fmWarning() << "No files selected for normal menu creation";
        return;
    }

    if (turnOn && CfgPresenter->mode() == OrganizerMode::kCustom) {
        QAction *tempAction = parent->addAction(predicateName.value(ActionID::kCreateACollection));
        predicateAction[ActionID::kCreateACollection] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kCreateACollection));
    }
}

void ExtendCanvasScenePrivate::updateEmptyMenu(QMenu *parent)
{
    auto actions = parent->actions();

    // Organize Desktop
    {
        auto it = std::find_if(actions.begin(), actions.end(), [](QAction *action) {
            return action->property(ActionPropertyKey::kActionID).toString() == ddplugin_canvas::ActionID::kSortBy;
        });

        // net one
        int pos { (it != actions.end()) ? int(std::distance(actions.begin(), it)) + 1 : -1 };
        if (pos == -1 || pos >= actions.size()) {
            fmWarning() << "Cannot find sort action for menu positioning";
        } else {
            QAction *indexAction = actions[pos];
            parent->insertAction(indexAction, predicateAction[ActionID::kOrganizeEnable]);
            if (turnOn) {
                parent->insertAction(indexAction, predicateAction[ActionID::kOrganizeBy]);
                parent->insertAction(indexAction, predicateAction[ActionID::kOrganizeOptions]);

                if (CfgPresenter->mode() == OrganizerMode::kCustom) {
                    predicateAction[ActionID::kOrganizeByCustom]->setChecked(true);
                } else if (CfgPresenter->mode() == OrganizerMode::kNormalized) {
                    QString id = classifierToActionID(CfgPresenter->classification());
                    if (auto ac = predicateAction.value(id))
                        ac->setChecked(true);
                }

                // on collection
                if (onCollection)
                    indexAction->setVisible(false);
            } else {
                parent->insertAction(indexAction, predicateAction[ActionID::kOrganizeOptions]);
            }
        }

        if (turnOn) {
            auto refreshPos = std::find_if(actions.begin(), actions.end(), [](QAction *action) {
                return action->property(ActionPropertyKey::kActionID).toString() == "refresh";
            });
            if (refreshPos == actions.end()) {
                parent->addAction(predicateAction[ActionID::kOrganizeTrigger]);
            } else {
                int idx = int(std::distance(actions.begin(), refreshPos)) + 1;
                if (idx < actions.count()) {
                    auto act = actions.at(idx);
                    parent->insertAction(act, predicateAction[ActionID::kOrganizeTrigger]);
                }
            }
        }
    }
}

void ExtendCanvasScenePrivate::updateNormalMenu(QMenu *parent)
{
    //    auto actions = parent->actions();
    //    auto actionIter = std::find_if(actions.begin(), actions.end(), [](const QAction *ac){
    //        return ac->property(ActionPropertyKey::kActionID).toString() == QString("display-settings");
    //    });

    //    if (actionIter == actions.end()) {
    //        fmWarning() << "can not find action:" << "display-settings";
    //        return ;
    //    }
}

QMenu *ExtendCanvasScenePrivate::organizeBySubActions(QMenu *menu)
{
    QMenu *subMenu = new QMenu(menu);

#ifdef EnableCollectionModeMenu
    QAction *tempAction = subMenu->addAction(predicateName.value(ActionID::kOrganizeByCustom));
    predicateAction[ActionID::kOrganizeByCustom] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeByCustom));
    tempAction->setCheckable(true);

    tempAction = subMenu->addAction(predicateName.value(ActionID::kOrganizeByType));
    predicateAction[ActionID::kOrganizeByType] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeByType));
    tempAction->setCheckable(true);

//        tempAction = subMenu->addAction(predicateName.value(ActionID::kOrganizeByTimeAccessed));
//        predicateAction[ActionID::kOrganizeByTimeAccessed] = tempAction;
//        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeByTimeAccessed));
//        tempAction->setCheckable(true);

//        tempAction = subMenu->addAction(predicateName.value(ActionID::kOrganizeByTimeModified));
//        predicateAction[ActionID::kOrganizeByTimeModified] = tempAction;
//        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeByTimeModified));
//        tempAction->setCheckable(true);

//        tempAction = subMenu->addAction(predicateName.value(ActionID::kOrganizeByTimeCreated));
//        predicateAction[ActionID::kOrganizeByTimeCreated] = tempAction;
//        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeByTimeCreated));
//        tempAction->setCheckable(true);
#endif

    return subMenu;
}

QString ExtendCanvasScenePrivate::classifierToActionID(Classifier cf)
{
    QString ret;
    switch (cf) {
    case kType:
        ret = ActionID::kOrganizeByType;
        break;
    case kTimeCreated:
        ret = ActionID::kOrganizeByTimeCreated;
        break;
    case kTimeModified:
        ret = ActionID::kOrganizeByTimeModified;
        break;
    case kLabel:

        break;
    case kName:

        break;
    case kSize:

        break;
    default:
        break;
    }
    return ret;
}

bool ExtendCanvasScenePrivate::triggerSortby(const QString &actionId)
{
    static const QMap<QString, Global::ItemRoles> sortRole = {
        { ddplugin_canvas::ActionID::kSrtName, Global::ItemRoles::kItemFileDisplayNameRole },
        { ddplugin_canvas::ActionID::kSrtSize, Global::ItemRoles::kItemFileSizeRole },
        { ddplugin_canvas::ActionID::kSrtType, Global::ItemRoles::kItemFileMimeTypeRole },
        { ddplugin_canvas::ActionID::kSrtTimeModified, Global::ItemRoles::kItemFileLastModifiedRole },
        { ddplugin_canvas::ActionID::kSrtTimeCreated, Global::ItemRoles::kItemFileCreatedRole }
    };

    if (sortRole.contains(actionId)) {
        Global::ItemRoles role = sortRole.value(actionId);
        if (view) {
            fmDebug() << "Triggering sort by" << actionId << "for collection view";
            view->sort(role);
        } else {
            fmCritical() << "Invalid view for sorting operation";
        }
        return true;
    }

    return false;
}

ExtendCanvasScene::ExtendCanvasScene(QObject *parent)
    : AbstractMenuScene(parent), d(new ExtendCanvasScenePrivate(this))
{
    d->predicateName[ActionID::kOrganizeEnable] = tr("Enable desktop organization");
    d->predicateName[ActionID::kOrganizeTrigger] = tr("Organize desktop");
    d->predicateName[ActionID::kOrganizeOptions] = tr("Desktop Settings");
    d->predicateName[ActionID::kOrganizeBy] = tr("Organize by");

    // organize by subactions
    d->predicateName[ActionID::kOrganizeByCustom] = tr("Custom collection");
    d->predicateName[ActionID::kOrganizeByType] = tr("Type");
    d->predicateName[ActionID::kOrganizeByTimeAccessed] = tr("Time accessed");
    d->predicateName[ActionID::kOrganizeByTimeModified] = tr("Time modified");
    d->predicateName[ActionID::kOrganizeByTimeCreated] = tr("Time created");

    d->predicateName[ActionID::kCreateACollection] = tr("Create a collection");
}

QString ExtendCanvasScene::name() const
{
    return ExtendCanvasCreator::name();
}

bool ExtendCanvasScene::initialize(const QVariantHash &params)
{
    d->turnOn = CfgPresenter->isEnable();
    d->isEmptyArea = params.value(MenuParamKey::kIsEmptyArea).toBool();
    d->onDesktop = params.value(MenuParamKey::kOnDesktop).toBool();
    d->selectFiles = params.value(MenuParamKey::kSelectFiles).value<QList<QUrl>>();
    d->onCollection = params.value(CollectionMenuParams::kOnColletion, false).toBool();
    d->view = reinterpret_cast<CollectionView *>(params.value(CollectionMenuParams::kColletionView).toLongLong());
    d->currentDir = params[MenuParamKey::kCurrentDir].toUrl();

    if (!d->selectFiles.isEmpty())
        d->focusFile = d->selectFiles.first();

    return d->onDesktop;
}

AbstractMenuScene *ExtendCanvasScene::scene(QAction *action) const
{
    if (!action)
        return nullptr;

    if (d->predicateAction.values().contains(action))
        return const_cast<ExtendCanvasScene *>(this);

    return AbstractMenuScene::scene(action);
}

bool ExtendCanvasScene::create(QMenu *parent)
{
    if (!parent) {
        fmWarning() << "Cannot create menu - parent menu is null";
        return false;
    }

    if (d->isEmptyArea) {
        d->emptyMenu(parent);
    } else {
        d->normalMenu(parent);
    }

    // 创建子场景菜单
    return AbstractMenuScene::create(parent);
}

void ExtendCanvasScene::updateState(QMenu *parent)
{
    if (d->isEmptyArea) {
        d->updateEmptyMenu(parent);
    } else {
        d->updateNormalMenu(parent);
    }

    AbstractMenuScene::updateState(parent);
}

bool ExtendCanvasScene::triggered(QAction *action)
{
    auto actionId = action->property(ActionPropertyKey::kActionID).toString();
    if (d->predicateAction.values().contains(action)) {
        fmDebug() << "Organizer action triggered:" << actionId;

        if (actionId == ActionID::kOrganizeEnable) {
            emit CfgPresenter->changeEnableState(action->isChecked());
        } else if (actionId == ActionID::kOrganizeByCustom) {
            emit CfgPresenter->switchToCustom();
        } else if (actionId == ActionID::kOrganizeByType) {
            emit CfgPresenter->switchToNormalized(Classifier::kType);
        } else if (actionId == ActionID::kOrganizeByTimeAccessed) {

        } else if (actionId == ActionID::kOrganizeByTimeModified) {
            emit CfgPresenter->switchToNormalized(Classifier::kTimeModified);
        } else if (actionId == ActionID::kOrganizeByTimeCreated) {
            emit CfgPresenter->switchToNormalized(Classifier::kTimeCreated);
        } else if (actionId == ActionID::kCreateACollection) {
            emit CfgPresenter->newCollection(d->selectFiles);
        } else if (actionId == ActionID::kOrganizeOptions) {
            emit CfgPresenter->showOptionWindow();
        } else if (actionId == ActionID::kOrganizeTrigger) {
            emit CfgPresenter->reorganizeDesktop();
        }
        return true;
    }

    return AbstractMenuScene::triggered(action);
}

bool ExtendCanvasScene::actionFilter(AbstractMenuScene *caller, QAction *action)
{
    if (!caller || !action) {
        fmWarning() << "Invalid parameters for action filter - caller or action is null";
        return false;
    }

    auto actionId = action->property(ActionPropertyKey::kActionID).toString();

    if (dfmplugin_menu::ActionID::kRename == actionId) {
        // cheat, use canvas event to trigger batch rename.
        fmDebug() << "Triggering canvas rename hook";
        if (dpfHookSequence->run("ddplugin_canvas", "hook_CanvasView_KeyPress",
                                 0, int(Qt::Key_F2), int(Qt::NoModifier), nullptr))
            return true;
    }

    if (d->onCollection) {
        bool isCanvas = caller->name() == "CanvasMenu";
        Q_ASSERT_X(isCanvas, "ExtendCanvasScene", "parent scene is not CanvasMenu");
        if (isCanvas) {
            fmDebug() << "Filtering action on collection:" << actionId;
            if (Q_UNLIKELY(!d->view)) {
                fmWarning() << "Cannot get collection view, action filter failed";
                return false;
            }

            if (dfmplugin_menu::ActionID::kSelectAll == actionId) {
                d->view->selectAll();
                return true;
            } else if (d->triggerSortby(actionId)) {
                return true;
            } else if (dfmplugin_menu::ActionID::kRename == actionId) {
                if (1 == d->selectFiles.count()) {
                    auto index = d->view->model()->index(d->focusFile);
                    if (Q_UNLIKELY(!index.isValid())) {
                        fmWarning() << "Cannot rename - invalid file:" << d->focusFile;
                    } else {
                        d->view->edit(index, QAbstractItemView::AllEditTriggers, nullptr);
                    }
                } else {
                    RenameDialog renameDlg(d->selectFiles.count());
                    renameDlg.moveToCenter();

                    // see DDialog::exec,it will return the index of buttons
                    if (1 == renameDlg.exec()) {
                        RenameDialog::ModifyMode mode = renameDlg.modifyMode();
                        if (RenameDialog::kReplace == mode) {
                            auto content = renameDlg.getReplaceContent();
                            FileOperatorIns->renameFiles(d->view, d->selectFiles, content, true);
                        } else if (RenameDialog::kAdd == mode) {
                            auto content = renameDlg.getAddContent();
                            FileOperatorIns->renameFiles(d->view, d->selectFiles, content);
                        } else if (RenameDialog::kCustom == mode) {
                            auto content = renameDlg.getCustomContent();
                            FileOperatorIns->renameFiles(d->view, d->selectFiles, content, false);
                        }
                    }
                }
                return true;
            } else if (dfmplugin_menu::ActionID::kReverseSelect == actionId) {
                d->view->toggleSelect();
                return true;
            }
            return false;
        } else {
            fmCritical() << "ExtendCanvasScene's parent is not CanvasMenu";
        }
    } else {
        // 为了能够选中所有集合中的文件
        if (dfmplugin_menu::ActionID::kSelectAll == actionId)
            dpfSlotChannel->push("ddplugin_organizer", "slot_CollectionModel_SelectAll");
    }

    return false;
}
