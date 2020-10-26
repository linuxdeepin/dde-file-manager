/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "dfmcrumbmanager.h"
#include "dfmcrumbinterface.h"
#include "dfmcrumbbar.h"
#include "plugins/dfmcrumbfactory.h"

#include "controllers/dfmcomputercrumbcontroller.h"
#include "controllers/dfmtrashcrumbcontroller.h"
#include "controllers/dfmfilecrumbcontroller.h"
#include "controllers/dfmbookmarkcrumbcontroller.h"
#include "controllers/dfmtagcrumbcontroller.h"
#include "controllers/dfmnetworkcrumbcontroller.h"
#include "controllers/dfmusersharecrumbcontroller.h"
#include "controllers/dfmavfscrumbcontroller.h"
#include "controllers/dfmsmbcrumbcontroller.h"
#include "controllers/dfmmtpcrumbcontroller.h"
#include "controllers/dfmsearchcrumbcontroller.h"
#include "controllers/dfmrecentcrumbcontroller.h"
#include "controllers/dfmmdcrumbcontrooler.h"
#include "controllers/dfmmasteredmediacrumbcontroller.h"
#include "controllers/dfmvaultcrumbcontroller.h"

DFM_BEGIN_NAMESPACE

class DFMCrumbManagerPrivate
{
public:
    explicit DFMCrumbManagerPrivate(DFMCrumbManager *qq);

    QMultiHash<const DFMCrumbManager::KeyType, DFMCrumbManager::CrumbCreaterType> controllerCreatorHash;

    DFMCrumbManager *q_ptr;
};

DFMCrumbManagerPrivate::DFMCrumbManagerPrivate(DFMCrumbManager *qq)
    : q_ptr(qq)
{

}

/*!
 * \class DFMCrumbManager
 * \inmodule dde-file-manager-lib
 *
 * \brief DFMCrumbManager is the manager of crumb controllers
 *
 * DFMCrumbManager is the manager of crumb controllers, it holds all avaliable crumb
 * controllers to do crumb bar reletive stuff.
 *
 * User can also write plugins to extend crumb bar functionality (by derive the DFMCrumbInterface
 * class) and DFMCrumbManager do the job about load and manage the plugins.
 *
 * \sa DFMCrumbBar, DFMCrumbInterface
 */

DFMCrumbManager *DFMCrumbManager::instance()
{
    static DFMCrumbManager manager;

    return &manager;
}

bool DFMCrumbManager::isRegisted(const QString &scheme, const std::type_info &info) const
{
    Q_D(const DFMCrumbManager);

    const KeyType &type = KeyType(scheme);

    foreach (const CrumbCreaterType &value, d->controllerCreatorHash.values(type)) {
        if (value.first == info.name())
            return true;
    }

    return false;
}

/*!
 * \brief Create a crumb controller by the given \a fileUrl
 *
 * \param fileUrl The url for creating controller
 * \param crumbBar The DFMCrumbBar pointer.
 *
 * \return Pointer of the created crumb controller.
 *
 * The DFMCrumbBar reference(pointer) is required in order to make sure the crumb controller will always
 * hold a instance of DFMCrumbBar. Thus the controller can invoke method provided by the crumb bar when
 * neccessary.
 */
DFMCrumbInterface *DFMCrumbManager::createControllerByUrl(const DUrl &fileUrl, DFMCrumbBar *crumbBar) const
{
    Q_D(const DFMCrumbManager);

    KeyType theType = fileUrl.scheme();

    const QList<CrumbCreaterType> creatorList = d->controllerCreatorHash.values(theType);

    Q_CHECK_PTR(crumbBar);

    if (!creatorList.isEmpty()) {
        DFMCrumbInterface *i = (creatorList.first().second)();
        i->setCrumbBar(crumbBar);
        return i;
    }

    return nullptr;
}

DFMCrumbManager::DFMCrumbManager(QObject *parent)
    : QObject(parent)
    , d_ptr(new DFMCrumbManagerPrivate(this))
{
    // register built-in
    dRegisterCrumbCreator<DFMFileCrumbController>(QStringLiteral(FILE_SCHEME));
    dRegisterCrumbCreator<DFMTrashCrumbController>(QStringLiteral(TRASH_SCHEME));
    dRegisterCrumbCreator<DFMComputerCrumbController>(QStringLiteral(COMPUTER_SCHEME));
    dRegisterCrumbCreator<DFMBookmarkCrumbController>(QStringLiteral(BOOKMARK_SCHEME));
    dRegisterCrumbCreator<DFMTagCrumbController>(QStringLiteral(TAG_SCHEME));
    dRegisterCrumbCreator<DFMNetworkCrumbController>(QStringLiteral(NETWORK_SCHEME));
    dRegisterCrumbCreator<DFMUserShareCrumbController>(QStringLiteral(USERSHARE_SCHEME));
    dRegisterCrumbCreator<DFMAvfsCrumbController>(QStringLiteral(AVFS_SCHEME));
    dRegisterCrumbCreator<DFMSmbCrumbController>(QStringLiteral(SMB_SCHEME));
    dRegisterCrumbCreator<DFMMtpCrumbController>(QStringLiteral(MTP_SCHEME));
    dRegisterCrumbCreator<DFMSearchCrumbController>(QStringLiteral(SEARCH_SCHEME));
    dRegisterCrumbCreator<DFMRecentCrumbController>(QStringLiteral(RECENT_SCHEME));
    dRegisterCrumbCreator<DFMMDCrumbControoler>(QStringLiteral(DFMMD_SCHEME));
    dRegisterCrumbCreator<DFMMasteredMediaCrumbController>(QStringLiteral(BURN_SCHEME));
    dRegisterCrumbCreator<DFMVaultCrumbController>(QStringLiteral(DFMVAULT_SCHEME));

    // register plugins
    for (const QString &key : DFMCrumbFactory::keys()) {
        const DUrl url(key);

        insertToCreatorHash(KeyType(url.scheme()), CrumbCreaterType(typeid(DFMCrumbFactory).name(), [key] {
            return DFMCrumbFactory::create(key);
        }));
    }
}

DFMCrumbManager::~DFMCrumbManager()
{

}

void DFMCrumbManager::insertToCreatorHash(const DFMCrumbManager::KeyType &type, const DFMCrumbManager::CrumbCreaterType &creator)
{
    Q_D(DFMCrumbManager);

    d->controllerCreatorHash.insertMulti(type, creator);
}

DFM_END_NAMESPACE
