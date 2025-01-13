// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORG_DEEPIN_DDE_FILE-MANAGER_DISKENCRYPT_H
#define ORG_DEEPIN_DDE_FILE-MANAGER_DISKENCRYPT_H

#include <QThread>
#include <QVariant>
#include <QDebug>
#include <QAtomicPointer>
#include <QAtomicInteger>
#include <DConfig>

class org_deepin_dde_file-manager_diskencrypt : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool allowExportEncKey READ allowExportEncKey WRITE setAllowExportEncKey NOTIFY allowExportEncKeyChanged)
    Q_PROPERTY(bool enableEncrypt READ enableEncrypt WRITE setEnableEncrypt NOTIFY enableEncryptChanged)
    Q_PROPERTY(QString encryptAlgorithm READ encryptAlgorithm WRITE setEncryptAlgorithm NOTIFY encryptAlgorithmChanged)
public:
    explicit org_deepin_dde_file-manager_diskencrypt(QThread *thread, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_diskencrypt(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_diskencrypt(QThread *thread, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_diskencrypt(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    ~org_deepin_dde_file-manager_diskencrypt() {
        if (m_config.loadRelaxed()) {
            m_config.loadRelaxed()->deleteLater();
        }
    }

    bool allowExportEncKey() const {
        return p_allowExportEncKey;
    }
    void setAllowExportEncKey(const bool &value) {
        auto oldValue = p_allowExportEncKey;
        p_allowExportEncKey = value;
        markPropertySet(0);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("allowExportEncKey"), value);
            });
        }
        if (p_allowExportEncKey != oldValue) {
            Q_EMIT allowExportEncKeyChanged();
        }
    }
    bool enableEncrypt() const {
        return p_enableEncrypt;
    }
    void setEnableEncrypt(const bool &value) {
        auto oldValue = p_enableEncrypt;
        p_enableEncrypt = value;
        markPropertySet(1);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("enableEncrypt"), value);
            });
        }
        if (p_enableEncrypt != oldValue) {
            Q_EMIT enableEncryptChanged();
        }
    }
    QString encryptAlgorithm() const {
        return p_encryptAlgorithm;
    }
    void setEncryptAlgorithm(const QString &value) {
        auto oldValue = p_encryptAlgorithm;
        p_encryptAlgorithm = value;
        markPropertySet(2);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("encryptAlgorithm"), value);
            });
        }
        if (p_encryptAlgorithm != oldValue) {
            Q_EMIT encryptAlgorithmChanged();
        }
    }
Q_SIGNALS:
    void allowExportEncKeyChanged();
    void enableEncryptChanged();
    void encryptAlgorithmChanged();
private:
    void initialize(DTK_CORE_NAMESPACE::DConfig *config) {
        Q_ASSERT(!m_config.loadRelaxed());
        m_config.storeRelaxed(config);
        if (testPropertySet(0)) {
            config->setValue(QStringLiteral("allowExportEncKey"), QVariant::fromValue(p_allowExportEncKey));
        } else {
            updateValue(QStringLiteral("allowExportEncKey"), QVariant::fromValue(p_allowExportEncKey));
        }
        if (testPropertySet(1)) {
            config->setValue(QStringLiteral("enableEncrypt"), QVariant::fromValue(p_enableEncrypt));
        } else {
            updateValue(QStringLiteral("enableEncrypt"), QVariant::fromValue(p_enableEncrypt));
        }
        if (testPropertySet(2)) {
            config->setValue(QStringLiteral("encryptAlgorithm"), QVariant::fromValue(p_encryptAlgorithm));
        } else {
            updateValue(QStringLiteral("encryptAlgorithm"), QVariant::fromValue(p_encryptAlgorithm));
        }

        connect(config, &DTK_CORE_NAMESPACE::DConfig::valueChanged, this, [this](const QString &key) {
            updateValue(key);
        }, Qt::DirectConnection);
    }
    void updateValue(const QString &key, const QVariant &fallback = QVariant()) {
        Q_ASSERT(QThread::currentThread() == m_config.loadRelaxed()->thread());
        const QVariant &value = m_config.loadRelaxed()->value(key, fallback);
        if (key == QStringLiteral("allowExportEncKey")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_allowExportEncKey != newValue) {
                    p_allowExportEncKey = newValue;
                    Q_EMIT allowExportEncKeyChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("enableEncrypt")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_enableEncrypt != newValue) {
                    p_enableEncrypt = newValue;
                    Q_EMIT enableEncryptChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("encryptAlgorithm")) {
            auto newValue = qvariant_cast<QString>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_encryptAlgorithm != newValue) {
                    p_encryptAlgorithm = newValue;
                    Q_EMIT encryptAlgorithmChanged();
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
    bool p_allowExportEncKey { true };
    bool p_enableEncrypt { false };
    QString p_encryptAlgorithm { QStringLiteral("sm4") };
    QAtomicInteger<quint32> m_propertySetStatus0 = 0;
};

#endif // ORG_DEEPIN_DDE_FILE-MANAGER_DISKENCRYPT_H
