// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORG_DEEPIN_DDE_FILE-MANAGER_MOUNT_H
#define ORG_DEEPIN_DDE_FILE-MANAGER_MOUNT_H

#include <QThread>
#include <QVariant>
#include <QDebug>
#include <QAtomicPointer>
#include <QAtomicInteger>
#include <DConfig>

class org_deepin_dde_file-manager_mount : public QObject {
    Q_OBJECT

    Q_PROPERTY(QVariantMap cifsMountOptionOverride READ cifsMountOptionOverride WRITE setCifsMountOptionOverride NOTIFY cifsMountOptionOverrideChanged)
    Q_PROPERTY(double deviceCapacityDisplay READ deviceCapacityDisplay WRITE setDeviceCapacityDisplay NOTIFY deviceCapacityDisplayChanged)
    Q_PROPERTY(bool enableCifsMount READ enableCifsMount WRITE setEnableCifsMount NOTIFY enableCifsMountChanged)
public:
    explicit org_deepin_dde_file-manager_mount(QThread *thread, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_mount(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_mount(QThread *thread, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_mount(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    ~org_deepin_dde_file-manager_mount() {
        if (m_config.loadRelaxed()) {
            m_config.loadRelaxed()->deleteLater();
        }
    }

    QVariantMap cifsMountOptionOverride() const {
        return p_cifsMountOptionOverride;
    }
    void setCifsMountOptionOverride(const QVariantMap &value) {
        auto oldValue = p_cifsMountOptionOverride;
        p_cifsMountOptionOverride = value;
        markPropertySet(0);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("cifsMountOptionOverride"), value);
            });
        }
        if (p_cifsMountOptionOverride != oldValue) {
            Q_EMIT cifsMountOptionOverrideChanged();
        }
    }
    double deviceCapacityDisplay() const {
        return p_deviceCapacityDisplay;
    }
    void setDeviceCapacityDisplay(const double &value) {
        auto oldValue = p_deviceCapacityDisplay;
        p_deviceCapacityDisplay = value;
        markPropertySet(1);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("deviceCapacityDisplay"), value);
            });
        }
        if (p_deviceCapacityDisplay != oldValue) {
            Q_EMIT deviceCapacityDisplayChanged();
        }
    }
    bool enableCifsMount() const {
        return p_enableCifsMount;
    }
    void setEnableCifsMount(const bool &value) {
        auto oldValue = p_enableCifsMount;
        p_enableCifsMount = value;
        markPropertySet(2);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("enableCifsMount"), value);
            });
        }
        if (p_enableCifsMount != oldValue) {
            Q_EMIT enableCifsMountChanged();
        }
    }
Q_SIGNALS:
    void cifsMountOptionOverrideChanged();
    void deviceCapacityDisplayChanged();
    void enableCifsMountChanged();
private:
    void initialize(DTK_CORE_NAMESPACE::DConfig *config) {
        Q_ASSERT(!m_config.loadRelaxed());
        m_config.storeRelaxed(config);
        if (testPropertySet(0)) {
            config->setValue(QStringLiteral("cifsMountOptionOverride"), QVariant::fromValue(p_cifsMountOptionOverride));
        } else {
            updateValue(QStringLiteral("cifsMountOptionOverride"), QVariant::fromValue(p_cifsMountOptionOverride));
        }
        if (testPropertySet(1)) {
            config->setValue(QStringLiteral("deviceCapacityDisplay"), QVariant::fromValue(p_deviceCapacityDisplay));
        } else {
            updateValue(QStringLiteral("deviceCapacityDisplay"), QVariant::fromValue(p_deviceCapacityDisplay));
        }
        if (testPropertySet(2)) {
            config->setValue(QStringLiteral("enableCifsMount"), QVariant::fromValue(p_enableCifsMount));
        } else {
            updateValue(QStringLiteral("enableCifsMount"), QVariant::fromValue(p_enableCifsMount));
        }

        connect(config, &DTK_CORE_NAMESPACE::DConfig::valueChanged, this, [this](const QString &key) {
            updateValue(key);
        }, Qt::DirectConnection);
    }
    void updateValue(const QString &key, const QVariant &fallback = QVariant()) {
        Q_ASSERT(QThread::currentThread() == m_config.loadRelaxed()->thread());
        const QVariant &value = m_config.loadRelaxed()->value(key, fallback);
        if (key == QStringLiteral("cifsMountOptionOverride")) {
            auto newValue = qvariant_cast<QVariantMap>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_cifsMountOptionOverride != newValue) {
                    p_cifsMountOptionOverride = newValue;
                    Q_EMIT cifsMountOptionOverrideChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("deviceCapacityDisplay")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_deviceCapacityDisplay != newValue) {
                    p_deviceCapacityDisplay = newValue;
                    Q_EMIT deviceCapacityDisplayChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("enableCifsMount")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_enableCifsMount != newValue) {
                    p_enableCifsMount = newValue;
                    Q_EMIT enableCifsMountChanged();
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
    QVariantMap p_cifsMountOptionOverride { QVariantMap{} };
    double p_deviceCapacityDisplay { 0 };
    bool p_enableCifsMount { true };
    QAtomicInteger<quint32> m_propertySetStatus0 = 0;
};

#endif // ORG_DEEPIN_DDE_FILE-MANAGER_MOUNT_H
