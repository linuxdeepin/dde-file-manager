// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORG_DEEPIN_DDE_FILE-MANAGER_VIEW_H
#define ORG_DEEPIN_DDE_FILE-MANAGER_VIEW_H

#include <QThread>
#include <QVariant>
#include <QDebug>
#include <QAtomicPointer>
#include <QAtomicInteger>
#include <DConfig>

class org_deepin_dde_file-manager_view : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool dfm.displaypreview.visible READ dfm.displaypreview.visible WRITE setDfm.displaypreview.visible NOTIFY dfm.displaypreview.visibleChanged)
    Q_PROPERTY(double dfm.icon.griddensity.level READ dfm.icon.griddensity.level WRITE setDfm.icon.griddensity.level NOTIFY dfm.icon.griddensity.levelChanged)
    Q_PROPERTY(double dfm.icon.size.level READ dfm.icon.size.level WRITE setDfm.icon.size.level NOTIFY dfm.icon.size.levelChanged)
    Q_PROPERTY(double dfm.list.height.level READ dfm.list.height.level WRITE setDfm.list.height.level NOTIFY dfm.list.height.levelChanged)
    Q_PROPERTY(bool dfm.open.in.single.process READ dfm.open.in.single.process WRITE setDfm.open.in.single.process NOTIFY dfm.open.in.single.processChanged)
    Q_PROPERTY(bool dfm.treeview.enable READ dfm.treeview.enable WRITE setDfm.treeview.enable NOTIFY dfm.treeview.enableChanged)
public:
    explicit org_deepin_dde_file-manager_view(QThread *thread, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_view(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_view(QThread *thread, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_view(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    ~org_deepin_dde_file-manager_view() {
        if (m_config.loadRelaxed()) {
            m_config.loadRelaxed()->deleteLater();
        }
    }

    bool dfm.displaypreview.visible() const {
        return p_dfm.displaypreview.visible;
    }
    void setDfm.displaypreview.visible(const bool &value) {
        auto oldValue = p_dfm.displaypreview.visible;
        p_dfm.displaypreview.visible = value;
        markPropertySet(0);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.displaypreview.visible"), value);
            });
        }
        if (p_dfm.displaypreview.visible != oldValue) {
            Q_EMIT dfm.displaypreview.visibleChanged();
        }
    }
    double dfm.icon.griddensity.level() const {
        return p_dfm.icon.griddensity.level;
    }
    void setDfm.icon.griddensity.level(const double &value) {
        auto oldValue = p_dfm.icon.griddensity.level;
        p_dfm.icon.griddensity.level = value;
        markPropertySet(1);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.icon.griddensity.level"), value);
            });
        }
        if (p_dfm.icon.griddensity.level != oldValue) {
            Q_EMIT dfm.icon.griddensity.levelChanged();
        }
    }
    double dfm.icon.size.level() const {
        return p_dfm.icon.size.level;
    }
    void setDfm.icon.size.level(const double &value) {
        auto oldValue = p_dfm.icon.size.level;
        p_dfm.icon.size.level = value;
        markPropertySet(2);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.icon.size.level"), value);
            });
        }
        if (p_dfm.icon.size.level != oldValue) {
            Q_EMIT dfm.icon.size.levelChanged();
        }
    }
    double dfm.list.height.level() const {
        return p_dfm.list.height.level;
    }
    void setDfm.list.height.level(const double &value) {
        auto oldValue = p_dfm.list.height.level;
        p_dfm.list.height.level = value;
        markPropertySet(3);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.list.height.level"), value);
            });
        }
        if (p_dfm.list.height.level != oldValue) {
            Q_EMIT dfm.list.height.levelChanged();
        }
    }
    bool dfm.open.in.single.process() const {
        return p_dfm.open.in.single.process;
    }
    void setDfm.open.in.single.process(const bool &value) {
        auto oldValue = p_dfm.open.in.single.process;
        p_dfm.open.in.single.process = value;
        markPropertySet(4);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.open.in.single.process"), value);
            });
        }
        if (p_dfm.open.in.single.process != oldValue) {
            Q_EMIT dfm.open.in.single.processChanged();
        }
    }
    bool dfm.treeview.enable() const {
        return p_dfm.treeview.enable;
    }
    void setDfm.treeview.enable(const bool &value) {
        auto oldValue = p_dfm.treeview.enable;
        p_dfm.treeview.enable = value;
        markPropertySet(5);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.treeview.enable"), value);
            });
        }
        if (p_dfm.treeview.enable != oldValue) {
            Q_EMIT dfm.treeview.enableChanged();
        }
    }
Q_SIGNALS:
    void dfm.displaypreview.visibleChanged();
    void dfm.icon.griddensity.levelChanged();
    void dfm.icon.size.levelChanged();
    void dfm.list.height.levelChanged();
    void dfm.open.in.single.processChanged();
    void dfm.treeview.enableChanged();
private:
    void initialize(DTK_CORE_NAMESPACE::DConfig *config) {
        Q_ASSERT(!m_config.loadRelaxed());
        m_config.storeRelaxed(config);
        if (testPropertySet(0)) {
            config->setValue(QStringLiteral("dfm.displaypreview.visible"), QVariant::fromValue(p_dfm.displaypreview.visible));
        } else {
            updateValue(QStringLiteral("dfm.displaypreview.visible"), QVariant::fromValue(p_dfm.displaypreview.visible));
        }
        if (testPropertySet(1)) {
            config->setValue(QStringLiteral("dfm.icon.griddensity.level"), QVariant::fromValue(p_dfm.icon.griddensity.level));
        } else {
            updateValue(QStringLiteral("dfm.icon.griddensity.level"), QVariant::fromValue(p_dfm.icon.griddensity.level));
        }
        if (testPropertySet(2)) {
            config->setValue(QStringLiteral("dfm.icon.size.level"), QVariant::fromValue(p_dfm.icon.size.level));
        } else {
            updateValue(QStringLiteral("dfm.icon.size.level"), QVariant::fromValue(p_dfm.icon.size.level));
        }
        if (testPropertySet(3)) {
            config->setValue(QStringLiteral("dfm.list.height.level"), QVariant::fromValue(p_dfm.list.height.level));
        } else {
            updateValue(QStringLiteral("dfm.list.height.level"), QVariant::fromValue(p_dfm.list.height.level));
        }
        if (testPropertySet(4)) {
            config->setValue(QStringLiteral("dfm.open.in.single.process"), QVariant::fromValue(p_dfm.open.in.single.process));
        } else {
            updateValue(QStringLiteral("dfm.open.in.single.process"), QVariant::fromValue(p_dfm.open.in.single.process));
        }
        if (testPropertySet(5)) {
            config->setValue(QStringLiteral("dfm.treeview.enable"), QVariant::fromValue(p_dfm.treeview.enable));
        } else {
            updateValue(QStringLiteral("dfm.treeview.enable"), QVariant::fromValue(p_dfm.treeview.enable));
        }

        connect(config, &DTK_CORE_NAMESPACE::DConfig::valueChanged, this, [this](const QString &key) {
            updateValue(key);
        }, Qt::DirectConnection);
    }
    void updateValue(const QString &key, const QVariant &fallback = QVariant()) {
        Q_ASSERT(QThread::currentThread() == m_config.loadRelaxed()->thread());
        const QVariant &value = m_config.loadRelaxed()->value(key, fallback);
        if (key == QStringLiteral("dfm.displaypreview.visible")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.displaypreview.visible != newValue) {
                    p_dfm.displaypreview.visible = newValue;
                    Q_EMIT dfm.displaypreview.visibleChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.icon.griddensity.level")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.icon.griddensity.level != newValue) {
                    p_dfm.icon.griddensity.level = newValue;
                    Q_EMIT dfm.icon.griddensity.levelChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.icon.size.level")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.icon.size.level != newValue) {
                    p_dfm.icon.size.level = newValue;
                    Q_EMIT dfm.icon.size.levelChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.list.height.level")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.list.height.level != newValue) {
                    p_dfm.list.height.level = newValue;
                    Q_EMIT dfm.list.height.levelChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.open.in.single.process")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.open.in.single.process != newValue) {
                    p_dfm.open.in.single.process = newValue;
                    Q_EMIT dfm.open.in.single.processChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.treeview.enable")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.treeview.enable != newValue) {
                    p_dfm.treeview.enable = newValue;
                    Q_EMIT dfm.treeview.enableChanged();
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
    bool p_dfm.displaypreview.visible { false };
    double p_dfm.icon.griddensity.level { 2 };
    double p_dfm.icon.size.level { 5 };
    double p_dfm.list.height.level { 1 };
    bool p_dfm.open.in.single.process { true };
    bool p_dfm.treeview.enable { true };
    QAtomicInteger<quint32> m_propertySetStatus0 = 0;
};

#endif // ORG_DEEPIN_DDE_FILE-MANAGER_VIEW_H
