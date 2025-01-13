// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORG_DEEPIN_DDE_FILE-MANAGER_PLUGINS_H
#define ORG_DEEPIN_DDE_FILE-MANAGER_PLUGINS_H

#include <QThread>
#include <QVariant>
#include <QDebug>
#include <QAtomicPointer>
#include <QAtomicInteger>
#include <DConfig>

class org_deepin_dde_file-manager_plugins : public QObject {
    Q_OBJECT

    Q_PROPERTY(QList<QVariant> daemon.blackList READ daemon.blackList WRITE setDaemon.blackList NOTIFY daemon.blackListChanged)
    Q_PROPERTY(QList<QVariant> desktop.blackList READ desktop.blackList WRITE setDesktop.blackList NOTIFY desktop.blackListChanged)
    Q_PROPERTY(QList<QVariant> filedialog.blackList READ filedialog.blackList WRITE setFiledialog.blackList NOTIFY filedialog.blackListChanged)
    Q_PROPERTY(QList<QVariant> filemanager.blackList READ filemanager.blackList WRITE setFilemanager.blackList NOTIFY filemanager.blackListChanged)
    Q_PROPERTY(QList<QVariant> filemanager.disablelist READ filemanager.disablelist WRITE setFilemanager.disablelist NOTIFY filemanager.disablelistChanged)
    Q_PROPERTY(QList<QVariant> server.blackList READ server.blackList WRITE setServer.blackList NOTIFY server.blackListChanged)
public:
    explicit org_deepin_dde_file-manager_plugins(QThread *thread, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_plugins(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_plugins(QThread *thread, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_plugins(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    ~org_deepin_dde_file-manager_plugins() {
        if (m_config.loadRelaxed()) {
            m_config.loadRelaxed()->deleteLater();
        }
    }

    QList<QVariant> daemon.blackList() const {
        return p_daemon.blackList;
    }
    void setDaemon.blackList(const QList<QVariant> &value) {
        auto oldValue = p_daemon.blackList;
        p_daemon.blackList = value;
        markPropertySet(0);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("daemon.blackList"), value);
            });
        }
        if (p_daemon.blackList != oldValue) {
            Q_EMIT daemon.blackListChanged();
        }
    }
    QList<QVariant> desktop.blackList() const {
        return p_desktop.blackList;
    }
    void setDesktop.blackList(const QList<QVariant> &value) {
        auto oldValue = p_desktop.blackList;
        p_desktop.blackList = value;
        markPropertySet(1);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("desktop.blackList"), value);
            });
        }
        if (p_desktop.blackList != oldValue) {
            Q_EMIT desktop.blackListChanged();
        }
    }
    QList<QVariant> filedialog.blackList() const {
        return p_filedialog.blackList;
    }
    void setFiledialog.blackList(const QList<QVariant> &value) {
        auto oldValue = p_filedialog.blackList;
        p_filedialog.blackList = value;
        markPropertySet(2);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("filedialog.blackList"), value);
            });
        }
        if (p_filedialog.blackList != oldValue) {
            Q_EMIT filedialog.blackListChanged();
        }
    }
    QList<QVariant> filemanager.blackList() const {
        return p_filemanager.blackList;
    }
    void setFilemanager.blackList(const QList<QVariant> &value) {
        auto oldValue = p_filemanager.blackList;
        p_filemanager.blackList = value;
        markPropertySet(3);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("filemanager.blackList"), value);
            });
        }
        if (p_filemanager.blackList != oldValue) {
            Q_EMIT filemanager.blackListChanged();
        }
    }
    QList<QVariant> filemanager.disablelist() const {
        return p_filemanager.disablelist;
    }
    void setFilemanager.disablelist(const QList<QVariant> &value) {
        auto oldValue = p_filemanager.disablelist;
        p_filemanager.disablelist = value;
        markPropertySet(4);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("filemanager.disablelist"), value);
            });
        }
        if (p_filemanager.disablelist != oldValue) {
            Q_EMIT filemanager.disablelistChanged();
        }
    }
    QList<QVariant> server.blackList() const {
        return p_server.blackList;
    }
    void setServer.blackList(const QList<QVariant> &value) {
        auto oldValue = p_server.blackList;
        p_server.blackList = value;
        markPropertySet(5);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("server.blackList"), value);
            });
        }
        if (p_server.blackList != oldValue) {
            Q_EMIT server.blackListChanged();
        }
    }
Q_SIGNALS:
    void daemon.blackListChanged();
    void desktop.blackListChanged();
    void filedialog.blackListChanged();
    void filemanager.blackListChanged();
    void filemanager.disablelistChanged();
    void server.blackListChanged();
private:
    void initialize(DTK_CORE_NAMESPACE::DConfig *config) {
        Q_ASSERT(!m_config.loadRelaxed());
        m_config.storeRelaxed(config);
        if (testPropertySet(0)) {
            config->setValue(QStringLiteral("daemon.blackList"), QVariant::fromValue(p_daemon.blackList));
        } else {
            updateValue(QStringLiteral("daemon.blackList"), QVariant::fromValue(p_daemon.blackList));
        }
        if (testPropertySet(1)) {
            config->setValue(QStringLiteral("desktop.blackList"), QVariant::fromValue(p_desktop.blackList));
        } else {
            updateValue(QStringLiteral("desktop.blackList"), QVariant::fromValue(p_desktop.blackList));
        }
        if (testPropertySet(2)) {
            config->setValue(QStringLiteral("filedialog.blackList"), QVariant::fromValue(p_filedialog.blackList));
        } else {
            updateValue(QStringLiteral("filedialog.blackList"), QVariant::fromValue(p_filedialog.blackList));
        }
        if (testPropertySet(3)) {
            config->setValue(QStringLiteral("filemanager.blackList"), QVariant::fromValue(p_filemanager.blackList));
        } else {
            updateValue(QStringLiteral("filemanager.blackList"), QVariant::fromValue(p_filemanager.blackList));
        }
        if (testPropertySet(4)) {
            config->setValue(QStringLiteral("filemanager.disablelist"), QVariant::fromValue(p_filemanager.disablelist));
        } else {
            updateValue(QStringLiteral("filemanager.disablelist"), QVariant::fromValue(p_filemanager.disablelist));
        }
        if (testPropertySet(5)) {
            config->setValue(QStringLiteral("server.blackList"), QVariant::fromValue(p_server.blackList));
        } else {
            updateValue(QStringLiteral("server.blackList"), QVariant::fromValue(p_server.blackList));
        }

        connect(config, &DTK_CORE_NAMESPACE::DConfig::valueChanged, this, [this](const QString &key) {
            updateValue(key);
        }, Qt::DirectConnection);
    }
    void updateValue(const QString &key, const QVariant &fallback = QVariant()) {
        Q_ASSERT(QThread::currentThread() == m_config.loadRelaxed()->thread());
        const QVariant &value = m_config.loadRelaxed()->value(key, fallback);
        if (key == QStringLiteral("daemon.blackList")) {
            auto newValue = qvariant_cast<QList<QVariant>>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_daemon.blackList != newValue) {
                    p_daemon.blackList = newValue;
                    Q_EMIT daemon.blackListChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("desktop.blackList")) {
            auto newValue = qvariant_cast<QList<QVariant>>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_desktop.blackList != newValue) {
                    p_desktop.blackList = newValue;
                    Q_EMIT desktop.blackListChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("filedialog.blackList")) {
            auto newValue = qvariant_cast<QList<QVariant>>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_filedialog.blackList != newValue) {
                    p_filedialog.blackList = newValue;
                    Q_EMIT filedialog.blackListChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("filemanager.blackList")) {
            auto newValue = qvariant_cast<QList<QVariant>>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_filemanager.blackList != newValue) {
                    p_filemanager.blackList = newValue;
                    Q_EMIT filemanager.blackListChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("filemanager.disablelist")) {
            auto newValue = qvariant_cast<QList<QVariant>>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_filemanager.disablelist != newValue) {
                    p_filemanager.disablelist = newValue;
                    Q_EMIT filemanager.disablelistChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("server.blackList")) {
            auto newValue = qvariant_cast<QList<QVariant>>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_server.blackList != newValue) {
                    p_server.blackList = newValue;
                    Q_EMIT server.blackListChanged();
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
    QList<QVariant> p_daemon.blackList { QList<QVariant>{} };
    QList<QVariant> p_desktop.blackList { QList<QVariant>{} };
    QList<QVariant> p_filedialog.blackList { QList<QVariant>{QVariant(QStringLiteral("dfmplugin-burn")), QVariant(QStringLiteral("dfmplugin-dirshare")), QVariant(QStringLiteral("dfmplugin-myshares")), QVariant(QStringLiteral("dfmplugin-propertydialog")), QVariant(QStringLiteral("dfmplugin-trashcore")), QVariant(QStringLiteral("dfmplugin-trash")), QVariant(QStringLiteral("dfmplugin-filepreview")), QVariant(QStringLiteral("dfmplugin-vault"))} };
    QList<QVariant> p_filemanager.blackList { QList<QVariant>{} };
    QList<QVariant> p_filemanager.disablelist { QList<QVariant>{} };
    QList<QVariant> p_server.blackList { QList<QVariant>{} };
    QAtomicInteger<quint32> m_propertySetStatus0 = 0;
};

#endif // ORG_DEEPIN_DDE_FILE-MANAGER_PLUGINS_H
