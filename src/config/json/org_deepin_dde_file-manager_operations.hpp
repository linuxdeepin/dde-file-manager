// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORG_DEEPIN_DDE_FILE-MANAGER_OPERATIONS_H
#define ORG_DEEPIN_DDE_FILE-MANAGER_OPERATIONS_H

#include <QThread>
#include <QVariant>
#include <QDebug>
#include <QAtomicPointer>
#include <QAtomicInteger>
#include <DConfig>

class org_deepin_dde_file-manager_operations : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString file.operation.bigfilesize READ file.operation.bigfilesize WRITE setFile.operation.bigfilesize NOTIFY file.operation.bigfilesizeChanged)
    Q_PROPERTY(bool file.operation.blockeverysync READ file.operation.blockeverysync WRITE setFile.operation.blockeverysync NOTIFY file.operation.blockeverysyncChanged)
    Q_PROPERTY(bool file.operation.broadcastpastevent READ file.operation.broadcastpastevent WRITE setFile.operation.broadcastpastevent NOTIFY file.operation.broadcastpasteventChanged)
public:
    explicit org_deepin_dde_file-manager_operations(QThread *thread, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_operations(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_operations(QThread *thread, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_operations(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    ~org_deepin_dde_file-manager_operations() {
        if (m_config.loadRelaxed()) {
            m_config.loadRelaxed()->deleteLater();
        }
    }

    QString file.operation.bigfilesize() const {
        return p_file.operation.bigfilesize;
    }
    void setFile.operation.bigfilesize(const QString &value) {
        auto oldValue = p_file.operation.bigfilesize;
        p_file.operation.bigfilesize = value;
        markPropertySet(0);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("file.operation.bigfilesize"), value);
            });
        }
        if (p_file.operation.bigfilesize != oldValue) {
            Q_EMIT file.operation.bigfilesizeChanged();
        }
    }
    bool file.operation.blockeverysync() const {
        return p_file.operation.blockeverysync;
    }
    void setFile.operation.blockeverysync(const bool &value) {
        auto oldValue = p_file.operation.blockeverysync;
        p_file.operation.blockeverysync = value;
        markPropertySet(1);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("file.operation.blockeverysync"), value);
            });
        }
        if (p_file.operation.blockeverysync != oldValue) {
            Q_EMIT file.operation.blockeverysyncChanged();
        }
    }
    bool file.operation.broadcastpastevent() const {
        return p_file.operation.broadcastpastevent;
    }
    void setFile.operation.broadcastpastevent(const bool &value) {
        auto oldValue = p_file.operation.broadcastpastevent;
        p_file.operation.broadcastpastevent = value;
        markPropertySet(2);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("file.operation.broadcastpastevent"), value);
            });
        }
        if (p_file.operation.broadcastpastevent != oldValue) {
            Q_EMIT file.operation.broadcastpasteventChanged();
        }
    }
Q_SIGNALS:
    void file.operation.bigfilesizeChanged();
    void file.operation.blockeverysyncChanged();
    void file.operation.broadcastpasteventChanged();
private:
    void initialize(DTK_CORE_NAMESPACE::DConfig *config) {
        Q_ASSERT(!m_config.loadRelaxed());
        m_config.storeRelaxed(config);
        if (testPropertySet(0)) {
            config->setValue(QStringLiteral("file.operation.bigfilesize"), QVariant::fromValue(p_file.operation.bigfilesize));
        } else {
            updateValue(QStringLiteral("file.operation.bigfilesize"), QVariant::fromValue(p_file.operation.bigfilesize));
        }
        if (testPropertySet(1)) {
            config->setValue(QStringLiteral("file.operation.blockeverysync"), QVariant::fromValue(p_file.operation.blockeverysync));
        } else {
            updateValue(QStringLiteral("file.operation.blockeverysync"), QVariant::fromValue(p_file.operation.blockeverysync));
        }
        if (testPropertySet(2)) {
            config->setValue(QStringLiteral("file.operation.broadcastpastevent"), QVariant::fromValue(p_file.operation.broadcastpastevent));
        } else {
            updateValue(QStringLiteral("file.operation.broadcastpastevent"), QVariant::fromValue(p_file.operation.broadcastpastevent));
        }

        connect(config, &DTK_CORE_NAMESPACE::DConfig::valueChanged, this, [this](const QString &key) {
            updateValue(key);
        }, Qt::DirectConnection);
    }
    void updateValue(const QString &key, const QVariant &fallback = QVariant()) {
        Q_ASSERT(QThread::currentThread() == m_config.loadRelaxed()->thread());
        const QVariant &value = m_config.loadRelaxed()->value(key, fallback);
        if (key == QStringLiteral("file.operation.bigfilesize")) {
            auto newValue = qvariant_cast<QString>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_file.operation.bigfilesize != newValue) {
                    p_file.operation.bigfilesize = newValue;
                    Q_EMIT file.operation.bigfilesizeChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("file.operation.blockeverysync")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_file.operation.blockeverysync != newValue) {
                    p_file.operation.blockeverysync = newValue;
                    Q_EMIT file.operation.blockeverysyncChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("file.operation.broadcastpastevent")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_file.operation.broadcastpastevent != newValue) {
                    p_file.operation.broadcastpastevent = newValue;
                    Q_EMIT file.operation.broadcastpasteventChanged();
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
    QString p_file.operation.bigfilesize { QStringLiteral("83886080") };
    bool p_file.operation.blockeverysync { true };
    bool p_file.operation.broadcastpastevent { false };
    QAtomicInteger<quint32> m_propertySetStatus0 = 0;
};

#endif // ORG_DEEPIN_DDE_FILE-MANAGER_OPERATIONS_H
