// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultfileiterator.h"
#include "private/vaultfileiteratorprivate.h"
#include "utils/vaulthelper.h"
#include "vaultfileinfo.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>

#include <dfm-io/denumerator.h>
#include <dfm-io/dfmio_utils.h>

#include <QSharedPointer>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_vault;
USING_IO_NAMESPACE

VaultFileIterator::VaultFileIterator(const QUrl &url, const QStringList &nameFilters, QDir::Filters filters, QDirIterator::IteratorFlags flags)
    : AbstractDirIterator(VaultHelper::vaultToLocalUrl(url), nameFilters, filters, flags)
{
    QUrl localUrl = VaultHelper::vaultToLocalUrl(url);
    fmDebug() << "Vault: Local URL:" << localUrl.toString();

    dfmioDirIterator.reset(new DFMIO::DEnumerator(localUrl, nameFilters,
                                                  static_cast<DEnumerator::DirFilter>(static_cast<int32_t>(filters)),
                                                  static_cast<DEnumerator::IteratorFlag>(static_cast<uint8_t>(flags))));
    if (!dfmioDirIterator)
        fmCritical("Vault: create DEnumerator failed!");

    const QUrl &hidUrl = DFMIO::DFMUtils::buildFilePath(localUrl.toString().toStdString().c_str(), ".hidden", nullptr);
    hideFileList = DFMIO::DFMUtils::hideListFromUrl(hidUrl);
}

VaultFileIterator::~VaultFileIterator()
{
    fmDebug() << "Vault: Destroying VaultFileIterator";
}

QUrl VaultFileIterator::next()
{
    if (dfmioDirIterator)
        currentUrl = VaultHelper::instance()->pathToVaultVirtualUrl(dfmioDirIterator->next().path());

    return currentUrl;
}

bool VaultFileIterator::hasNext() const
{
    return (dfmioDirIterator ? dfmioDirIterator->hasNext() : false);
}

QString VaultFileIterator::fileName() const
{
    return fileUrl().fileName();
}

QUrl VaultFileIterator::fileUrl() const
{
    return currentUrl;
}

const FileInfoPointer VaultFileIterator::fileInfo() const
{
    assert(QThread::currentThread() != qApp->thread());

    QUrl url = VaultHelper::instance()->vaultToLocalUrl(fileUrl());
    QSharedPointer<DFileInfo> fileinfo = dfmioDirIterator->fileInfo();

    const QString &fileName = fileinfo->attribute(DFileInfo::AttributeID::kStandardName, nullptr).toString();
    bool isHidden = false;
    if (fileName.startsWith(".")) {
        isHidden = true;
    } else {
        isHidden = hideFileList.contains(fileName);
    }

    QSharedPointer<FileInfo> info = QSharedPointer<AsyncFileInfo>(new AsyncFileInfo(url, fileinfo));
    info->setExtendedAttributes(ExtInfoType::kFileIsHid, isHidden);
    info.dynamicCast<AsyncFileInfo>()->cacheAsyncAttributes();

    QSharedPointer<FileInfo> infoTrans = InfoFactory::transfromInfo<FileInfo>(url.scheme(), info);
    if (infoTrans) {
        infoTrans->setExtendedAttributes(ExtInfoType::kFileIsHid, isHidden);
        infoTrans->setExtendedAttributes(ExtInfoType::kFileLocalDevice, false);
        infoTrans->setExtendedAttributes(ExtInfoType::kFileCdRomDevice, false);
        InfoFactory::cacheFileInfo(infoTrans);
    } else {
        fmWarning() << "Vault: info is nullptr, url = " << url;
        return InfoFactory::create<FileInfo>(fileUrl());
    }

    FileInfoPointer vaultInfo(new VaultFileInfo(fileUrl(), infoTrans));
    return vaultInfo;
}

bool VaultFileIterator::initIterator()
{
    if (dfmioDirIterator)
        return dfmioDirIterator->initEnumerator(oneByOne());
    return false;
}

QUrl VaultFileIterator::url() const
{
    return VaultHelper::instance()->rootUrl();
}
