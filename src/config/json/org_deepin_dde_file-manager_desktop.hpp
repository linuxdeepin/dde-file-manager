// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORG_DEEPIN_DDE_FILE-MANAGER_DESKTOP_H
#define ORG_DEEPIN_DDE_FILE-MANAGER_DESKTOP_H

#include <QThread>
#include <QVariant>
#include <QDebug>
#include <QAtomicPointer>
#include <QAtomicInteger>
#include <DConfig>

class org_deepin_dde_file-manager_desktop : public QObject {
    Q_OBJECT

    Q_PROPERTY(double autoAlign READ autoAlign WRITE setAutoAlign NOTIFY autoAlignChanged)
    Q_PROPERTY(bool enableMask READ enableMask WRITE setEnableMask NOTIFY enableMaskChanged)
    Q_PROPERTY(double maskHeight READ maskHeight WRITE setMaskHeight NOTIFY maskHeightChanged)
    Q_PROPERTY(QString maskLogoUri READ maskLogoUri WRITE setMaskLogoUri NOTIFY maskLogoUriChanged)
    Q_PROPERTY(double maskOffsetX READ maskOffsetX WRITE setMaskOffsetX NOTIFY maskOffsetXChanged)
    Q_PROPERTY(double maskOffsetY READ maskOffsetY WRITE setMaskOffsetY NOTIFY maskOffsetYChanged)
    Q_PROPERTY(double maskWidth READ maskWidth WRITE setMaskWidth NOTIFY maskWidthChanged)
public:
    explicit org_deepin_dde_file-manager_desktop(QThread *thread, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_desktop(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_desktop(QThread *thread, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_desktop(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    ~org_deepin_dde_file-manager_desktop() {
        if (m_config.loadRelaxed()) {
            m_config.loadRelaxed()->deleteLater();
        }
    }

    double autoAlign() const {
        return p_autoAlign;
    }
    void setAutoAlign(const double &value) {
        auto oldValue = p_autoAlign;
        p_autoAlign = value;
        markPropertySet(0);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("autoAlign"), value);
            });
        }
        if (p_autoAlign != oldValue) {
            Q_EMIT autoAlignChanged();
        }
    }
    bool enableMask() const {
        return p_enableMask;
    }
    void setEnableMask(const bool &value) {
        auto oldValue = p_enableMask;
        p_enableMask = value;
        markPropertySet(1);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("enableMask"), value);
            });
        }
        if (p_enableMask != oldValue) {
            Q_EMIT enableMaskChanged();
        }
    }
    double maskHeight() const {
        return p_maskHeight;
    }
    void setMaskHeight(const double &value) {
        auto oldValue = p_maskHeight;
        p_maskHeight = value;
        markPropertySet(2);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("maskHeight"), value);
            });
        }
        if (p_maskHeight != oldValue) {
            Q_EMIT maskHeightChanged();
        }
    }
    QString maskLogoUri() const {
        return p_maskLogoUri;
    }
    void setMaskLogoUri(const QString &value) {
        auto oldValue = p_maskLogoUri;
        p_maskLogoUri = value;
        markPropertySet(3);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("maskLogoUri"), value);
            });
        }
        if (p_maskLogoUri != oldValue) {
            Q_EMIT maskLogoUriChanged();
        }
    }
    double maskOffsetX() const {
        return p_maskOffsetX;
    }
    void setMaskOffsetX(const double &value) {
        auto oldValue = p_maskOffsetX;
        p_maskOffsetX = value;
        markPropertySet(4);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("maskOffsetX"), value);
            });
        }
        if (p_maskOffsetX != oldValue) {
            Q_EMIT maskOffsetXChanged();
        }
    }
    double maskOffsetY() const {
        return p_maskOffsetY;
    }
    void setMaskOffsetY(const double &value) {
        auto oldValue = p_maskOffsetY;
        p_maskOffsetY = value;
        markPropertySet(5);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("maskOffsetY"), value);
            });
        }
        if (p_maskOffsetY != oldValue) {
            Q_EMIT maskOffsetYChanged();
        }
    }
    double maskWidth() const {
        return p_maskWidth;
    }
    void setMaskWidth(const double &value) {
        auto oldValue = p_maskWidth;
        p_maskWidth = value;
        markPropertySet(6);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("maskWidth"), value);
            });
        }
        if (p_maskWidth != oldValue) {
            Q_EMIT maskWidthChanged();
        }
    }
Q_SIGNALS:
    void autoAlignChanged();
    void enableMaskChanged();
    void maskHeightChanged();
    void maskLogoUriChanged();
    void maskOffsetXChanged();
    void maskOffsetYChanged();
    void maskWidthChanged();
private:
    void initialize(DTK_CORE_NAMESPACE::DConfig *config) {
        Q_ASSERT(!m_config.loadRelaxed());
        m_config.storeRelaxed(config);
        if (testPropertySet(0)) {
            config->setValue(QStringLiteral("autoAlign"), QVariant::fromValue(p_autoAlign));
        } else {
            updateValue(QStringLiteral("autoAlign"), QVariant::fromValue(p_autoAlign));
        }
        if (testPropertySet(1)) {
            config->setValue(QStringLiteral("enableMask"), QVariant::fromValue(p_enableMask));
        } else {
            updateValue(QStringLiteral("enableMask"), QVariant::fromValue(p_enableMask));
        }
        if (testPropertySet(2)) {
            config->setValue(QStringLiteral("maskHeight"), QVariant::fromValue(p_maskHeight));
        } else {
            updateValue(QStringLiteral("maskHeight"), QVariant::fromValue(p_maskHeight));
        }
        if (testPropertySet(3)) {
            config->setValue(QStringLiteral("maskLogoUri"), QVariant::fromValue(p_maskLogoUri));
        } else {
            updateValue(QStringLiteral("maskLogoUri"), QVariant::fromValue(p_maskLogoUri));
        }
        if (testPropertySet(4)) {
            config->setValue(QStringLiteral("maskOffsetX"), QVariant::fromValue(p_maskOffsetX));
        } else {
            updateValue(QStringLiteral("maskOffsetX"), QVariant::fromValue(p_maskOffsetX));
        }
        if (testPropertySet(5)) {
            config->setValue(QStringLiteral("maskOffsetY"), QVariant::fromValue(p_maskOffsetY));
        } else {
            updateValue(QStringLiteral("maskOffsetY"), QVariant::fromValue(p_maskOffsetY));
        }
        if (testPropertySet(6)) {
            config->setValue(QStringLiteral("maskWidth"), QVariant::fromValue(p_maskWidth));
        } else {
            updateValue(QStringLiteral("maskWidth"), QVariant::fromValue(p_maskWidth));
        }

        connect(config, &DTK_CORE_NAMESPACE::DConfig::valueChanged, this, [this](const QString &key) {
            updateValue(key);
        }, Qt::DirectConnection);
    }
    void updateValue(const QString &key, const QVariant &fallback = QVariant()) {
        Q_ASSERT(QThread::currentThread() == m_config.loadRelaxed()->thread());
        const QVariant &value = m_config.loadRelaxed()->value(key, fallback);
        if (key == QStringLiteral("autoAlign")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_autoAlign != newValue) {
                    p_autoAlign = newValue;
                    Q_EMIT autoAlignChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("enableMask")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_enableMask != newValue) {
                    p_enableMask = newValue;
                    Q_EMIT enableMaskChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("maskHeight")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_maskHeight != newValue) {
                    p_maskHeight = newValue;
                    Q_EMIT maskHeightChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("maskLogoUri")) {
            auto newValue = qvariant_cast<QString>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_maskLogoUri != newValue) {
                    p_maskLogoUri = newValue;
                    Q_EMIT maskLogoUriChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("maskOffsetX")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_maskOffsetX != newValue) {
                    p_maskOffsetX = newValue;
                    Q_EMIT maskOffsetXChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("maskOffsetY")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_maskOffsetY != newValue) {
                    p_maskOffsetY = newValue;
                    Q_EMIT maskOffsetYChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("maskWidth")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_maskWidth != newValue) {
                    p_maskWidth = newValue;
                    Q_EMIT maskWidthChanged();
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
    double p_autoAlign { -1 };
    bool p_enableMask { false };
    double p_maskHeight { 14 };
    QString p_maskLogoUri { QStringLiteral("") };
    double p_maskOffsetX { 0 };
    double p_maskOffsetY { 0 };
    double p_maskWidth { 114 };
    QAtomicInteger<quint32> m_propertySetStatus0 = 0;
};

#endif // ORG_DEEPIN_DDE_FILE-MANAGER_DESKTOP_H
