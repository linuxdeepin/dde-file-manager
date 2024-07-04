// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "crumbinterface.h"
#include "utils/titlebarhelper.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>

#include <dfm-framework/event/event.h>

using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE

CrumbInterface::CrumbInterface(QObject *parent)
    : QObject(parent)
{
}

void CrumbInterface::setKeepAddressBar(bool keep)
{
    keepAddr = keep;
}

void CrumbInterface::setSupportedScheme(const QString &scheme)
{
    curScheme = scheme;
}

bool CrumbInterface::isKeepAddressBar()
{
    return keepAddr;
}

bool CrumbInterface::isSupportedScheme(const QString &scheme)
{
    return curScheme == scheme;
}

void CrumbInterface::processAction(CrumbInterface::ActionType type)
{
    switch (type) {
    case kEscKeyPressed:
    case kClearButtonPressed:
        emit hideAddressBar(keepAddr);
        break;
    case kAddressBarLostFocus:
        if (!keepAddr)
            emit hideAddressBar(keepAddr);
        break;
    }
}

void CrumbInterface::crumbUrlChangedBehavior(const QUrl &url)
{
    if (keepAddr) {
        emit keepAddressBar(url);
    } else {
        emit hideAddrAndUpdateCrumbs(url);
    }
}

QList<CrumbData> CrumbInterface::seprateUrl(const QUrl &url)
{
    // TODO(zhangs): follow
    QList<QVariantMap> mapGroup;
    if (dpfHookSequence->run("dfmplugin_titlebar", "hook_Crumb_Seprate", url, &mapGroup))
        return TitleBarHelper::tansToCrumbDataList(mapGroup);

    if (url.scheme() == Global::Scheme::kFile)
        return TitleBarHelper::crumbSeprateUrl(url);

    // default method
    QList<CrumbData> list;
    QList<QUrl> urls;
    urls.push_back(url);
    UrlRoute::urlParentList(url, &urls);

    for (int count = urls.size() - 1; count >= 0; count--) {
        QUrl curUrl { urls.at(count) };
        QStringList pathList { curUrl.path().split("/") };
        QString displayText = pathList.isEmpty() ? "" : pathList.last();
        if (curUrl.scheme() == Global::Scheme::kTrash) {
            auto info = InfoFactory::create<FileInfo>(curUrl);
            displayText = info ? info->displayOf(DisPlayInfoType::kFileDisplayName) : displayText;
        }
        CrumbData data { curUrl, displayText};
        if (UrlRoute::isRootUrl(curUrl))
            data.iconName = UrlRoute::icon(curUrl.scheme()).name();
        list.append(data);
    }

    return list;
}

/*!
 * \brief Start request a completion list for address bar auto-completion.
 *
 * \param url The base url need to be completed.
 *
 * Since completion list can be long, so we need do async completion. Calling this
 * function will start a completion request and the completion list item will be sent
 * via signal completionFound. When user no longer need current completion list and
 * the transmission isn't completed, you should call cancelCompletionListTransmission.
 * When transmission completed, it will send completionListTransmissionCompleted signal.
 *
 * \sa completionFound, completionListTransmissionCompleted, cancelCompletionListTransmission
 */
void CrumbInterface::requestCompletionList(const QUrl &url)
{
    if (folderCompleterJobPointer) {
        folderCompleterJobPointer->disconnect();
        folderCompleterJobPointer->stopAndDeleteLater();
        folderCompleterJobPointer->setParent(nullptr);
    }
    folderCompleterJobPointer = new TraversalDirThread(url, QStringList(),
                                                       QDir::Dirs | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    folderCompleterJobPointer->setParent(this);
    if (folderCompleterJobPointer.isNull())
        return;

    connect(folderCompleterJobPointer.data(), &TraversalDirThread::updateChildren, this,
            &CrumbInterface::onUpdateChildren, Qt::DirectConnection);

    connect(
            folderCompleterJobPointer.data(), &TraversalDirThread::finished, this,
            [this]() {
                emit completionListTransmissionCompleted();
            },
            Qt::QueuedConnection);
    // TODO(liyigang): imple follow interfaces
    //    folderCompleterJobPointer->setTimeCeiling(1000);
    //    folderCompleterJobPointer->setCountCeiling(500);
    folderCompleterJobPointer->start();
}

/*!
 * \brief Cancel the started completion list transmission.
 *
 * \sa completionFound, completionListTransmissionCompleted, requestCompletionList
 */
void CrumbInterface::cancelCompletionListTransmission()
{
    if (folderCompleterJobPointer)
        folderCompleterJobPointer->stop();
}

void CrumbInterface::onUpdateChildren(QList<QUrl> children)
{
    QStringList list;

    for (const auto &child : children) {
        auto info = InfoFactory::create<FileInfo>(child);
        if (info)
            list.append(info->nameOf(NameInfoType::kFileName));
    }
    emit completionFound(list);
}
