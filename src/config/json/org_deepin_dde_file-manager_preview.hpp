// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORG_DEEPIN_DDE_FILE-MANAGER_PREVIEW_H
#define ORG_DEEPIN_DDE_FILE-MANAGER_PREVIEW_H

#include <QThread>
#include <QVariant>
#include <QDebug>
#include <QAtomicPointer>
#include <QAtomicInteger>
#include <DConfig>

class org_deepin_dde_file-manager_preview : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool mtpThumbnailEnable READ mtpThumbnailEnable WRITE setMtpThumbnailEnable NOTIFY mtpThumbnailEnableChanged)
    Q_PROPERTY(bool previewEnable READ previewEnable WRITE setPreviewEnable NOTIFY previewEnableChanged)
    Q_PROPERTY(bool remoteThumbnailEnable READ remoteThumbnailEnable WRITE setRemoteThumbnailEnable NOTIFY remoteThumbnailEnableChanged)
public:
    explicit org_deepin_dde_file-manager_preview(QThread *thread, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_preview(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_preview(QThread *thread, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_preview(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    ~org_deepin_dde_file-manager_preview() {
        if (m_config.loadRelaxed()) {
            m_config.loadRelaxed()->deleteLater();
        }
    }

    bool mtpThumbnailEnable() const {
        return p_mtpThumbnailEnable;
    }
    void setMtpThumbnailEnable(const bool &value) {
        auto oldValue = p_mtpThumbnailEnable;
        p_mtpThumbnailEnable = value;
        markPropertySet(0);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("mtpThumbnailEnable"), value);
            });
        }
        if (p_mtpThumbnailEnable != oldValue) {
            Q_EMIT mtpThumbnailEnableChanged();
        }
    }
    bool previewEnable() const {
        return p_previewEnable;
    }
    void setPreviewEnable(const bool &value) {
        auto oldValue = p_previewEnable;
        p_previewEnable = value;
        markPropertySet(1);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("previewEnable"), value);
            });
        }
        if (p_previewEnable != oldValue) {
            Q_EMIT previewEnableChanged();
        }
    }
    bool remoteThumbnailEnable() const {
        return p_remoteThumbnailEnable;
    }
    void setRemoteThumbnailEnable(const bool &value) {
        auto oldValue = p_remoteThumbnailEnable;
        p_remoteThumbnailEnable = value;
        markPropertySet(2);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("remoteThumbnailEnable"), value);
            });
        }
        if (p_remoteThumbnailEnable != oldValue) {
            Q_EMIT remoteThumbnailEnableChanged();
        }
    }
Q_SIGNALS:
    void mtpThumbnailEnableChanged();
    void previewEnableChanged();
    void remoteThumbnailEnableChanged();
private:
    void initialize(DTK_CORE_NAMESPACE::DConfig *config) {
        Q_ASSERT(!m_config.loadRelaxed());
        m_config.storeRelaxed(config);
        if (testPropertySet(0)) {
            config->setValue(QStringLiteral("mtpThumbnailEnable"), QVariant::fromValue(p_mtpThumbnailEnable));
        } else {
            updateValue(QStringLiteral("mtpThumbnailEnable"), QVariant::fromValue(p_mtpThumbnailEnable));
        }
        if (testPropertySet(1)) {
            config->setValue(QStringLiteral("previewEnable"), QVariant::fromValue(p_previewEnable));
        } else {
            updateValue(QStringLiteral("previewEnable"), QVariant::fromValue(p_previewEnable));
        }
        if (testPropertySet(2)) {
            config->setValue(QStringLiteral("remoteThumbnailEnable"), QVariant::fromValue(p_remoteThumbnailEnable));
        } else {
            updateValue(QStringLiteral("remoteThumbnailEnable"), QVariant::fromValue(p_remoteThumbnailEnable));
        }

        connect(config, &DTK_CORE_NAMESPACE::DConfig::valueChanged, this, [this](const QString &key) {
            updateValue(key);
        }, Qt::DirectConnection);
    }
    void updateValue(const QString &key, const QVariant &fallback = QVariant()) {
        Q_ASSERT(QThread::currentThread() == m_config.loadRelaxed()->thread());
        const QVariant &value = m_config.loadRelaxed()->value(key, fallback);
        if (key == QStringLiteral("mtpThumbnailEnable")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_mtpThumbnailEnable != newValue) {
                    p_mtpThumbnailEnable = newValue;
                    Q_EMIT mtpThumbnailEnableChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("previewEnable")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_previewEnable != newValue) {
                    p_previewEnable = newValue;
                    Q_EMIT previewEnableChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("remoteThumbnailEnable")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_remoteThumbnailEnable != newValue) {
                    p_remoteThumbnailEnable = newValue;
                    Q_EMIT remoteThumbnailEnableChanged();
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
    bool p_mtpThumbnailEnable { true };
    bool p_previewEnable { true };
    bool p_remoteThumbnailEnable { false };
    QAtomicInteger<quint32> m_propertySetStatus0 = 0;
};

#endif // ORG_DEEPIN_DDE_FILE-MANAGER_PREVIEW_H
