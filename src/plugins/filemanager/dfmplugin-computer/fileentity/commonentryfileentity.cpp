// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "commonentryfileentity.h"
#include "watcher/computeritemwatcher.h"

namespace dfmplugin_computer {

DFMBASE_USE_NAMESPACE

/*!
 * \brief 基于插件的配置信息来反射出对象，目的是为了更快的显示出计算机的 Item。
 * 而不用等待具体插件被加载
 * \param url
 */
CommonEntryFileEntity::CommonEntryFileEntity(const QUrl &url)
    : AbstractEntryFileEntity(url)
{
    const auto &infos { ComputerItemWatcher::instance()->getComputerInfos() };
    if (infos.contains(url)) {
        const auto &map { infos.value(url) };
        reflectionObjName = map.value("ReflectionObject").toString();
        defaultName = QObject::tr(qPrintable(map.value("ItemName").toString()));
        defualtIcon = QIcon::fromTheme(map.value("ItemIcon").toString());
    }
}

CommonEntryFileEntity::~CommonEntryFileEntity()
{
    if (reflectionObj) {
        delete reflectionObj;
        reflectionObj = nullptr;
    }
}

QString CommonEntryFileEntity::displayName() const
{
    if (!defaultName.isEmpty())
        return defaultName;
    if (reflection() && hasMethod("displayName")) {
        QString name;
        bool ret { QMetaObject::invokeMethod(reflectionObj, "displayName",
                                             Qt::DirectConnection, Q_RETURN_ARG(QString, name)) };
        if (ret)
            return name;
    }
    return "";
}

QIcon CommonEntryFileEntity::icon() const
{
    if (!defualtIcon.isNull())
        return defualtIcon;
    if (reflection() && hasMethod("icon")) {
        QIcon theIcon;
        bool ret { QMetaObject::invokeMethod(reflectionObj, "icon",
                                             Qt::DirectConnection, Q_RETURN_ARG(QIcon, theIcon)) };
        if (ret)
            return theIcon;
    }
    return QIcon();
}

bool CommonEntryFileEntity::exists() const
{
    if (reflection() && hasMethod("exists")) {
        bool isExists;
        bool ret { QMetaObject::invokeMethod(reflectionObj, "exists",
                                             Qt::DirectConnection, Q_RETURN_ARG(bool, isExists)) };
        if (ret)
            return isExists;
    }
    return false;
}

bool CommonEntryFileEntity::showProgress() const
{
    if (reflection() && hasMethod("showProgress")) {
        bool show;
        bool ret { QMetaObject::invokeMethod(reflectionObj, "showProgress",
                                             Qt::DirectConnection, Q_RETURN_ARG(bool, show)) };
        if (ret)
            return show;
    }
    return false;
}

bool CommonEntryFileEntity::showTotalSize() const
{
    if (reflection() && hasMethod("showTotalSize")) {
        bool show;
        bool ret { QMetaObject::invokeMethod(reflectionObj, "showTotalSize",
                                             Qt::DirectConnection, Q_RETURN_ARG(bool, show)) };
        if (ret)
            return show;
    }
    return false;
}

bool CommonEntryFileEntity::showUsageSize() const
{
    if (reflection() && hasMethod("showUsageSize")) {
        bool show;
        bool ret { QMetaObject::invokeMethod(reflectionObj, "showUsageSize",
                                             Qt::DirectConnection, Q_RETURN_ARG(bool, show)) };
        if (ret)
            return show;
    }
    return false;
}

AbstractEntryFileEntity::EntryOrder CommonEntryFileEntity::order() const
{
    if (reflection() && hasMethod("order")) {
        AbstractEntryFileEntity::EntryOrder theOrder;
        bool ret { QMetaObject::invokeMethod(reflectionObj, "order", Qt::DirectConnection,
                                             Q_RETURN_ARG(AbstractEntryFileEntity::EntryOrder, theOrder)) };
        if (ret)
            return theOrder;
    }
    return AbstractEntryFileEntity::EntryOrder::kOrderCustom;
}

void CommonEntryFileEntity::refresh()
{
    if (reflection() && hasMethod("refresh")) {
        bool ret { QMetaObject::invokeMethod(reflectionObj, "refresh") };
        if (ret)
            return;
    }
    return AbstractEntryFileEntity::refresh();
}

quint64 CommonEntryFileEntity::sizeTotal() const
{
    if (reflection() && hasMethod("sizeTotal")) {
        quint64 size;
        bool ret { QMetaObject::invokeMethod(reflectionObj, "sizeTotal",
                                             Qt::DirectConnection, Q_RETURN_ARG(quint64, size)) };
        if (ret)
            return size;
    }
    return AbstractEntryFileEntity::sizeTotal();
}

quint64 CommonEntryFileEntity::sizeUsage() const
{
    if (reflection() && hasMethod("sizeUsage")) {
        quint64 size;
        bool ret { QMetaObject::invokeMethod(reflectionObj, "sizeUsage",
                                             Qt::DirectConnection, Q_RETURN_ARG(quint64, size)) };
        if (ret)
            return size;
    }
    return AbstractEntryFileEntity::sizeUsage();
}

QString CommonEntryFileEntity::description() const
{
    if (reflection() && hasMethod("description")) {
        QString desc;
        bool ret { QMetaObject::invokeMethod(reflectionObj, "description",
                                             Qt::DirectConnection, Q_RETURN_ARG(QString, desc)) };
        if (ret)
            return desc;
    }
    return AbstractEntryFileEntity::description();
}

QUrl CommonEntryFileEntity::targetUrl() const
{
    if (reflection() && hasMethod("targetUrl")) {
        QUrl url;
        bool ret { QMetaObject::invokeMethod(reflectionObj, "targetUrl",
                                             Qt::DirectConnection, Q_RETURN_ARG(QUrl, url)) };
        if (ret)
            return url;
    }
    return AbstractEntryFileEntity::targetUrl();
}

bool CommonEntryFileEntity::isAccessable() const
{
    if (reflection() && hasMethod("isAccessable")) {
        bool accessable { false };
        bool ret { QMetaObject::invokeMethod(reflectionObj, "isAccessable",
                                             Qt::DirectConnection, Q_RETURN_ARG(bool, accessable)) };
        if (ret)
            return accessable;
    }
    return AbstractEntryFileEntity::isAccessable();
}

bool CommonEntryFileEntity::renamable() const
{
    if (reflection() && hasMethod("renamable")) {
        bool isRenamable { false };
        bool ret { QMetaObject::invokeMethod(reflectionObj, "renamable",
                                             Qt::DirectConnection, Q_RETURN_ARG(bool, isRenamable)) };
        if (ret)
            return isRenamable;
    }
    return AbstractEntryFileEntity::renamable();
}

QVariantHash CommonEntryFileEntity::extraProperties() const
{
    if (reflection() && hasMethod("extraProperties")) {
        QVariantHash properties;
        bool ret { QMetaObject::invokeMethod(reflectionObj, "extraProperties",
                                             Qt::DirectConnection, Q_RETURN_ARG(QVariantHash, properties)) };
        if (ret)
            return properties;
    }
    return AbstractEntryFileEntity::extraProperties();
}

void CommonEntryFileEntity::setExtraProperty(const QString &key, const QVariant &val)
{
    if (reflection() && hasMethod("setExtraProperty")) {
        bool ret { QMetaObject::invokeMethod(reflectionObj, "setExtraProperty",
                                             Q_ARG(QString, key), Q_ARG(QVariant, val)) };
        if (ret)
            return;
    }

    return AbstractEntryFileEntity::setExtraProperty(key, val);
}

bool CommonEntryFileEntity::reflection() const
{
    if (reflectionObj)
        return true;
    int type = QMetaType::type(reflectionObjName.toLocal8Bit().data());
    if (type == QMetaType::UnknownType) {
        fmWarning() << "Unknown meta type for reflection object:" << reflectionObjName;
        return false;
    }

    auto metaObj = QMetaType::metaObjectForType(type);
    if (!metaObj) {
        fmWarning() << "No meta object found for reflection type:" << reflectionObjName;
        return false;
    }

    reflectionObj = metaObj->newInstance();
    return reflectionObj;
}

bool CommonEntryFileEntity::hasMethod(const QString &methodName) const
{
    if (!reflectionObj) {
        fmDebug() << "No reflection object available for method check:" << methodName;
        return false;
    }

    QString fullMethod { methodName + "()" };
    return reflectionObj->metaObject()->indexOfMethod(fullMethod.toLocal8Bit().data()) > 0;
}

}   // end namespace dfmplugin_computer
