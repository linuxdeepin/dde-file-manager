// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/standardpaths.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/fileutils.h>

#include <QCoreApplication>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QThread>

/*!
 * \class SettingsPrivate 通用设置的私有类
 * \brief The SettingsPrivate class 保存类Settings的所有数据和成员变量
 */
namespace dfmbase {

class SettingsPrivate
{
public:
    explicit SettingsPrivate(Settings *qq);

    bool autoSync = false;   // automatically synchronize
    bool watchChanges = false;   //monitor for configuration changes
    bool settingFileIsDirty = false;   // set whether the file has cached data (dirty data)
    QSet<QString> autoSyncGroupExclude;   // when auto sync, exclude some group
    QTimer *syncTimer = nullptr;   // synchronization Timer
    QString fallbackFile;   // backup settings file path
    QString settingFile;   // set the file path
    AbstractFileWatcherPointer settingWatcher;   // watch file changed
    Settings *q;

    struct Data
    {
        QHash<QString, QVariantHash> values;   // Set the file's configuration property hash table
        QHash<QString, QVariantHash> privateValues;   // set the file's configuration private attribute hash table
        /*!
         * \brief value 获取相应的属性
         *
         * \param group 哪一个组
         *
         * \param key 键值key
         *
         * \param dv 默认的返回属性
         *
         * \return QVariant 属性值
         */
        QVariant value(const QString &group, const QString &key, const QVariant &dv = QVariant()) const
        {
            return values.value(group).value(key, dv);
        }
        /*!
         * \brief setValue 设置相应的属性值
         *
         * \param group 哪一个组
         *
         * \param key 键值key
         *
         * \param value 属性值
         */
        void setValue(const QString group, const QString &key, const QVariant &value)
        {
            if (!values.contains(group)) {
                values.insert(group, { { key, value } });

                return;
            }

            values[group][key] = value;
        }
        /*!
         * \brief groupMetaData 获取一组的属性
         *
         * \param group 组名
         *
         * \return QVariantMap 这一组的属性
         */
        QVariantMap groupMetaData(const QString &group) const
        {
            return privateValues.value("__metadata__").value(group).toMap();
        }
        /*!
         * \brief groupKeyOrderedList 获取组属性的key序列表
         *
         * \param group 组名
         *
         * \return QStringList key序列表
         */
        QStringList groupKeyOrderedList(const QString &group) const
        {
            return groupMetaData(group).value("keyOrdered").toStringList();
        }
    };

    Data defaultData;   // 默认的属性data
    Data fallbackData;   // 备份的属性data
    Data writableData;   // 写入的属性data

    void fromJsonFile(const QString &fileName, Data *data);
    void fromJson(const QByteArray &json, Data *data);
    QByteArray toJson(const Data &data);

    /*!
     * \brief makeSettingFileToDirty 同步设置到配置文件
     * \param dirty 是否是脏数据
     */
    void makeSettingFileToDirty(bool dirty)
    {
        if (settingFileIsDirty == dirty) {
            return;
        }

        settingFileIsDirty = dirty;

        if (!autoSync) {
            return;
        }

        Q_ASSERT(syncTimer);

        if (QThread::currentThread() == syncTimer->thread()) {
            if (dirty) {
                syncTimer->start();
            } else {
                syncTimer->stop();
            }
        } else {
            syncTimer->metaObject()->invokeMethod(syncTimer, dirty ? "start" : "stop", Qt::QueuedConnection);
        }
    }
    /*!
     * \brief urlToKey url转换为key值
     *
     * \param url 文件的url
     *
     * \return QString key值
     */
    QString urlToKey(const QUrl &url) const
    {
        if (dfmbase::FileUtils::isLocalFile(url)) {
            const QUrl &new_url = StandardPaths::toStandardUrl(url.toLocalFile());

            if (new_url.isValid()) {
                return new_url.toString();
            }
        }

        return url.toString();
    }

    void _q_onFileChanged(const QUrl &url);
};

SettingsPrivate::SettingsPrivate(Settings *qq)
    : q(qq)
{
}
/*!
 * \brief SettingsPrivate::fromJsonFile 从json文件中读取属性到data中
 *
 * \param fileName 配置文件名称
 *
 * \param data 输出参数data，属性
 */
void SettingsPrivate::fromJsonFile(const QString &fileName, Data *data)
{
    QFile file(fileName);

    if (!file.exists()) {
        return;
    }

    if (!file.open(QFile::ReadOnly)) {
        qCWarning(logDFMBase) << file.errorString();

        return;
    }

    const QByteArray &json = file.readAll();

    if (json.isEmpty()) {
        return;
    }

    fromJson(json, data);
}
/*!
 * \brief SettingsPrivate::fromJson 从json文件中读取data
 *
 * \param json json文件
 *
 * \param data 输出参数属性data
 */
void SettingsPrivate::fromJson(const QByteArray &json, Data *data)
{
    QJsonParseError error;
    const QJsonDocument &doc = QJsonDocument::fromJson(json, &error);

    if (error.error != QJsonParseError::NoError) {
        qCWarning(logDFMBase) << error.errorString();
        return;
    }

    if (!doc.isObject()) {
        qCWarning(logDFMBase) << QString();
        return;
    }

    const QJsonObject &groups_object = doc.object();

    for (auto begin = groups_object.constBegin(); begin != groups_object.constEnd(); ++begin) {
        const QJsonValue &value = begin.value();

        if (!value.isObject()) {
            qCWarning(logDFMBase) << QString();
            continue;
        }

        const QJsonObject &value_object = value.toObject();
        QVariantHash hash;

        for (auto iter = value_object.constBegin(); iter != value_object.constEnd(); ++iter) {
            hash[iter.key()] = iter.value().toVariant();
        }

        // private groups
        if (begin.key().startsWith("__") && begin.key().endsWith("__"))
            data->privateValues[begin.key()] = hash;
        else
            data->values[begin.key()] = hash;
    }
}
/*!
 * \brief SettingsPrivate::toJson 将属性转换为Json对象的QByteArray
 *
 * \param data 属性data
 *
 * \return QByteArray json 的对象
 */
QByteArray SettingsPrivate::toJson(const Data &data)
{
    QJsonObject root_object;

    for (auto begin = data.values.constBegin(); begin != data.values.constEnd(); ++begin) {
        const QString &key = begin.key();
        if (!autoSyncGroupExclude.contains(key))
            root_object.insert(key, QJsonValue(QJsonObject::fromVariantHash(begin.value())));
    }

    return QJsonDocument(root_object).toJson();
}
/*!
 * \brief SettingsPrivate::_q_onFileChanged 槽函数，当配置文件发上改变时调用
 *
 * \param url 文件改变的url
 */
void SettingsPrivate::_q_onFileChanged(const QUrl &url)
{
    if (url.toLocalFile() != settingFile) {
        return;
    }

    const auto old_values = writableData.values;

    writableData.values.clear();
    fromJsonFile(settingFile, &writableData);
    makeSettingFileToDirty(false);

    for (auto begin = writableData.values.constBegin(); begin != writableData.values.constEnd(); ++begin) {
        for (auto i = begin.value().constBegin(); i != begin.value().constEnd(); ++i) {
            if (old_values.value(begin.key()).contains(i.key())) {
                if (old_values.value(begin.key()).value(i.key()) == i.value()) {
                    continue;
                }
            } else {
                if (fallbackData.values.value(begin.key()).contains(i.key())) {
                    if (fallbackData.values.value(begin.key()).value(i.key()) == i.value()) {
                        continue;
                    }
                }

                if (defaultData.values.value(begin.key()).value(i.key()) == i.value()) {
                    continue;
                }
            }

            Q_EMIT q->valueEdited(begin.key(), i.key(), i.value());
            Q_EMIT q->valueChanged(begin.key(), i.key(), i.value());
        }
    }

    for (auto begin = old_values.constBegin(); begin != old_values.constEnd(); ++begin) {
        for (auto i = begin.value().constBegin(); i != begin.value().constEnd(); ++i) {
            if (writableData.values.value(begin.key()).contains(i.key())) {
                continue;
            }

            const QVariant &new_value = q->value(begin.key(), i.key());

            if (new_value != old_values.value(begin.key()).value(i.key())) {
                Q_EMIT q->valueEdited(begin.key(), i.key(), new_value);
                Q_EMIT q->valueChanged(begin.key(), i.key(), new_value);
            }
        }
    }
}

/*!
 * \class Settings
 *
 * \brief Settings provide interfaces to access and modify the file manager setting options.
 */
Settings::Settings(const QString &defaultFile, const QString &fallbackFile, const QString &settingFile, QObject *parent)
    : QObject(parent), d(new SettingsPrivate(this))
{
    d->fallbackFile = fallbackFile;
    d->settingFile = settingFile;

    d->fromJsonFile(defaultFile, &d->defaultData);
    d->fromJsonFile(fallbackFile, &d->fallbackData);
    d->fromJsonFile(settingFile, &d->writableData);
}
/*!
 * \brief getConfigFilePath 获取配置文件的路径
 *
 * \param type StandardLocation的类型
 *
 * \param fileName 文件名称
 *
 * \param writable 是否可写
 *
 * \return QString 配置文件的路径
 */
static QString getConfigFilePath(QStandardPaths::StandardLocation type, const QString &fileName, bool writable)
{
    if (writable) {
        QString path = QStandardPaths::writableLocation(type);

        if (path.isEmpty()) {
            path = QDir::home().absoluteFilePath(QString(".config/%1/%2").arg(qApp->organizationName()).arg(qApp->applicationName()));
        }

        return path.append(QString("/%1.json").arg(fileName));
    }

    const QStringList &list = QStandardPaths::standardLocations(type);

    QString path = list.isEmpty() ? QString("/etc/xdg/%1/%2").arg(qApp->organizationName()).arg(qApp->applicationName()) : list.last();

    return path.append(QString("/%1.json").arg(fileName));
}

Settings::Settings(const QString &name, ConfigType type, QObject *parent)
    : Settings(QString(":/config/%1.json").arg(name),
               getConfigFilePath(type == kAppConfig
                                         ? QStandardPaths::AppConfigLocation
                                         : QStandardPaths::GenericConfigLocation,
                                 name, false),
               getConfigFilePath(type == kAppConfig
                                         ? QStandardPaths::AppConfigLocation
                                         : QStandardPaths::GenericConfigLocation,
                                 name, true),
               parent)
{
}

Settings::~Settings()
{
    if (d->syncTimer) {
        d->syncTimer->stop();
    }

    if (d->settingFileIsDirty) {
        sync();
    }
}
/*!
 * \brief Settings::contains 判断是否包含这个键值的属性
 *
 * \param group 组名
 *
 * \param key key键值
 *
 * \return bool 是否包含这个key值
 */
bool Settings::contains(const QString &group, const QString &key) const
{
    if (key.isEmpty()) {
        if (d->writableData.values.contains(group)) {
            return true;
        }

        if (d->fallbackData.values.contains(group)) {
            return true;
        }

        return d->defaultData.values.contains(group);
    }

    if (d->writableData.values.value(group).contains(key)) {
        return true;
    }

    if (d->fallbackData.values.value(group).contains(key)) {
        return true;
    }

    return d->defaultData.values.value(group).contains(key);
}
/*!
 * \brief Settings::groups 获取属性有哪些组
 *
 * \return QSet<QString> 组的集合
 */
QSet<QString> Settings::groups() const
{
    QSet<QString> groups;

    groups.reserve(d->writableData.values.size() + d->fallbackData.values.size() + d->defaultData.values.size());

    for (auto begin = d->writableData.values.constBegin(); begin != d->writableData.values.constEnd(); ++begin) {
        groups << begin.key();
    }

    for (auto begin = d->fallbackData.values.constBegin(); begin != d->fallbackData.values.constEnd(); ++begin) {
        groups << begin.key();
    }

    for (auto begin = d->defaultData.values.constBegin(); begin != d->defaultData.values.constEnd(); ++begin) {
        groups << begin.key();
    }

    return groups;
}
/*!
 * \brief Settings::keys 获取某组中的所有key键值的集合
 *
 * \param group 组名
 *
 * \return QSet<QString> key键值的集合
 */
QSet<QString> Settings::keys(const QString &group) const
{
    QSet<QString> keys;

    const auto &&wg = d->writableData.values.value(group);
    const auto &&fg = d->fallbackData.values.value(group);
    const auto &&dg = d->defaultData.values.value(group);

    keys.reserve(wg.size() + fg.size() + dg.size());

    for (auto begin = wg.constBegin(); begin != wg.constEnd(); ++begin) {
        keys << begin.key();
    }

    for (auto begin = fg.constBegin(); begin != fg.constEnd(); ++begin) {
        keys << begin.key();
    }

    for (auto begin = dg.constBegin(); begin != dg.constEnd(); ++begin) {
        keys << begin.key();
    }

    return keys;
}

/*!
 * \brief Settings::keysList
 *
 * \param group name
 *
 * \return An ordered key list of the group
 */
QStringList Settings::keyList(const QString &group) const
{
    QStringList keyList;
    QSet<QString> keys = this->keys(group);

    for (const QString &ordered_key : d->defaultData.groupKeyOrderedList(group)) {
        if (keys.contains(ordered_key)) {
            keyList << ordered_key;
            keys.remove(ordered_key);
        }
    }

    for (const QString &ordered_key : d->fallbackData.groupKeyOrderedList(group)) {
        if (keys.contains(ordered_key)) {
            keyList << ordered_key;
            keys.remove(ordered_key);
        }
    }

    for (const QString &ordered_key : d->writableData.groupKeyOrderedList(group)) {
        if (keys.contains(ordered_key)) {
            keyList << ordered_key;
            keys.remove(ordered_key);
        }
    }

#if (QT_VERSION <= QT_VERSION_CHECK(5, 14, 0))
    keyList << keys.toList();
#else
    keyList << keys.values();
#endif

    return keyList;
}

QStringList Settings::defaultConfigkeyList(const QString &group) const
{
    QStringList keyList;
    QSet<QString> keys = this->keys(group);

    for (const QString &ordered_key : d->defaultData.groupKeyOrderedList(group)) {
        if (keys.contains(ordered_key)) {
            keyList << ordered_key;
            keys.remove(ordered_key);
        }
    }

#if (QT_VERSION <= QT_VERSION_CHECK(5, 14, 0))
    keyList << keys.toList();
#else
    keyList << keys.values();
#endif

    return keyList;
}
/*!
 * \brief Settings::toUrlValue 从QVariant转换为QUrl
 *
 * \param url 属性值
 *
 * \return QUrl 文件的URL
 */
QUrl Settings::toUrlValue(const QVariant &url)
{
    const QString &urlString = url.toString();

    if (urlString.isEmpty()) {
        return QUrl();
    }

    const QString &path = StandardPaths::fromStandardUrl(QUrl(urlString));

    if (!path.isEmpty()) {
        return QUrl::fromLocalFile(path);
    }

    return QUrl::fromUserInput(urlString);
}
/*!
 * \brief Settings::value 获取某个组的某个key对应的value，配置的某个属性
 *
 * \param group 组名
 *
 * \param key 键值
 *
 * \param defaultValue 返回的默认值
 *
 * \return QVariant 属性值
 */
QVariant Settings::value(const QString &group, const QString &key, const QVariant &defaultValue) const
{
    QVariant value = d->writableData.values.value(group).value(key);

    if (!value.isNull()) {
        return value;
    }

    value = d->fallbackData.values.value(group).value(key);

    if (!value.isNull()) {
        return value;
    }

    return d->defaultData.values.value(group).value(key, defaultValue);
}
/*!
 * \brief Settings::value 获取某个组的某个key对应的value，配置的某个属性
 *
 * \param group 组名
 *
 * \param key 键值
 *
 * \param defaultValue 返回的默认值
 *
 * \return QVariant 属性值
 */
QVariant Settings::value(const QString &group, const QUrl &key, const QVariant &defaultValue) const
{
    return value(group, d->urlToKey(key), defaultValue);
}

QVariant Settings::defaultConfigValue(const QString &group, const QString &key, const QVariant &defaultValue) const
{
    return d->defaultData.values.value(group).value(key, defaultValue);
}

QVariant Settings::defaultConfigValue(const QString &group, const QUrl &key, const QVariant &defaultValue) const
{
    return defaultConfigValue(group, d->urlToKey(key), defaultValue);
}
/*!
 * \brief Settings::urlValue 获取属性转换为对应文件的url
 *
 * \param group 组名
 *
 * \param key 键值QString
 *
 * \param defaultValue 返回的默认值
 *
 * \return QUrl 文件的URL
 */
QUrl Settings::urlValue(const QString &group, const QString &key, const QUrl &defaultValue) const
{
    return toUrlValue(value(group, key, defaultValue));
}
/*!
 * \brief Settings::urlValue 获取属性转换为对应文件的url
 *
 * \param group 组名
 *
 * \param key 键值url
 *
 * \param defaultValue 返回的默认值
 *
 * \return QUrl 文件的URL
 */
QUrl Settings::urlValue(const QString &group, const QUrl &key, const QUrl &defaultValue) const
{
    return urlValue(group, d->urlToKey(key), defaultValue);
}
/*!
 * \brief Settings::setValue 设置属性
 *
 * \param group 组名
 *
 * \param key 键值QString
 *
 * \param value 属性值
 */
void Settings::setValue(const QString &group, const QString &key, const QVariant &value)
{
    if (setValueNoNotify(group, key, value)) {
        Q_EMIT valueChanged(group, key, value);
    }
}
/*!
 * \brief Settings::setValue 设置属性
 *
 * \param group 组名
 *
 * \param key 键值QUrl
 *
 * \param value 属性值
 */
void Settings::setValue(const QString &group, const QUrl &key, const QVariant &value)
{
    setValue(group, d->urlToKey(key), value);
}
/*!
 * \brief Settings::setValueNoNotify 设置了属性以后的通知
 *
 * \param group 组名
 *
 * \param key 键值QString
 *
 * \param value 属性值
 *
 * \return bool 是否通知成功
 */
bool Settings::setValueNoNotify(const QString &group, const QString &key, const QVariant &value)
{
    bool changed = false;

    if (isRemovable(group, key)) {
        if (d->writableData.value(group, key) == value) {
            return false;
        }

        changed = true;
    } else {
        changed = this->value(group, key, value) != value;
    }

    d->writableData.setValue(group, key, value);
    d->makeSettingFileToDirty(true);

    return changed;
}
/*!
 * \brief Settings::setValueNoNotify 设置了属性以后的通知
 *
 * \param group 组名
 *
 * \param key 键值QUrl
 *
 * \param value 属性值
 *
 * \return bool 是否通知成功
 */
bool Settings::setValueNoNotify(const QString &group, const QUrl &key, const QVariant &value)
{
    return setValueNoNotify(group, d->urlToKey(key), value);
}
/*!
 * \brief Settings::removeGroup 移除某个组
 *
 * \param group 组名
 */
void Settings::removeGroup(const QString &group)
{
    if (!d->writableData.values.contains(group)) {
        return;
    }

    const QVariantHash &group_values = d->writableData.values.take(group);

    d->makeSettingFileToDirty(true);

    for (auto begin = group_values.constBegin(); begin != group_values.constEnd(); ++begin) {
        const QVariant &new_value = value(group, begin.key());

        if (new_value != begin.value()) {
            Q_EMIT valueChanged(group, begin.key(), new_value);
        }
    }
}
/*!
 * \brief Settings::isRemovable 属性是否移除
 *
 * \param group 组名
 *
 * \param key 键值QString
 *
 * \return bool 是否移除了
 */
bool Settings::isRemovable(const QString &group, const QString &key) const
{
    return d->writableData.values.value(group).contains(key);
}
/*!
 * \brief Settings::isRemovable 属性是否移除
 *
 * \param group 组名
 *
 * \param key 键值QUrl
 *
 * \return bool 是否移除了
 */
bool Settings::isRemovable(const QString &group, const QUrl &key) const
{
    return isRemovable(group, d->urlToKey(key));
}
/*!
 * \brief Settings::remove 移除某个属性
 *
 * \param group 组名
 *
 * \param key 键值
 */
void Settings::remove(const QString &group, const QString &key)
{
    if (!d->writableData.values.value(group).contains(key)) {
        return;
    }

    const QVariant &old_value = d->writableData.values[group].take(key);
    d->makeSettingFileToDirty(true);

    const QVariant &new_value = value(group, key);

    if (old_value == new_value) {
        return;
    }

    Q_EMIT valueChanged(group, key, new_value);
}
/*!
 * \brief Settings::remove 移除某个属性
 *
 * \param group 组名
 *
 * \param key 键值QUrl
 */
void Settings::remove(const QString &group, const QUrl &key)
{
    remove(group, d->urlToKey(key));
}
/*!
 * \brief Settings::clear 清理所有的属性，并保存
 */
void Settings::clear()
{
    if (d->writableData.values.isEmpty()) {
        return;
    }

    const QHash<QString, QVariantHash> old_values = d->writableData.values;

    d->writableData.values.clear();
    d->makeSettingFileToDirty(true);

    for (auto begin = old_values.constBegin(); begin != old_values.constEnd(); ++begin) {
        const QVariantHash &values = begin.value();

        for (auto i = values.constBegin(); i != values.constEnd(); ++i) {
            const QVariant &new_value = value(begin.key(), i.key());

            if (new_value != i.value()) {
                Q_EMIT valueChanged(begin.key(), i.key(), new_value);
            }
        }
    }
}

/*!
 * \brief Reload config file.
 *
 * This will be needed if file watcher is disabled, or say, you defined the
 *
 * DFM_NO_FILE_WATCHER marco.
 */
void Settings::reload()
{
    d->fallbackData.privateValues.clear();
    d->fallbackData.values.clear();
    d->fromJsonFile(d->fallbackFile, &d->fallbackData);

    d->writableData.privateValues.clear();
    d->writableData.values.clear();
    d->fromJsonFile(d->settingFile, &d->writableData);
}
/*!
 * \brief Settings::sync 将属性写入到配置文件中
 *
 * \return  bool 是否写入成功
 */
bool Settings::sync()
{
    if (!d->settingFileIsDirty) {
        return true;
    }

    const QByteArray &json = d->toJson(d->writableData);

    QFile file(d->settingFile);

    if (!file.open(QFile::WriteOnly)) {
        return false;
    }

    bool ok = file.write(json) == json.size();

    if (ok) {
        d->makeSettingFileToDirty(false);
    }
    file.close();

    return ok;
}
/*!
 * \brief Settings::autoSync 自动将属性写入配置文件
 *
 * \return bool 是否写入成功
 */
bool Settings::autoSync() const
{
    return d->autoSync;
}
/*!
 * \brief Settings::watchChanges 槽函数监听配置文件是否有改变
 *
 * \return bool 配置文件是否有改变
 */
bool Settings::watchChanges() const
{
    return d->watchChanges;
}

void Settings::autoSyncExclude(const QString &group, bool sync /*= false*/)
{
    if (!sync)
        d->autoSyncGroupExclude.insert(group);
    else
        d->autoSyncGroupExclude.remove(group);
}
/*!
 * \brief Settings::setAutoSync 设置是否自动写配置文件
 *
 * \param autoSync 是否自动写配置文件
 */
void Settings::setAutoSync(bool autoSync)
{
    if (d->autoSync == autoSync) {
        return;
    }

    d->autoSync = autoSync;

    if (autoSync) {
        if (d->settingFileIsDirty) {
            sync();
        }

        if (!d->syncTimer) {
            d->syncTimer = new QTimer(this);
            d->syncTimer->moveToThread(thread());
            d->syncTimer->setSingleShot(true);
            d->syncTimer->setInterval(1000);

            connect(d->syncTimer, &QTimer::timeout, this, &Settings::sync);
        }
    } else {
        if (d->syncTimer) {
            d->syncTimer->stop();
            d->syncTimer->deleteLater();
            d->syncTimer = nullptr;
        }
    }
}
/*!
 * \brief Settings::onFileChanged 配置文件的对应key属性是否发生改变
 *
 * \param url 文件url
 */
void Settings::onFileChanged(const QUrl &url)
{
    d->_q_onFileChanged(url);
}

void Settings::setWatchChanges(bool watchChanges)
{
    if (d->watchChanges == watchChanges)
        return;

    d->watchChanges = watchChanges;

    if (watchChanges) {
        {
            QFileInfo info(d->settingFile);
            if (!info.exists()) {
                if (info.absoluteDir().mkpath(info.absolutePath())) {
                    QFile file(d->settingFile);
                    file.open(QFile::WriteOnly);
                }
            }
        }
        d->settingWatcher = WatcherFactory::create<AbstractFileWatcher>(QUrl::fromLocalFile(d->settingFile));
        if (!d->settingWatcher) {
            qCWarning(logDFMBase) << "Create watcher failed:" << d->settingFile;
            return;
        }

        d->settingWatcher->moveToThread(thread());
        connect(d->settingWatcher.get(), &AbstractFileWatcher::fileAttributeChanged, this, &Settings::onFileChanged);

        d->settingWatcher->startWatcher();
    } else if (d->settingWatcher) {
        d->settingWatcher.reset();
    }
}

}
