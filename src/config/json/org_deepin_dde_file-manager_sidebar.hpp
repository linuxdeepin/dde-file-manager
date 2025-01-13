// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORG_DEEPIN_DDE_FILE-MANAGER_SIDEBAR_H
#define ORG_DEEPIN_DDE_FILE-MANAGER_SIDEBAR_H

#include <QThread>
#include <QVariant>
#include <QDebug>
#include <QAtomicPointer>
#include <QAtomicInteger>
#include <DConfig>

class org_deepin_dde_file-manager_sidebar : public QObject {
    Q_OBJECT

    Q_PROPERTY(QVariantMap groupExpanded READ groupExpanded WRITE setGroupExpanded NOTIFY groupExpandedChanged)
    Q_PROPERTY(QVariantMap itemVisiable READ itemVisiable WRITE setItemVisiable NOTIFY itemVisiableChanged)
public:
    explicit org_deepin_dde_file-manager_sidebar(QThread *thread, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_sidebar(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_sidebar(QThread *thread, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_sidebar(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    ~org_deepin_dde_file-manager_sidebar() {
        if (m_config.loadRelaxed()) {
            m_config.loadRelaxed()->deleteLater();
        }
    }

    QVariantMap groupExpanded() const {
        return p_groupExpanded;
    }
    void setGroupExpanded(const QVariantMap &value) {
        auto oldValue = p_groupExpanded;
        p_groupExpanded = value;
        markPropertySet(0);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("groupExpanded"), value);
            });
        }
        if (p_groupExpanded != oldValue) {
            Q_EMIT groupExpandedChanged();
        }
    }
    QVariantMap itemVisiable() const {
        return p_itemVisiable;
    }
    void setItemVisiable(const QVariantMap &value) {
        auto oldValue = p_itemVisiable;
        p_itemVisiable = value;
        markPropertySet(1);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("itemVisiable"), value);
            });
        }
        if (p_itemVisiable != oldValue) {
            Q_EMIT itemVisiableChanged();
        }
    }
Q_SIGNALS:
    void groupExpandedChanged();
    void itemVisiableChanged();
private:
    void initialize(DTK_CORE_NAMESPACE::DConfig *config) {
        Q_ASSERT(!m_config.loadRelaxed());
        m_config.storeRelaxed(config);
        if (testPropertySet(0)) {
            config->setValue(QStringLiteral("groupExpanded"), QVariant::fromValue(p_groupExpanded));
        } else {
            updateValue(QStringLiteral("groupExpanded"), QVariant::fromValue(p_groupExpanded));
        }
        if (testPropertySet(1)) {
            config->setValue(QStringLiteral("itemVisiable"), QVariant::fromValue(p_itemVisiable));
        } else {
            updateValue(QStringLiteral("itemVisiable"), QVariant::fromValue(p_itemVisiable));
        }

        connect(config, &DTK_CORE_NAMESPACE::DConfig::valueChanged, this, [this](const QString &key) {
            updateValue(key);
        }, Qt::DirectConnection);
    }
    void updateValue(const QString &key, const QVariant &fallback = QVariant()) {
        Q_ASSERT(QThread::currentThread() == m_config.loadRelaxed()->thread());
        const QVariant &value = m_config.loadRelaxed()->value(key, fallback);
        if (key == QStringLiteral("groupExpanded")) {
            auto newValue = qvariant_cast<QVariantMap>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_groupExpanded != newValue) {
                    p_groupExpanded = newValue;
                    Q_EMIT groupExpandedChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("itemVisiable")) {
            auto newValue = qvariant_cast<QVariantMap>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_itemVisiable != newValue) {
                    p_itemVisiable = newValue;
                    Q_EMIT itemVisiableChanged();
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
    QVariantMap p_groupExpanded { QVariantMap{{QStringLiteral("Group_Bookmark"), QVariant(true)}, {QStringLiteral("Group_Common"), QVariant(true)}, {QStringLiteral("Group_Device"), QVariant(true)}, {QStringLiteral("Group_Network"), QVariant(true)}, {QStringLiteral("Group_Tag"), QVariant(true)}} };
    QVariantMap p_itemVisiable { QVariantMap{{QStringLiteral("builtin_disks"), QVariant(true)}, {QStringLiteral("computer"), QVariant(true)}, {QStringLiteral("computers_in_lan"), QVariant(true)}, {QStringLiteral("desktop"), QVariant(true)}, {QStringLiteral("documents"), QVariant(true)}, {QStringLiteral("downloads"), QVariant(true)}, {QStringLiteral("home"), QVariant(true)}, {QStringLiteral("loop_dev"), QVariant(false)}, {QStringLiteral("mounted_share_dirs"), QVariant(true)}, {QStringLiteral("music"), QVariant(true)}, {QStringLiteral("my_shares"), QVariant(true)}, {QStringLiteral("other_disks"), QVariant(true)}, {QStringLiteral("pictures"), QVariant(true)}, {QStringLiteral("recent"), QVariant(true)}, {QStringLiteral("tags"), QVariant(true)}, {QStringLiteral("trash"), QVariant(true)}, {QStringLiteral("vault"), QVariant(true)}, {QStringLiteral("videos"), QVariant(true)}} };
    QAtomicInteger<quint32> m_propertySetStatus0 = 0;
};

#endif // ORG_DEEPIN_DDE_FILE-MANAGER_SIDEBAR_H
