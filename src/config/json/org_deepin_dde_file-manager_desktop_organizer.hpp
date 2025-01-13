// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORG_DEEPIN_DDE_FILE-MANAGER_DESKTOP_ORGANIZER_H
#define ORG_DEEPIN_DDE_FILE-MANAGER_DESKTOP_ORGANIZER_H

#include <QThread>
#include <QVariant>
#include <QDebug>
#include <QAtomicPointer>
#include <QAtomicInteger>
#include <DConfig>

class org_deepin_dde_file-manager_desktop_organizer : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool collectionMovingOptimize READ collectionMovingOptimize WRITE setCollectionMovingOptimize NOTIFY collectionMovingOptimizeChanged)
    Q_PROPERTY(double enableOrganizer READ enableOrganizer WRITE setEnableOrganizer NOTIFY enableOrganizerChanged)
    Q_PROPERTY(bool enableVisibility READ enableVisibility WRITE setEnableVisibility NOTIFY enableVisibilityChanged)
    Q_PROPERTY(bool hideAllDialogRepeatNoMore READ hideAllDialogRepeatNoMore WRITE setHideAllDialogRepeatNoMore NOTIFY hideAllDialogRepeatNoMoreChanged)
    Q_PROPERTY(QString hideAllKeySeq READ hideAllKeySeq WRITE setHideAllKeySeq NOTIFY hideAllKeySeqChanged)
    Q_PROPERTY(double organizeAction READ organizeAction WRITE setOrganizeAction NOTIFY organizeActionChanged)
    Q_PROPERTY(QList<QVariant> organizeCategories READ organizeCategories WRITE setOrganizeCategories NOTIFY organizeCategoriesChanged)
public:
    explicit org_deepin_dde_file-manager_desktop_organizer(QThread *thread, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_desktop_organizer(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_desktop_organizer(QThread *thread, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_desktop_organizer(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    ~org_deepin_dde_file-manager_desktop_organizer() {
        if (m_config.loadRelaxed()) {
            m_config.loadRelaxed()->deleteLater();
        }
    }

    bool collectionMovingOptimize() const {
        return p_collectionMovingOptimize;
    }
    void setCollectionMovingOptimize(const bool &value) {
        auto oldValue = p_collectionMovingOptimize;
        p_collectionMovingOptimize = value;
        markPropertySet(0);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("collectionMovingOptimize"), value);
            });
        }
        if (p_collectionMovingOptimize != oldValue) {
            Q_EMIT collectionMovingOptimizeChanged();
        }
    }
    double enableOrganizer() const {
        return p_enableOrganizer;
    }
    void setEnableOrganizer(const double &value) {
        auto oldValue = p_enableOrganizer;
        p_enableOrganizer = value;
        markPropertySet(1);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("enableOrganizer"), value);
            });
        }
        if (p_enableOrganizer != oldValue) {
            Q_EMIT enableOrganizerChanged();
        }
    }
    bool enableVisibility() const {
        return p_enableVisibility;
    }
    void setEnableVisibility(const bool &value) {
        auto oldValue = p_enableVisibility;
        p_enableVisibility = value;
        markPropertySet(2);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("enableVisibility"), value);
            });
        }
        if (p_enableVisibility != oldValue) {
            Q_EMIT enableVisibilityChanged();
        }
    }
    bool hideAllDialogRepeatNoMore() const {
        return p_hideAllDialogRepeatNoMore;
    }
    void setHideAllDialogRepeatNoMore(const bool &value) {
        auto oldValue = p_hideAllDialogRepeatNoMore;
        p_hideAllDialogRepeatNoMore = value;
        markPropertySet(3);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("hideAllDialogRepeatNoMore"), value);
            });
        }
        if (p_hideAllDialogRepeatNoMore != oldValue) {
            Q_EMIT hideAllDialogRepeatNoMoreChanged();
        }
    }
    QString hideAllKeySeq() const {
        return p_hideAllKeySeq;
    }
    void setHideAllKeySeq(const QString &value) {
        auto oldValue = p_hideAllKeySeq;
        p_hideAllKeySeq = value;
        markPropertySet(4);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("hideAllKeySeq"), value);
            });
        }
        if (p_hideAllKeySeq != oldValue) {
            Q_EMIT hideAllKeySeqChanged();
        }
    }
    double organizeAction() const {
        return p_organizeAction;
    }
    void setOrganizeAction(const double &value) {
        auto oldValue = p_organizeAction;
        p_organizeAction = value;
        markPropertySet(5);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("organizeAction"), value);
            });
        }
        if (p_organizeAction != oldValue) {
            Q_EMIT organizeActionChanged();
        }
    }
    QList<QVariant> organizeCategories() const {
        return p_organizeCategories;
    }
    void setOrganizeCategories(const QList<QVariant> &value) {
        auto oldValue = p_organizeCategories;
        p_organizeCategories = value;
        markPropertySet(6);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("organizeCategories"), value);
            });
        }
        if (p_organizeCategories != oldValue) {
            Q_EMIT organizeCategoriesChanged();
        }
    }
Q_SIGNALS:
    void collectionMovingOptimizeChanged();
    void enableOrganizerChanged();
    void enableVisibilityChanged();
    void hideAllDialogRepeatNoMoreChanged();
    void hideAllKeySeqChanged();
    void organizeActionChanged();
    void organizeCategoriesChanged();
private:
    void initialize(DTK_CORE_NAMESPACE::DConfig *config) {
        Q_ASSERT(!m_config.loadRelaxed());
        m_config.storeRelaxed(config);
        if (testPropertySet(0)) {
            config->setValue(QStringLiteral("collectionMovingOptimize"), QVariant::fromValue(p_collectionMovingOptimize));
        } else {
            updateValue(QStringLiteral("collectionMovingOptimize"), QVariant::fromValue(p_collectionMovingOptimize));
        }
        if (testPropertySet(1)) {
            config->setValue(QStringLiteral("enableOrganizer"), QVariant::fromValue(p_enableOrganizer));
        } else {
            updateValue(QStringLiteral("enableOrganizer"), QVariant::fromValue(p_enableOrganizer));
        }
        if (testPropertySet(2)) {
            config->setValue(QStringLiteral("enableVisibility"), QVariant::fromValue(p_enableVisibility));
        } else {
            updateValue(QStringLiteral("enableVisibility"), QVariant::fromValue(p_enableVisibility));
        }
        if (testPropertySet(3)) {
            config->setValue(QStringLiteral("hideAllDialogRepeatNoMore"), QVariant::fromValue(p_hideAllDialogRepeatNoMore));
        } else {
            updateValue(QStringLiteral("hideAllDialogRepeatNoMore"), QVariant::fromValue(p_hideAllDialogRepeatNoMore));
        }
        if (testPropertySet(4)) {
            config->setValue(QStringLiteral("hideAllKeySeq"), QVariant::fromValue(p_hideAllKeySeq));
        } else {
            updateValue(QStringLiteral("hideAllKeySeq"), QVariant::fromValue(p_hideAllKeySeq));
        }
        if (testPropertySet(5)) {
            config->setValue(QStringLiteral("organizeAction"), QVariant::fromValue(p_organizeAction));
        } else {
            updateValue(QStringLiteral("organizeAction"), QVariant::fromValue(p_organizeAction));
        }
        if (testPropertySet(6)) {
            config->setValue(QStringLiteral("organizeCategories"), QVariant::fromValue(p_organizeCategories));
        } else {
            updateValue(QStringLiteral("organizeCategories"), QVariant::fromValue(p_organizeCategories));
        }

        connect(config, &DTK_CORE_NAMESPACE::DConfig::valueChanged, this, [this](const QString &key) {
            updateValue(key);
        }, Qt::DirectConnection);
    }
    void updateValue(const QString &key, const QVariant &fallback = QVariant()) {
        Q_ASSERT(QThread::currentThread() == m_config.loadRelaxed()->thread());
        const QVariant &value = m_config.loadRelaxed()->value(key, fallback);
        if (key == QStringLiteral("collectionMovingOptimize")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_collectionMovingOptimize != newValue) {
                    p_collectionMovingOptimize = newValue;
                    Q_EMIT collectionMovingOptimizeChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("enableOrganizer")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_enableOrganizer != newValue) {
                    p_enableOrganizer = newValue;
                    Q_EMIT enableOrganizerChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("enableVisibility")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_enableVisibility != newValue) {
                    p_enableVisibility = newValue;
                    Q_EMIT enableVisibilityChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("hideAllDialogRepeatNoMore")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_hideAllDialogRepeatNoMore != newValue) {
                    p_hideAllDialogRepeatNoMore = newValue;
                    Q_EMIT hideAllDialogRepeatNoMoreChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("hideAllKeySeq")) {
            auto newValue = qvariant_cast<QString>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_hideAllKeySeq != newValue) {
                    p_hideAllKeySeq = newValue;
                    Q_EMIT hideAllKeySeqChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("organizeAction")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_organizeAction != newValue) {
                    p_organizeAction = newValue;
                    Q_EMIT organizeActionChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("organizeCategories")) {
            auto newValue = qvariant_cast<QList<QVariant>>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_organizeCategories != newValue) {
                    p_organizeCategories = newValue;
                    Q_EMIT organizeCategoriesChanged();
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
    bool p_collectionMovingOptimize { true };
    double p_enableOrganizer { 1 };
    bool p_enableVisibility { true };
    bool p_hideAllDialogRepeatNoMore { false };
    QString p_hideAllKeySeq { QStringLiteral("Alt+O") };
    double p_organizeAction { 0 };
    QList<QVariant> p_organizeCategories { QList<QVariant>{QVariant(QStringLiteral("kDocument")), QVariant(QStringLiteral("kPicture")), QVariant(QStringLiteral("kVideo")), QVariant(QStringLiteral("kMusic")), QVariant(QStringLiteral("kFolder"))} };
    QAtomicInteger<quint32> m_propertySetStatus0 = 0;
};

#endif // ORG_DEEPIN_DDE_FILE-MANAGER_DESKTOP_ORGANIZER_H
