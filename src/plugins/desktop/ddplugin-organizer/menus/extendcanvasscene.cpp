// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "extendcanvasscene_p.h"
#include "organizermenu_defines.h"
#include "config/configpresenter.h"
#include "view/collectionview.h"
#include "models/collectionmodel.h"
#include "utils/renamedialog.h"
#include "utils/fileoperator.h"
#include "core/ddplugin-canvas/menu/canvasmenu_defines.h"

#include "plugins/common/core/dfmplugin-menu/menuscene/menuutils.h"
#include "plugins/common/core/dfmplugin-menu/menuscene/action_defines.h"
#include "dfm-framework/dpf.h"

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
    QAction *tempAction = parent->addAction(predicateName.value(ActionID::kOrganizeDesktop));
    predicateAction[ActionID::kOrganizeDesktop] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeDesktop));
    tempAction->setCheckable(true);
    tempAction->setChecked(turnOn);

    if (turnOn) {
#ifdef EnableCollectionModeMenu
        tempAction = parent->addAction(predicateName.value(ActionID::kOrganizeBy));
        tempAction->setMenu(organizeBySubActions(parent));
        predicateAction[ActionID::kOrganizeBy] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeBy));
#endif

#ifdef EnableDisplaySizeMenu
        // display size
        tempAction = parent->addAction(predicateName.value(ActionID::kDisplaySize));
        tempAction->setMenu(displaySizeSubActions(parent));
        predicateAction[ActionID::kDisplaySize] = tempAction;
        tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kDisplaySize));
#endif
    }

    tempAction = parent->addAction(predicateName.value(ActionID::kOrganizeOptions));
    predicateAction[ActionID::kOrganizeOptions] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeOptions));
}

void ExtendCanvasScenePrivate::normalMenu(QMenu *parent)
{
    if (Q_UNLIKELY(selectFiles.isEmpty())) {
        qWarning() << "no files for normal menu.";
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
    // auto arrage
    {
        auto actionIter = std::find_if(actions.begin(), actions.end(), [](const QAction *ac) {
            return ac->property(ActionPropertyKey::kActionID).toString() == ddplugin_canvas::ActionID::kAutoArrange;
        });

        if (actionIter != actions.end() && turnOn) {
            bool hide = false;
            if (CfgPresenter->mode() == OrganizerMode::kCustom) {
                hide = onCollection;   // don't show on colletion.
            } else if (CfgPresenter->mode() == OrganizerMode::kNormalized) {
                hide = true;   // don't show in normal mode.
            }
            if (hide)
                (*actionIter)->setVisible(false);
        }
    }

    // sort by
    {
        auto actionIter = std::find_if(actions.begin(), actions.end(), [](const QAction *ac) {
            return ac->property(ActionPropertyKey::kActionID).toString() == ddplugin_canvas::ActionID::kSortBy;
        });

        // normal mode
        if (actionIter != actions.end()
            && turnOn
            && CfgPresenter->mode() == OrganizerMode::kNormalized) {
            // on desktop
            if (!onCollection)
                (*actionIter)->setVisible(false);
        }
    }

    // select all
    {
        auto actionIter = std::find_if(actions.begin(), actions.end(), [](const QAction *ac) {
            return ac->property(ActionPropertyKey::kActionID).toString() == dfmplugin_menu::ActionID::kSelectAll;
        });

        // normal mode
        if (actionIter != actions.end()
            && turnOn
            && CfgPresenter->mode() == OrganizerMode::kNormalized) {
            // on desktop
            if (!onCollection)
                (*actionIter)->setVisible(false);
        }
    }

    // wallpager
    {
        auto actionIter = std::find_if(actions.begin(), actions.end(), [](const QAction *ac) {
            return ac->property(ActionPropertyKey::kActionID).toString() == ddplugin_canvas::ActionID::kWallpaperSettings;
        });

        // normal mode
        if (actionIter != actions.end()
            && turnOn
            && CfgPresenter->mode() == OrganizerMode::kNormalized) {
            // on onCollection
            if (onCollection)
                (*actionIter)->setVisible(false);
        }
    }

    // Organize Desktop
    {
        auto actionIter = std::find_if(actions.begin(), actions.end(), [](const QAction *ac) {
            return ac->property(ActionPropertyKey::kActionID).toString() == ddplugin_canvas::ActionID::kDisplaySettings;
        });

        if (actionIter == actions.end()) {
            qWarning() << "can not find action:"
                       << "display-settings";
        } else {
            QAction *indexAction = *actionIter;
            parent->insertAction(indexAction, predicateAction[ActionID::kOrganizeDesktop]);
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
    }

    // icon size
    {
        auto iconSizeIter = std::find_if(actions.begin(), actions.end(), [](const QAction *ac) {
            return ac->property(ActionPropertyKey::kActionID).toString() == ddplugin_canvas::ActionID::kIconSize;
        });

        if (iconSizeIter != actions.end() && turnOn) {
            (*iconSizeIter)->setVisible(false);
            (*iconSizeIter)->setEnabled(false);
#ifdef EnableDisplaySizeMenu
            if (auto ac = predicateAction.value(displaySizeToActionID(CfgPresenter->displaySize())))
                ac->setChecked(true);
            parent->insertAction((*iconSizeIter), predicateAction[ActionID::kDisplaySize]);
#endif
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
    //        qWarning() << "can not find action:" << "display-settings";
    //        return ;
    //    }
}

QMenu *ExtendCanvasScenePrivate::organizeBySubActions(QMenu *menu)
{
    QMenu *subMenu = new QMenu(menu);

    //    QAction *tempAction = subMenu->addAction(predicateName.value(ActionID::kOrganizeByCustom));
    //    predicateAction[ActionID::kOrganizeByCustom] = tempAction;
    //    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeByCustom));
    //    tempAction->setCheckable(true);

    //    QAction *tempAction = subMenu->addAction(predicateName.value(ActionID::kOrganizeByType));
    //    predicateAction[ActionID::kOrganizeByType] = tempAction;
    //    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeByType));
    //    tempAction->setCheckable(true);

    //    tempAction = subMenu->addAction(predicateName.value(ActionID::kOrganizeByTimeAccessed));
    //    predicateAction[ActionID::kOrganizeByTimeAccessed] = tempAction;
    //    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeByTimeAccessed));
    //    tempAction->setCheckable(true);

    //    tempAction = subMenu->addAction(predicateName.value(ActionID::kOrganizeByTimeModified));
    //    predicateAction[ActionID::kOrganizeByTimeModified] = tempAction;
    //    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeByTimeModified));
    //    tempAction->setCheckable(true);

    //    tempAction = subMenu->addAction(predicateName.value(ActionID::kOrganizeByTimeCreated));
    //    predicateAction[ActionID::kOrganizeByTimeCreated] = tempAction;
    //    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kOrganizeByTimeCreated));
    //    tempAction->setCheckable(true);

    return subMenu;
}

#ifdef EnableDisplaySizeMenu

QMenu *ExtendCanvasScenePrivate::displaySizeSubActions(QMenu *menu)
{
    QMenu *subMenu = new QMenu(menu);

    QAction *tempAction = subMenu->addAction(predicateName.value(ActionID::kDisplaySizeSmaller));
    predicateAction[ActionID::kDisplaySizeSmaller] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kDisplaySizeSmaller));
    tempAction->setCheckable(true);

    tempAction = subMenu->addAction(predicateName.value(ActionID::kDisplaySizeNormal));
    predicateAction[ActionID::kDisplaySizeNormal] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kDisplaySizeNormal));
    tempAction->setCheckable(true);

    tempAction = subMenu->addAction(predicateName.value(ActionID::kDisplaySizeLarger));
    predicateAction[ActionID::kDisplaySizeLarger] = tempAction;
    tempAction->setProperty(ActionPropertyKey::kActionID, QString(ActionID::kDisplaySizeLarger));
    tempAction->setCheckable(true);

    return subMenu;
}
#endif

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
        { ddplugin_canvas::ActionID::kSrtTimeModified, Global::ItemRoles::kItemFileLastModifiedRole }
    };

    if (sortRole.contains(actionId)) {
        Global::ItemRoles role = sortRole.value(actionId);
        if (view)
            view->sort(role);
        else
            qCritical() << "invaild view to sort.";
        return true;
    }

    return false;
}

#ifdef EnableDisplaySizeMenu
QString ExtendCanvasScenePrivate::displaySizeToActionID(DisplaySize size)
{
    QString ret;
    switch (size) {
    case kSmaller:
        ret = ActionID::kDisplaySizeSmaller;
        break;
    case kNormal:
        ret = ActionID::kDisplaySizeNormal;
        break;
    case kLarger:
        ret = ActionID::kDisplaySizeLarger;
        break;
    default:
        break;
    }
    return ret;
}
#endif

ExtendCanvasScene::ExtendCanvasScene(QObject *parent)
    : AbstractMenuScene(parent), d(new ExtendCanvasScenePrivate(this))
{
    d->predicateName[ActionID::kOrganizeDesktop] = tr("Organize desktop");
    d->predicateName[ActionID::kOrganizeOptions] = tr("Desktop options");
    d->predicateName[ActionID::kOrganizeBy] = tr("Organize by");

    // organize by subactions
    d->predicateName[ActionID::kOrganizeByCustom] = tr("Custom collection");
    d->predicateName[ActionID::kOrganizeByType] = tr("Type");
    d->predicateName[ActionID::kOrganizeByTimeAccessed] = tr("Time accessed");
    d->predicateName[ActionID::kOrganizeByTimeModified] = tr("Time modified");
    d->predicateName[ActionID::kOrganizeByTimeCreated] = tr("Time created");

    d->predicateName[ActionID::kCreateACollection] = tr("Create a collection");

#ifdef EnableDisplaySizeMenu
    // display size and sub actions
    d->predicateName[ActionID::kDisplaySize] = tr("Display Size");
    d->predicateName[ActionID::kDisplaySizeSmaller] = tr("Smaller");
    d->predicateName[ActionID::kDisplaySizeNormal] = tr("Normal");
    d->predicateName[ActionID::kDisplaySizeLarger] = tr("Larger");
#endif
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
    if (!parent)
        return false;

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
        qDebug() << "organizer for canvas:" << actionId;
        if (actionId == ActionID::kOrganizeDesktop) {
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
        }
#ifdef EnableDisplaySizeMenu
        else if (actionId == ActionID::kDisplaySizeSmaller) {
            emit CfgPresenter->changeDisplaySize(DisplaySize::kSmaller);
        } else if (actionId == ActionID::kDisplaySizeNormal) {
            emit CfgPresenter->changeDisplaySize(DisplaySize::kNormal);
        } else if (actionId == ActionID::kDisplaySizeLarger) {
            emit CfgPresenter->changeDisplaySize(DisplaySize::kLarger);
        }
#endif
        else if (actionId == ActionID::kOrganizeOptions) {
            emit CfgPresenter->showOptionWindow();
        }
        return true;
    }

    return AbstractMenuScene::triggered(action);
}

bool ExtendCanvasScene::actionFilter(AbstractMenuScene *caller, QAction *action)
{
    if (d->onCollection && caller && action) {
        auto actionId = action->property(ActionPropertyKey::kActionID).toString();
        bool isCanvas = caller->name() == "CanvasMenu";
        Q_ASSERT_X(isCanvas, "ExtendCanvasScene", "parent scene is not CanvasMenu");
        if (isCanvas) {
            qDebug() << "filter action" << actionId;
            if (Q_UNLIKELY(!d->view)) {
                qWarning() << "warning:can not get collection view, and filter action failed.";
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
                    if (Q_UNLIKELY(!index.isValid()))
                        qWarning() << "can not rename: invaild file" << d->focusFile;
                    else
                        d->view->edit(index, QAbstractItemView::AllEditTriggers, nullptr);
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
            }
            return false;
        } else {
            qCritical() << "ExtendCanvasScene's parent is not CanvasMenu";
        }
    }

    return false;
}
