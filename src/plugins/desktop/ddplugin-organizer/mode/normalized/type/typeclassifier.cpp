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
#include "models/modeldatahandler.h"

#include "file/local/localfileinfo.h"
#include "base/schemefactory.h"

DDP_ORGANIZER_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

namespace  {
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

#define InitSuffixTable(table, suffix) \
        {\
            QSet<QString> *tablePtr = const_cast<QSet<QString> *>(&table);\
            *tablePtr = tablePtr->fromList(QString(suffix).split(','));\
        }
TypeClassifierPrivate::TypeClassifierPrivate(TypeClassifier *qq) : q(qq)
{
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
    : FileClassifier(parent)
    , d(new TypeClassifierPrivate(this))
{
    QHash<QString, QString> *namePtr = const_cast<QHash<QString, QString> *>(&d->keyNames);
    *namePtr = {
            {kTypeKeyApp, tr("Apps")},
            {kTypeKeyDoc, tr("Documents")},
            {kTypeKeyPic, tr("Pictures")},
            {kTypeKeyVid, tr("Videos")},
            {kTypeKeyMuz, tr("Music")},
            {kTypeKeyFld, tr("Folders")},
            {kTypeKeyOth, tr("Other")}
        };
    // all datas shoud be accepted.
    handler = new ModelDataHandler();
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
    QStringList usedKey = d->keyNames.keys();
    // order it
    const QMap<QString, int> fixOrder = {
        {kTypeKeyApp, 0},
        {kTypeKeyDoc, 1},
        {kTypeKeyPic, 2},
        {kTypeKeyVid, 3},
        {kTypeKeyMuz, 4},
        {kTypeKeyFld, 5},
        {kTypeKeyOth, 6}
    };
    const int max = fixOrder.size();
    std::sort(usedKey.begin(), usedKey.end(), [&fixOrder, max](const QString &t1, const QString &t2){
        return fixOrder.value(t1, max) < fixOrder.value(t2, max);
    });
    return usedKey;
}

QString TypeClassifier::classify(const QUrl &url) const
{
    auto itemInfo = InfoFactory::create<LocalFileInfo>(url);
    if (!itemInfo)
        return QString();

    if (itemInfo->isDir())
        return kTypeKeyFld;

    // classified by suffix.
    const QString &suffix = itemInfo->suffix().toLower();
    if (d->docSuffix.contains(suffix))
        return kTypeKeyDoc;
    else if (d->appSuffix.contains(suffix))
        return kTypeKeyApp;
    else if (d->vidSuffix.contains(suffix))
        return kTypeKeyVid;
    else if (d->picSuffix.contains(suffix))
        return kTypeKeyPic;
    else if (d->muzSuffix.contains(suffix))
        return kTypeKeyMuz;

#if 0
     // classified by mimetype.
//    const QString &mimeType = itemInfo->mimeTypeName();
//    if (d->appMimeType.contains(mimeType))
//        return kTypeKeyApp;
//    else if (mimeType.startsWith("video/"))
//        return kTypeKeyVid;
//    else if (mimeType.startsWith("image/"))
//        return kTypeKeyPic;
//    else if (mimeType.startsWith("audio/"))
//        return kTypeKeyMuz;
#endif

    return kTypeKeyOth;
}

QString TypeClassifier::className(const QString &key) const
{
    return d->keyNames.value(key);
}
