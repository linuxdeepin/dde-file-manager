// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "typeclassifier_p.h"
#include "models/generalmodelfilter.h"
#include "config/configpresenter.h"

#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/schemefactory.h>

using namespace ddplugin_organizer;
DFMBASE_USE_NAMESPACE

namespace {

inline const char kTypeSuffixDoc[] = "pdf,txt,doc,docx,dot,dotx,ppt,pptx,"
                                     "pot,potx,xls,xlsx,xlt,xltx,wps,wpt,rtf,"
                                     "md,latex,et,dps,wdb,wks,csv,dpt,ofd,uof,xml";
inline const char kTypeSuffixPic[] = "jpg,jpeg,jpe,bmp,png,gif,svg,tif,tiff,webp";
inline const char kTypeSuffixMuz[] = "au,snd,mid,mp3,aif,aifc,aiff,m3u,ra,ram,wav,cda,wma,ape,flac,aac";
inline const char kTypeSuffixVid[] = "avi,mov,mp4,mp2,mpa,mpg,mpeg,mpe,qt,rm,rmvb,mkv,asx,asf,flv,3gp,wmv,3g2";
inline const char kTypeSuffixApp[] = "desktop";
//inline const char kTypeMimeApp[] = "application/x-shellscript,application/x-desktop,application/x-executable";
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#    define InitSuffixTable(table, suffix)                                 \
        {                                                                  \
            QSet<QString> *tablePtr = const_cast<QSet<QString> *>(&table); \
            *tablePtr = tablePtr->fromList(QString(suffix).split(','));    \
        }
#else
static void InitSuffixTable(const QSet<QString> &table, const QString &suffix)
{
    QSet<QString> *tablePtr = const_cast<QSet<QString> *>(&table);
    if (!tablePtr)
        return;
    tablePtr->clear();
    QStringList list = suffix.split(',');
    for (const QString &item : list)
        tablePtr->insert(item);
}
#endif

TypeClassifierPrivate::TypeClassifierPrivate(TypeClassifier *qq)
    : q(qq)
{
    //todo(zy) 类型后缀支持可配置
    InitSuffixTable(docSuffix, kTypeSuffixDoc);
    InitSuffixTable(picSuffix, kTypeSuffixPic);
    InitSuffixTable(muzSuffix, kTypeSuffixMuz);
    InitSuffixTable(vidSuffix, kTypeSuffixVid);
    InitSuffixTable(appSuffix, kTypeSuffixApp);
    //InitSuffixTable(appMimeType, kTypeMimeApp);
}

TypeClassifierPrivate::~TypeClassifierPrivate()
{
}

TypeClassifier::TypeClassifier(QObject *parent)
    : FileClassifier(parent), d(new TypeClassifierPrivate(this))
{
    {
        QHash<QString, QString> *namePtr = const_cast<QHash<QString, QString> *>(&d->keyNames);
        *namePtr = {
            { kTypeKeyApp, tr("Apps") },
            { kTypeKeyDoc, tr("Documents") },
            { kTypeKeyPic, tr("Pictures") },
            { kTypeKeyVid, tr("Videos") },
            { kTypeKeyMuz, tr("Music") },
            { kTypeKeyFld, tr("Folders") },
            { kTypeKeyOth, tr("Other") }
        };
    }

    // all datas shoud be accepted.
    auto filter = new GeneralModelFilter();
    filter->installFilter(this);
    handler = filter;

    // get enable items
    d->categories = CfgPresenter->enabledTypeCategories();
    fmInfo() << "enabled categories" << ItemCategories::Int(d->categories);
}

TypeClassifier::~TypeClassifier()
{
    delete d;
    d = nullptr;

    delete handler;
    handler = nullptr;
}

Classifier TypeClassifier::mode() const
{
    return Classifier::kType;
}

ModelDataHandler *TypeClassifier::dataHandler() const
{
    return handler;
}

QStringList TypeClassifier::classes() const
{
    // classes
    QStringList usedKey;
    if (d->categories == kCatNone) {
        // nothing to do.
    } else if (d->categories == kCatDefault) {
        // append category in order.
        usedKey.append(kTypeKeyDoc);
        usedKey.append(kTypeKeyPic);
        usedKey.append(kTypeKeyVid);
        usedKey.append(kTypeKeyMuz);
        usedKey.append(kTypeKeyFld);
        // not others default
    } else {
        // test enabled category.
        for (int i = kCatApplication; i <= kCatEnd; i = i << 1) {
            auto cat = static_cast<ItemCategory>(i);
            if (d->categories.testFlag(cat)) {
                Q_ASSERT(kCategory2Key.contains(cat));
                auto key = kCategory2Key.value(cat);
                if (d->keyNames.contains(key)) {
                    Q_ASSERT(!usedKey.contains(key));
                    usedKey.append(key);
                }
            }
        }
    }

    return usedKey;
}

QString TypeClassifier::classify(const QUrl &url) const
{
    auto itemInfo = InfoFactory::create<FileInfo>(url);
    if (!itemInfo)
        return QString();   // must return null string to represent the file is not existed.

    QString key;
    //Classify whether it is a symlink according to the symlink's target
    int depth = 3;
    while (depth--) {
        if (itemInfo->isAttributes(OptInfoType::kIsSymLink)) {
            QUrl fileUrl = itemInfo->urlOf(UrlInfoType::kRedirectedFileUrl);
            itemInfo = InfoFactory::create<FileInfo>(fileUrl);
            if (!itemInfo)
                return kTypeKeyOth;
            if (itemInfo->isAttributes(OptInfoType::kIsSymLink))
                continue;
        }
    }

    if (itemInfo->isAttributes(OptInfoType::kIsDir)) {
        key = kTypeKeyFld;
    } else {
        // classified by suffix.
        const QString &suffix = itemInfo->nameOf(NameInfoType::kSuffix).toLower();
        if (d->docSuffix.contains(suffix))
            key = kTypeKeyDoc;
        else if (d->appSuffix.contains(suffix))
            key = kTypeKeyApp;
        else if (d->vidSuffix.contains(suffix))
            key = kTypeKeyVid;
        else if (d->picSuffix.contains(suffix))
            key = kTypeKeyPic;
        else if (d->muzSuffix.contains(suffix))
            key = kTypeKeyMuz;
    }

    // set it to other if it not belong to any category
    // if its category is disabled. use: `d->categories.testFlag(d->categoryKey.key(key)`
    if (key.isEmpty())
        key = kTypeKeyOth;
    return key;
}

QString TypeClassifier::className(const QString &key) const
{
    return d->keyNames.value(key);
}

bool TypeClassifier::updateClassifier()
{
    const auto tmp = d->categories;
    d->categories = CfgPresenter->enabledTypeCategories();
    return tmp != d->categories;
}

QString TypeClassifier::replace(const QUrl &oldUrl, const QUrl &newUrl)
{
    if (!classes().contains(classify(newUrl)))
        return classify(newUrl);
    return FileClassifier::replace(oldUrl, newUrl);
}

QString TypeClassifier::append(const QUrl &url)
{
    if (!classes().contains(classify(url)))
        return classify(url);
    return FileClassifier::append(url);
}

QString TypeClassifier::prepend(const QUrl &url)
{
    if (!classes().contains(classify(url)))
        return classify(url);
    return FileClassifier::prepend(url);
}

QString TypeClassifier::remove(const QUrl &url)
{
    // 当此接口被调用时，文件可能已经被真正的移除了
    // 因此无法通过创建文件信息判断类型
    return FileClassifier::remove(url);
}

QString TypeClassifier::change(const QUrl &url)
{
    if (!classes().contains(classify(url)))
        return classify(url);
    return FileClassifier::change(url);
}

bool TypeClassifier::acceptRename(const QUrl &oldUrl, const QUrl &newUrl)
{
    if (!CfgPresenter->organizeOnTriggered())
        return FileClassifier::acceptRename(oldUrl, newUrl);

    QList<QUrl> collected;
    for (auto base : baseData())
        collected.append(base->items);
    if (collected.contains(newUrl)) {
        if (!collected.contains(oldUrl)) {
            // oldUrl not in collection (e.g., temporary file .EmvVrz), newUrl in collection
            // This is a file content replacement (save operation), not file overwrite
            // Keep the file in collection without removing it
            return true;
        }
        // Both oldUrl and newUrl are in collection
        // This is a real file overwrite scenario (e.g., mv a.txt b.txt, both exist)
        // Remove newUrl before processing the rename
        remove(newUrl);
        return true;
    } else if (collected.contains(oldUrl)) {
        return true;
    }
    return false;
}
