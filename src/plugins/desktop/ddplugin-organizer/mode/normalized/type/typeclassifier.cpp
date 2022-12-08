/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "typeclassifier_p.h"
#include "models/generalmodelfilter.h"
#include "config/configpresenter.h"

#include "file/local/localfileinfo.h"
#include "base/schemefactory.h"

using namespace ddplugin_organizer;
DFMBASE_USE_NAMESPACE

namespace {
inline const char kTypeKeyApp[] = "Type_Apps";
inline const char kTypeKeyDoc[] = "Type_Documents";
inline const char kTypeKeyPic[] = "Type_Pictures";
inline const char kTypeKeyVid[] = "Type_Videos";
inline const char kTypeKeyMuz[] = "Type_Music";
inline const char kTypeKeyFld[] = "Type_Folders";
inline const char kTypeKeyOth[] = "Type_Other";

inline const char kTypeSuffixDoc[] = "pdf,txt,doc,docx,dot,dotx,ppt,pptx,pot,potx,xls,xlsx,xlt,xltx,wps,wpt,rtf,md,latex";
inline const char kTypeSuffixPic[] = "jpg,jpeg,jpe,bmp,png,gif,svg,tif,tiff";
inline const char kTypeSuffixMuz[] = "au,snd,mid,mp3,aif,aifc,aiff,m3u,ra,ram,wav,cda,wma,ape";
inline const char kTypeSuffixVid[] = "avi,mov,mp4,mp2,mpa,mpg,mpeg,mpe,qt,rm,rmvb,mkv,asx,asf,flv,3gp";
inline const char kTypeSuffixApp[] = "desktop";
//inline const char kTypeMimeApp[] = "application/x-shellscript,application/x-desktop,application/x-executable";
}

#define InitSuffixTable(table, suffix)                                 \
    {                                                                  \
        QSet<QString> *tablePtr = const_cast<QSet<QString> *>(&table); \
        *tablePtr = tablePtr->fromList(QString(suffix).split(','));    \
    }
TypeClassifierPrivate::TypeClassifierPrivate(TypeClassifier *qq)
    : q(qq) {
          //todo(zy) 类型后缀支持可配置
          InitSuffixTable(docSuffix, kTypeSuffixDoc)
                  InitSuffixTable(picSuffix, kTypeSuffixPic)
                          InitSuffixTable(muzSuffix, kTypeSuffixMuz)
                                  InitSuffixTable(vidSuffix, kTypeSuffixVid)
                                          InitSuffixTable(appSuffix, kTypeSuffixApp)
          //InitSuffixTable(appMimeType, kTypeMimeApp)
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
    {
        QHash<ItemCategory, QString> *categoryPtr = const_cast<QHash<ItemCategory, QString> *>(&d->categoryKey);
        *categoryPtr = {
            { kCatApplication, kTypeKeyApp },
            { kCatDocument, kTypeKeyDoc },
            { kCatPicture, kTypeKeyPic },
            { kCatVideo, kTypeKeyVid },
            { kCatMusic, kTypeKeyMuz },
            { kCatFloder, kTypeKeyFld }
        };
    }
    // all datas shoud be accepted.
    handler = new GeneralModelFilter();

    // get enable items
    d->categories = CfgPresenter->enabledTypeCategories();
    qInfo() << "enabled categories" << ItemCategories::Int(d->categories);
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
    } else if (d->categories == kCatAll) {
        // append category in order.
        usedKey.append(kTypeKeyApp);
        usedKey.append(kTypeKeyDoc);
        usedKey.append(kTypeKeyPic);
        usedKey.append(kTypeKeyVid);
        usedKey.append(kTypeKeyMuz);
        usedKey.append(kTypeKeyFld);
    } else {
        // test enabled category.
        for (int i = kCatApplication; i <= kCatEnd; i = i << 1) {
            auto cat = static_cast<ItemCategory>(i);
            if (d->categories.testFlag(cat)) {
                auto key = d->categoryKey.value(cat);
                if (d->keyNames.contains(key)) {
                    Q_ASSERT(!usedKey.contains(key));
                    usedKey.append(key);
                }
            }
        }
    }

    // the `other` is a fixed item.
    usedKey.append(kTypeKeyOth);
    return usedKey;
}

QString TypeClassifier::classify(const QUrl &url) const
{
    auto itemInfo = InfoFactory::create<LocalFileInfo>(url);
    if (!itemInfo)
        return QString();   // must return null string to represent the file is not existed.

    QString key;
    //Classify whether it is a symlink according to the symlink's target
    if (itemInfo->isAttributes(AbstractFileInfo::FileIsType::kIsSymLink)) {
        QUrl fileUrl = itemInfo->urlInfo(AbstractFileInfo::FileUrlInfoType::kRedirectedFileUrl);
        itemInfo = InfoFactory::create<LocalFileInfo>(fileUrl);
        if (itemInfo->isAttributes(AbstractFileInfo::FileIsType::kIsSymLink)) {
            key = kTypeKeyOth;
            return key;
        }
    }

    if (itemInfo->isAttributes(AbstractFileInfo::FileIsType::kIsDir)) {
        key = kTypeKeyFld;
    } else {
        // classified by suffix.
        const QString &suffix = itemInfo->nameInfo(NameInfo::kSuffix).toLower();
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

    // set it to other if it not belong to any category or its category is disabled.
    if (key.isEmpty() || !d->categories.testFlag(d->categoryKey.key(key)))
        key = kTypeKeyOth;
    return key;
}

QString TypeClassifier::className(const QString &key) const
{
    return d->keyNames.value(key);
}
