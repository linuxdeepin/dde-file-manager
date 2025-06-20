// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORG_DEEPIN_DDE_FILE-MANAGER_H
#define ORG_DEEPIN_DDE_FILE-MANAGER_H

#include <QThread>
#include <QVariant>
#include <QDebug>
#include <QAtomicPointer>
#include <QAtomicInteger>
#include <DConfig>

class org_deepin_dde_file-manager : public QObject {
    Q_OBJECT

    Q_PROPERTY(QList<QVariant> bookmark READ bookmark WRITE setBookmark NOTIFY bookmarkChanged)
    Q_PROPERTY(bool dd.disabled READ dd.disabled WRITE setDd.disabled NOTIFY dd.disabledChanged)
    Q_PROPERTY(QList<QVariant> dd.menu.action.hidden READ dd.menu.action.hidden WRITE setDd.menu.action.hidden NOTIFY dd.menu.action.hiddenChanged)
    Q_PROPERTY(QList<QVariant> dfd.menu.action.hidden READ dfd.menu.action.hidden WRITE setDfd.menu.action.hidden NOTIFY dfd.menu.action.hiddenChanged)
    Q_PROPERTY(QList<QVariant> dfm.disk.hidden READ dfm.disk.hidden WRITE setDfm.disk.hidden NOTIFY dfm.disk.hiddenChanged)
    Q_PROPERTY(bool dfm.headless READ dfm.headless WRITE setDfm.headless NOTIFY dfm.headlessChanged)
    Q_PROPERTY(bool dfm.iterator.allasync READ dfm.iterator.allasync WRITE setDfm.iterator.allasync NOTIFY dfm.iterator.allasyncChanged)
    Q_PROPERTY(bool dfm.memory.autorelease READ dfm.memory.autorelease WRITE setDfm.memory.autorelease NOTIFY dfm.memory.autoreleaseChanged)
    Q_PROPERTY(QList<QVariant> dfm.menu.action.hidden READ dfm.menu.action.hidden WRITE setDfm.menu.action.hidden NOTIFY dfm.menu.action.hiddenChanged)
    Q_PROPERTY(bool dfm.menu.blockdev.enable READ dfm.menu.blockdev.enable WRITE setDfm.menu.blockdev.enable NOTIFY dfm.menu.blockdev.enableChanged)
    Q_PROPERTY(QList<QVariant> dfm.menu.hidden READ dfm.menu.hidden WRITE setDfm.menu.hidden NOTIFY dfm.menu.hiddenChanged)
    Q_PROPERTY(bool dfm.menu.protocoldev.enable READ dfm.menu.protocoldev.enable WRITE setDfm.menu.protocoldev.enable NOTIFY dfm.menu.protocoldev.enableChanged)
    Q_PROPERTY(bool dfm.mount.dlnfs READ dfm.mount.dlnfs WRITE setDfm.mount.dlnfs NOTIFY dfm.mount.dlnfsChanged)
    Q_PROPERTY(QList<QVariant> dfm.mount.dlnfs.defaults READ dfm.mount.dlnfs.defaults WRITE setDfm.mount.dlnfs.defaults NOTIFY dfm.mount.dlnfs.defaultsChanged)
    Q_PROPERTY(bool dfm.samba.permanent READ dfm.samba.permanent WRITE setDfm.samba.permanent NOTIFY dfm.samba.permanentChanged)
    Q_PROPERTY(bool dfm.trash.allfiletotrash READ dfm.trash.allfiletotrash WRITE setDfm.trash.allfiletotrash NOTIFY dfm.trash.allfiletotrashChanged)
    Q_PROPERTY(QString dfm.vault.algo.name READ dfm.vault.algo.name WRITE setDfm.vault.algo.name NOTIFY dfm.vault.algo.nameChanged)
    Q_PROPERTY(QString log_rules READ log_rules WRITE setLog_rules NOTIFY log_rulesChanged)
public:
    explicit org_deepin_dde_file-manager(QThread *thread, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
        : QObject(parent) {

        if (!thread->isRunning()) {
            qWarning() << QStringLiteral("Warning: The provided thread is not running.");
        }
        Q_ASSERT(QThread::currentThread() != thread);
        auto worker = new QObject();
        worker->moveToThread(thread);
        QMetaObject::invokeMethod(worker, [=]() {
            auto config = DTK_CORE_NAMESPACE::DConfig::create(appId, name, subpath, nullptr);
            if (!config) {
                qWarning() << QStringLiteral("Failed to create DConfig instance.");
                worker->deleteLater();
                return;
            }
            config->moveToThread(QThread::currentThread());
            initialize(config);
            worker->deleteLater();
        });
    }
    explicit org_deepin_dde_file-manager(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
        : QObject(parent) {

        if (!thread->isRunning()) {
            qWarning() << QStringLiteral("Warning: The provided thread is not running.");
        }
        Q_ASSERT(QThread::currentThread() != thread);
        auto worker = new QObject();
        worker->moveToThread(thread);
        QMetaObject::invokeMethod(worker, [=]() {
            auto config = DTK_CORE_NAMESPACE::DConfig::create(backend, appId, name, subpath, nullptr);
            if (!config) {
                qWarning() << QStringLiteral("Failed to create DConfig instance.");
                worker->deleteLater();
                return;
            }
            config->moveToThread(QThread::currentThread());
            initialize(config);
            worker->deleteLater();
        });
    }
    explicit org_deepin_dde_file-manager(QThread *thread, const QString &name, const QString &subpath, QObject *parent = nullptr)
        : QObject(parent) {

        if (!thread->isRunning()) {
            qWarning() << QStringLiteral("Warning: The provided thread is not running.");
        }
        Q_ASSERT(QThread::currentThread() != thread);
        auto worker = new QObject();
        worker->moveToThread(thread);
        QMetaObject::invokeMethod(worker, [=]() {
            auto config = DTK_CORE_NAMESPACE::DConfig::create(name, subpath, nullptr);
            if (!config) {
                qWarning() << QStringLiteral("Failed to create DConfig instance.");
                worker->deleteLater();
                return;
            }
            config->moveToThread(QThread::currentThread());
            initialize(config);
            worker->deleteLater();
        });
    }
    explicit org_deepin_dde_file-manager(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &subpath, QObject *parent = nullptr)
        : QObject(parent) {

        if (!thread->isRunning()) {
            qWarning() << QStringLiteral("Warning: The provided thread is not running.");
        }
        Q_ASSERT(QThread::currentThread() != thread);
        auto worker = new QObject();
        worker->moveToThread(thread);
        QMetaObject::invokeMethod(worker, [=]() {
            auto config = DTK_CORE_NAMESPACE::DConfig::create(backend, name, subpath, nullptr);
            if (!config) {
                qWarning() << QStringLiteral("Failed to create DConfig instance.");
                worker->deleteLater();
                return;
            }
            config->moveToThread(QThread::currentThread());
            initialize(config);
            worker->deleteLater();
        });
    }
    ~org_deepin_dde_file-manager() {
        if (m_config.loadRelaxed()) {
            m_config.loadRelaxed()->deleteLater();
        }
    }

    QList<QVariant> bookmark() const {
        return p_bookmark;
    }
    void setBookmark(const QList<QVariant> &value) {
        auto oldValue = p_bookmark;
        p_bookmark = value;
        markPropertySet(0);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("bookmark"), value);
            });
        }
        if (p_bookmark != oldValue) {
            Q_EMIT bookmarkChanged();
        }
    }
    bool dd.disabled() const {
        return p_dd.disabled;
    }
    void setDd.disabled(const bool &value) {
        auto oldValue = p_dd.disabled;
        p_dd.disabled = value;
        markPropertySet(1);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dd.disabled"), value);
            });
        }
        if (p_dd.disabled != oldValue) {
            Q_EMIT dd.disabledChanged();
        }
    }
    QList<QVariant> dd.menu.action.hidden() const {
        return p_dd.menu.action.hidden;
    }
    void setDd.menu.action.hidden(const QList<QVariant> &value) {
        auto oldValue = p_dd.menu.action.hidden;
        p_dd.menu.action.hidden = value;
        markPropertySet(2);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dd.menu.action.hidden"), value);
            });
        }
        if (p_dd.menu.action.hidden != oldValue) {
            Q_EMIT dd.menu.action.hiddenChanged();
        }
    }
    QList<QVariant> dfd.menu.action.hidden() const {
        return p_dfd.menu.action.hidden;
    }
    void setDfd.menu.action.hidden(const QList<QVariant> &value) {
        auto oldValue = p_dfd.menu.action.hidden;
        p_dfd.menu.action.hidden = value;
        markPropertySet(3);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfd.menu.action.hidden"), value);
            });
        }
        if (p_dfd.menu.action.hidden != oldValue) {
            Q_EMIT dfd.menu.action.hiddenChanged();
        }
    }
    QList<QVariant> dfm.disk.hidden() const {
        return p_dfm.disk.hidden;
    }
    void setDfm.disk.hidden(const QList<QVariant> &value) {
        auto oldValue = p_dfm.disk.hidden;
        p_dfm.disk.hidden = value;
        markPropertySet(4);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.disk.hidden"), value);
            });
        }
        if (p_dfm.disk.hidden != oldValue) {
            Q_EMIT dfm.disk.hiddenChanged();
        }
    }
    bool dfm.headless() const {
        return p_dfm.headless;
    }
    void setDfm.headless(const bool &value) {
        auto oldValue = p_dfm.headless;
        p_dfm.headless = value;
        markPropertySet(5);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.headless"), value);
            });
        }
        if (p_dfm.headless != oldValue) {
            Q_EMIT dfm.headlessChanged();
        }
    }
    bool dfm.iterator.allasync() const {
        return p_dfm.iterator.allasync;
    }
    void setDfm.iterator.allasync(const bool &value) {
        auto oldValue = p_dfm.iterator.allasync;
        p_dfm.iterator.allasync = value;
        markPropertySet(6);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.iterator.allasync"), value);
            });
        }
        if (p_dfm.iterator.allasync != oldValue) {
            Q_EMIT dfm.iterator.allasyncChanged();
        }
    }
    bool dfm.memory.autorelease() const {
        return p_dfm.memory.autorelease;
    }
    void setDfm.memory.autorelease(const bool &value) {
        auto oldValue = p_dfm.memory.autorelease;
        p_dfm.memory.autorelease = value;
        markPropertySet(7);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.memory.autorelease"), value);
            });
        }
        if (p_dfm.memory.autorelease != oldValue) {
            Q_EMIT dfm.memory.autoreleaseChanged();
        }
    }
    QList<QVariant> dfm.menu.action.hidden() const {
        return p_dfm.menu.action.hidden;
    }
    void setDfm.menu.action.hidden(const QList<QVariant> &value) {
        auto oldValue = p_dfm.menu.action.hidden;
        p_dfm.menu.action.hidden = value;
        markPropertySet(8);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.menu.action.hidden"), value);
            });
        }
        if (p_dfm.menu.action.hidden != oldValue) {
            Q_EMIT dfm.menu.action.hiddenChanged();
        }
    }
    bool dfm.menu.blockdev.enable() const {
        return p_dfm.menu.blockdev.enable;
    }
    void setDfm.menu.blockdev.enable(const bool &value) {
        auto oldValue = p_dfm.menu.blockdev.enable;
        p_dfm.menu.blockdev.enable = value;
        markPropertySet(9);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.menu.blockdev.enable"), value);
            });
        }
        if (p_dfm.menu.blockdev.enable != oldValue) {
            Q_EMIT dfm.menu.blockdev.enableChanged();
        }
    }
    QList<QVariant> dfm.menu.hidden() const {
        return p_dfm.menu.hidden;
    }
    void setDfm.menu.hidden(const QList<QVariant> &value) {
        auto oldValue = p_dfm.menu.hidden;
        p_dfm.menu.hidden = value;
        markPropertySet(10);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.menu.hidden"), value);
            });
        }
        if (p_dfm.menu.hidden != oldValue) {
            Q_EMIT dfm.menu.hiddenChanged();
        }
    }
    bool dfm.menu.protocoldev.enable() const {
        return p_dfm.menu.protocoldev.enable;
    }
    void setDfm.menu.protocoldev.enable(const bool &value) {
        auto oldValue = p_dfm.menu.protocoldev.enable;
        p_dfm.menu.protocoldev.enable = value;
        markPropertySet(11);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.menu.protocoldev.enable"), value);
            });
        }
        if (p_dfm.menu.protocoldev.enable != oldValue) {
            Q_EMIT dfm.menu.protocoldev.enableChanged();
        }
    }
    bool dfm.mount.dlnfs() const {
        return p_dfm.mount.dlnfs;
    }
    void setDfm.mount.dlnfs(const bool &value) {
        auto oldValue = p_dfm.mount.dlnfs;
        p_dfm.mount.dlnfs = value;
        markPropertySet(12);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.mount.dlnfs"), value);
            });
        }
        if (p_dfm.mount.dlnfs != oldValue) {
            Q_EMIT dfm.mount.dlnfsChanged();
        }
    }
    QList<QVariant> dfm.mount.dlnfs.defaults() const {
        return p_dfm.mount.dlnfs.defaults;
    }
    void setDfm.mount.dlnfs.defaults(const QList<QVariant> &value) {
        auto oldValue = p_dfm.mount.dlnfs.defaults;
        p_dfm.mount.dlnfs.defaults = value;
        markPropertySet(13);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.mount.dlnfs.defaults"), value);
            });
        }
        if (p_dfm.mount.dlnfs.defaults != oldValue) {
            Q_EMIT dfm.mount.dlnfs.defaultsChanged();
        }
    }
    bool dfm.samba.permanent() const {
        return p_dfm.samba.permanent;
    }
    void setDfm.samba.permanent(const bool &value) {
        auto oldValue = p_dfm.samba.permanent;
        p_dfm.samba.permanent = value;
        markPropertySet(14);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.samba.permanent"), value);
            });
        }
        if (p_dfm.samba.permanent != oldValue) {
            Q_EMIT dfm.samba.permanentChanged();
        }
    }
    bool dfm.trash.allfiletotrash() const {
        return p_dfm.trash.allfiletotrash;
    }
    void setDfm.trash.allfiletotrash(const bool &value) {
        auto oldValue = p_dfm.trash.allfiletotrash;
        p_dfm.trash.allfiletotrash = value;
        markPropertySet(15);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.trash.allfiletotrash"), value);
            });
        }
        if (p_dfm.trash.allfiletotrash != oldValue) {
            Q_EMIT dfm.trash.allfiletotrashChanged();
        }
    }
    QString dfm.vault.algo.name() const {
        return p_dfm.vault.algo.name;
    }
    void setDfm.vault.algo.name(const QString &value) {
        auto oldValue = p_dfm.vault.algo.name;
        p_dfm.vault.algo.name = value;
        markPropertySet(16);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.vault.algo.name"), value);
            });
        }
        if (p_dfm.vault.algo.name != oldValue) {
            Q_EMIT dfm.vault.algo.nameChanged();
        }
    }
    QString log_rules() const {
        return p_log_rules;
    }
    void setLog_rules(const QString &value) {
        auto oldValue = p_log_rules;
        p_log_rules = value;
        markPropertySet(17);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("log_rules"), value);
            });
        }
        if (p_log_rules != oldValue) {
            Q_EMIT log_rulesChanged();
        }
    }
Q_SIGNALS:
    void bookmarkChanged();
    void dd.disabledChanged();
    void dd.menu.action.hiddenChanged();
    void dfd.menu.action.hiddenChanged();
    void dfm.disk.hiddenChanged();
    void dfm.headlessChanged();
    void dfm.iterator.allasyncChanged();
    void dfm.memory.autoreleaseChanged();
    void dfm.menu.action.hiddenChanged();
    void dfm.menu.blockdev.enableChanged();
    void dfm.menu.hiddenChanged();
    void dfm.menu.protocoldev.enableChanged();
    void dfm.mount.dlnfsChanged();
    void dfm.mount.dlnfs.defaultsChanged();
    void dfm.samba.permanentChanged();
    void dfm.trash.allfiletotrashChanged();
    void dfm.vault.algo.nameChanged();
    void log_rulesChanged();
private:
    void initialize(DTK_CORE_NAMESPACE::DConfig *config) {
        Q_ASSERT(!m_config.loadRelaxed());
        m_config.storeRelaxed(config);
        if (testPropertySet(0)) {
            config->setValue(QStringLiteral("bookmark"), QVariant::fromValue(p_bookmark));
        } else {
            updateValue(QStringLiteral("bookmark"), QVariant::fromValue(p_bookmark));
        }
        if (testPropertySet(1)) {
            config->setValue(QStringLiteral("dd.disabled"), QVariant::fromValue(p_dd.disabled));
        } else {
            updateValue(QStringLiteral("dd.disabled"), QVariant::fromValue(p_dd.disabled));
        }
        if (testPropertySet(2)) {
            config->setValue(QStringLiteral("dd.menu.action.hidden"), QVariant::fromValue(p_dd.menu.action.hidden));
        } else {
            updateValue(QStringLiteral("dd.menu.action.hidden"), QVariant::fromValue(p_dd.menu.action.hidden));
        }
        if (testPropertySet(3)) {
            config->setValue(QStringLiteral("dfd.menu.action.hidden"), QVariant::fromValue(p_dfd.menu.action.hidden));
        } else {
            updateValue(QStringLiteral("dfd.menu.action.hidden"), QVariant::fromValue(p_dfd.menu.action.hidden));
        }
        if (testPropertySet(4)) {
            config->setValue(QStringLiteral("dfm.disk.hidden"), QVariant::fromValue(p_dfm.disk.hidden));
        } else {
            updateValue(QStringLiteral("dfm.disk.hidden"), QVariant::fromValue(p_dfm.disk.hidden));
        }
        if (testPropertySet(5)) {
            config->setValue(QStringLiteral("dfm.headless"), QVariant::fromValue(p_dfm.headless));
        } else {
            updateValue(QStringLiteral("dfm.headless"), QVariant::fromValue(p_dfm.headless));
        }
        if (testPropertySet(6)) {
            config->setValue(QStringLiteral("dfm.iterator.allasync"), QVariant::fromValue(p_dfm.iterator.allasync));
        } else {
            updateValue(QStringLiteral("dfm.iterator.allasync"), QVariant::fromValue(p_dfm.iterator.allasync));
        }
        if (testPropertySet(7)) {
            config->setValue(QStringLiteral("dfm.memory.autorelease"), QVariant::fromValue(p_dfm.memory.autorelease));
        } else {
            updateValue(QStringLiteral("dfm.memory.autorelease"), QVariant::fromValue(p_dfm.memory.autorelease));
        }
        if (testPropertySet(8)) {
            config->setValue(QStringLiteral("dfm.menu.action.hidden"), QVariant::fromValue(p_dfm.menu.action.hidden));
        } else {
            updateValue(QStringLiteral("dfm.menu.action.hidden"), QVariant::fromValue(p_dfm.menu.action.hidden));
        }
        if (testPropertySet(9)) {
            config->setValue(QStringLiteral("dfm.menu.blockdev.enable"), QVariant::fromValue(p_dfm.menu.blockdev.enable));
        } else {
            updateValue(QStringLiteral("dfm.menu.blockdev.enable"), QVariant::fromValue(p_dfm.menu.blockdev.enable));
        }
        if (testPropertySet(10)) {
            config->setValue(QStringLiteral("dfm.menu.hidden"), QVariant::fromValue(p_dfm.menu.hidden));
        } else {
            updateValue(QStringLiteral("dfm.menu.hidden"), QVariant::fromValue(p_dfm.menu.hidden));
        }
        if (testPropertySet(11)) {
            config->setValue(QStringLiteral("dfm.menu.protocoldev.enable"), QVariant::fromValue(p_dfm.menu.protocoldev.enable));
        } else {
            updateValue(QStringLiteral("dfm.menu.protocoldev.enable"), QVariant::fromValue(p_dfm.menu.protocoldev.enable));
        }
        if (testPropertySet(12)) {
            config->setValue(QStringLiteral("dfm.mount.dlnfs"), QVariant::fromValue(p_dfm.mount.dlnfs));
        } else {
            updateValue(QStringLiteral("dfm.mount.dlnfs"), QVariant::fromValue(p_dfm.mount.dlnfs));
        }
        if (testPropertySet(13)) {
            config->setValue(QStringLiteral("dfm.mount.dlnfs.defaults"), QVariant::fromValue(p_dfm.mount.dlnfs.defaults));
        } else {
            updateValue(QStringLiteral("dfm.mount.dlnfs.defaults"), QVariant::fromValue(p_dfm.mount.dlnfs.defaults));
        }
        if (testPropertySet(14)) {
            config->setValue(QStringLiteral("dfm.samba.permanent"), QVariant::fromValue(p_dfm.samba.permanent));
        } else {
            updateValue(QStringLiteral("dfm.samba.permanent"), QVariant::fromValue(p_dfm.samba.permanent));
        }
        if (testPropertySet(15)) {
            config->setValue(QStringLiteral("dfm.trash.allfiletotrash"), QVariant::fromValue(p_dfm.trash.allfiletotrash));
        } else {
            updateValue(QStringLiteral("dfm.trash.allfiletotrash"), QVariant::fromValue(p_dfm.trash.allfiletotrash));
        }
        if (testPropertySet(16)) {
            config->setValue(QStringLiteral("dfm.vault.algo.name"), QVariant::fromValue(p_dfm.vault.algo.name));
        } else {
            updateValue(QStringLiteral("dfm.vault.algo.name"), QVariant::fromValue(p_dfm.vault.algo.name));
        }
        if (testPropertySet(17)) {
            config->setValue(QStringLiteral("log_rules"), QVariant::fromValue(p_log_rules));
        } else {
            updateValue(QStringLiteral("log_rules"), QVariant::fromValue(p_log_rules));
        }

        connect(config, &DTK_CORE_NAMESPACE::DConfig::valueChanged, this, [this](const QString &key) {
            updateValue(key);
        }, Qt::DirectConnection);
    }
    void updateValue(const QString &key, const QVariant &fallback = QVariant()) {
        Q_ASSERT(QThread::currentThread() == m_config.loadRelaxed()->thread());
        const QVariant &value = m_config.loadRelaxed()->value(key, fallback);
        if (key == QStringLiteral("bookmark")) {
            auto newValue = qvariant_cast<QList<QVariant>>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_bookmark != newValue) {
                    p_bookmark = newValue;
                    Q_EMIT bookmarkChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dd.disabled")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dd.disabled != newValue) {
                    p_dd.disabled = newValue;
                    Q_EMIT dd.disabledChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dd.menu.action.hidden")) {
            auto newValue = qvariant_cast<QList<QVariant>>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dd.menu.action.hidden != newValue) {
                    p_dd.menu.action.hidden = newValue;
                    Q_EMIT dd.menu.action.hiddenChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfd.menu.action.hidden")) {
            auto newValue = qvariant_cast<QList<QVariant>>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfd.menu.action.hidden != newValue) {
                    p_dfd.menu.action.hidden = newValue;
                    Q_EMIT dfd.menu.action.hiddenChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.disk.hidden")) {
            auto newValue = qvariant_cast<QList<QVariant>>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.disk.hidden != newValue) {
                    p_dfm.disk.hidden = newValue;
                    Q_EMIT dfm.disk.hiddenChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.headless")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.headless != newValue) {
                    p_dfm.headless = newValue;
                    Q_EMIT dfm.headlessChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.iterator.allasync")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.iterator.allasync != newValue) {
                    p_dfm.iterator.allasync = newValue;
                    Q_EMIT dfm.iterator.allasyncChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.memory.autorelease")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.memory.autorelease != newValue) {
                    p_dfm.memory.autorelease = newValue;
                    Q_EMIT dfm.memory.autoreleaseChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.menu.action.hidden")) {
            auto newValue = qvariant_cast<QList<QVariant>>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.menu.action.hidden != newValue) {
                    p_dfm.menu.action.hidden = newValue;
                    Q_EMIT dfm.menu.action.hiddenChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.menu.blockdev.enable")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.menu.blockdev.enable != newValue) {
                    p_dfm.menu.blockdev.enable = newValue;
                    Q_EMIT dfm.menu.blockdev.enableChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.menu.hidden")) {
            auto newValue = qvariant_cast<QList<QVariant>>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.menu.hidden != newValue) {
                    p_dfm.menu.hidden = newValue;
                    Q_EMIT dfm.menu.hiddenChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.menu.protocoldev.enable")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.menu.protocoldev.enable != newValue) {
                    p_dfm.menu.protocoldev.enable = newValue;
                    Q_EMIT dfm.menu.protocoldev.enableChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.mount.dlnfs")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.mount.dlnfs != newValue) {
                    p_dfm.mount.dlnfs = newValue;
                    Q_EMIT dfm.mount.dlnfsChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.mount.dlnfs.defaults")) {
            auto newValue = qvariant_cast<QList<QVariant>>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.mount.dlnfs.defaults != newValue) {
                    p_dfm.mount.dlnfs.defaults = newValue;
                    Q_EMIT dfm.mount.dlnfs.defaultsChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.samba.permanent")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.samba.permanent != newValue) {
                    p_dfm.samba.permanent = newValue;
                    Q_EMIT dfm.samba.permanentChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.trash.allfiletotrash")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.trash.allfiletotrash != newValue) {
                    p_dfm.trash.allfiletotrash = newValue;
                    Q_EMIT dfm.trash.allfiletotrashChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.vault.algo.name")) {
            auto newValue = qvariant_cast<QString>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.vault.algo.name != newValue) {
                    p_dfm.vault.algo.name = newValue;
                    Q_EMIT dfm.vault.algo.nameChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("log_rules")) {
            auto newValue = qvariant_cast<QString>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_log_rules != newValue) {
                    p_log_rules = newValue;
                    Q_EMIT log_rulesChanged();
                }
            });
            return;
        }
    }
    inline void markPropertySet(const int index) {
        if (index < 32) {
            m_propertySetStatus0.fetchAndOrOrdered(1 << (index - 0));
            return;
        }
        Q_UNREACHABLE();
    }
    inline bool testPropertySet(const int index) const {
        if (index < 32) {
            return (m_propertySetStatus0.loadRelaxed() & (1 << (index - 0)));
        }
        Q_UNREACHABLE();
    }
    QAtomicPointer<DTK_CORE_NAMESPACE::DConfig> m_config = nullptr;
    QList<QVariant> p_bookmark { QList<QVariant>{} };
    bool p_dd.disabled { false };
    QList<QVariant> p_dd.menu.action.hidden { QList<QVariant>{} };
    QList<QVariant> p_dfd.menu.action.hidden { QList<QVariant>{} };
    QList<QVariant> p_dfm.disk.hidden { QList<QVariant>{} };
    bool p_dfm.headless { true };
    bool p_dfm.iterator.allasync { false };
    bool p_dfm.memory.autorelease { true };
    QList<QVariant> p_dfm.menu.action.hidden { QList<QVariant>{} };
    bool p_dfm.menu.blockdev.enable { true };
    QList<QVariant> p_dfm.menu.hidden { QList<QVariant>{} };
    bool p_dfm.menu.protocoldev.enable { true };
    bool p_dfm.mount.dlnfs { false };
    QList<QVariant> p_dfm.mount.dlnfs.defaults { QList<QVariant>{QVariant(QStringLiteral("$HOME/Desktop")), QVariant(QStringLiteral("$HOME/Music")), QVariant(QStringLiteral("$HOME/Pictures")), QVariant(QStringLiteral("$HOME/Downloads")), QVariant(QStringLiteral("$HOME/Videos")), QVariant(QStringLiteral("$HOME/Documents"))} };
    bool p_dfm.samba.permanent { true };
    bool p_dfm.trash.allfiletotrash { false };
    QString p_dfm.vault.algo.name { QStringLiteral("sm4-128-ecb") };
    QString p_log_rules { QStringLiteral("*.debug=false;*.info=false;*.warning=true") };
    QAtomicInteger<quint32> m_propertySetStatus0 = 0;
};

#endif // ORG_DEEPIN_DDE_FILE-MANAGER_H
