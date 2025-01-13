// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORG_DEEPIN_DDE_FILE-MANAGER_ANIMATION_H
#define ORG_DEEPIN_DDE_FILE-MANAGER_ANIMATION_H

#include <QThread>
#include <QVariant>
#include <QDebug>
#include <QAtomicPointer>
#include <QAtomicInteger>
#include <DConfig>

class org_deepin_dde_file-manager_animation : public QObject {
    Q_OBJECT

    Q_PROPERTY(double dd.animation.resort.align.curve READ dd.animation.resort.align.curve WRITE setDd.animation.resort.align.curve NOTIFY dd.animation.resort.align.curveChanged)
    Q_PROPERTY(double dd.animation.resort.align.duration READ dd.animation.resort.align.duration WRITE setDd.animation.resort.align.duration NOTIFY dd.animation.resort.align.durationChanged)
    Q_PROPERTY(double dd.animation.resort.custom.curve READ dd.animation.resort.custom.curve WRITE setDd.animation.resort.custom.curve NOTIFY dd.animation.resort.custom.curveChanged)
    Q_PROPERTY(double dd.animation.resort.custom.duration READ dd.animation.resort.custom.duration WRITE setDd.animation.resort.custom.duration NOTIFY dd.animation.resort.custom.durationChanged)
    Q_PROPERTY(bool dd.animation.resort.enable READ dd.animation.resort.enable WRITE setDd.animation.resort.enable NOTIFY dd.animation.resort.enableChanged)
    Q_PROPERTY(double dfm.animation.detailview.curve READ dfm.animation.detailview.curve WRITE setDfm.animation.detailview.curve NOTIFY dfm.animation.detailview.curveChanged)
    Q_PROPERTY(double dfm.animation.detailview.duration READ dfm.animation.detailview.duration WRITE setDfm.animation.detailview.duration NOTIFY dfm.animation.detailview.durationChanged)
    Q_PROPERTY(bool dfm.animation.detailview.enable READ dfm.animation.detailview.enable WRITE setDfm.animation.detailview.enable NOTIFY dfm.animation.detailview.enableChanged)
    Q_PROPERTY(bool dfm.animation.enable READ dfm.animation.enable WRITE setDfm.animation.enable NOTIFY dfm.animation.enableChanged)
    Q_PROPERTY(double dfm.animation.enter.curve READ dfm.animation.enter.curve WRITE setDfm.animation.enter.curve NOTIFY dfm.animation.enter.curveChanged)
    Q_PROPERTY(double dfm.animation.enter.duration READ dfm.animation.enter.duration WRITE setDfm.animation.enter.duration NOTIFY dfm.animation.enter.durationChanged)
    Q_PROPERTY(bool dfm.animation.enter.enable READ dfm.animation.enter.enable WRITE setDfm.animation.enter.enable NOTIFY dfm.animation.enter.enableChanged)
    Q_PROPERTY(double dfm.animation.enter.opacity READ dfm.animation.enter.opacity WRITE setDfm.animation.enter.opacity NOTIFY dfm.animation.enter.opacityChanged)
    Q_PROPERTY(double dfm.animation.enter.scale READ dfm.animation.enter.scale WRITE setDfm.animation.enter.scale NOTIFY dfm.animation.enter.scaleChanged)
    Q_PROPERTY(double dfm.animation.layout.curve READ dfm.animation.layout.curve WRITE setDfm.animation.layout.curve NOTIFY dfm.animation.layout.curveChanged)
    Q_PROPERTY(double dfm.animation.layout.duration READ dfm.animation.layout.duration WRITE setDfm.animation.layout.duration NOTIFY dfm.animation.layout.durationChanged)
    Q_PROPERTY(bool dfm.animation.layout.enable READ dfm.animation.layout.enable WRITE setDfm.animation.layout.enable NOTIFY dfm.animation.layout.enableChanged)
    Q_PROPERTY(double dfm.animation.sidebar.curve READ dfm.animation.sidebar.curve WRITE setDfm.animation.sidebar.curve NOTIFY dfm.animation.sidebar.curveChanged)
    Q_PROPERTY(double dfm.animation.sidebar.duration READ dfm.animation.sidebar.duration WRITE setDfm.animation.sidebar.duration NOTIFY dfm.animation.sidebar.durationChanged)
    Q_PROPERTY(bool dfm.animation.sidebar.enable READ dfm.animation.sidebar.enable WRITE setDfm.animation.sidebar.enable NOTIFY dfm.animation.sidebar.enableChanged)
public:
    explicit org_deepin_dde_file-manager_animation(QThread *thread, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_animation(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_animation(QThread *thread, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_animation(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    ~org_deepin_dde_file-manager_animation() {
        if (m_config.loadRelaxed()) {
            m_config.loadRelaxed()->deleteLater();
        }
    }

    double dd.animation.resort.align.curve() const {
        return p_dd.animation.resort.align.curve;
    }
    void setDd.animation.resort.align.curve(const double &value) {
        auto oldValue = p_dd.animation.resort.align.curve;
        p_dd.animation.resort.align.curve = value;
        markPropertySet(0);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dd.animation.resort.align.curve"), value);
            });
        }
        if (p_dd.animation.resort.align.curve != oldValue) {
            Q_EMIT dd.animation.resort.align.curveChanged();
        }
    }
    double dd.animation.resort.align.duration() const {
        return p_dd.animation.resort.align.duration;
    }
    void setDd.animation.resort.align.duration(const double &value) {
        auto oldValue = p_dd.animation.resort.align.duration;
        p_dd.animation.resort.align.duration = value;
        markPropertySet(1);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dd.animation.resort.align.duration"), value);
            });
        }
        if (p_dd.animation.resort.align.duration != oldValue) {
            Q_EMIT dd.animation.resort.align.durationChanged();
        }
    }
    double dd.animation.resort.custom.curve() const {
        return p_dd.animation.resort.custom.curve;
    }
    void setDd.animation.resort.custom.curve(const double &value) {
        auto oldValue = p_dd.animation.resort.custom.curve;
        p_dd.animation.resort.custom.curve = value;
        markPropertySet(2);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dd.animation.resort.custom.curve"), value);
            });
        }
        if (p_dd.animation.resort.custom.curve != oldValue) {
            Q_EMIT dd.animation.resort.custom.curveChanged();
        }
    }
    double dd.animation.resort.custom.duration() const {
        return p_dd.animation.resort.custom.duration;
    }
    void setDd.animation.resort.custom.duration(const double &value) {
        auto oldValue = p_dd.animation.resort.custom.duration;
        p_dd.animation.resort.custom.duration = value;
        markPropertySet(3);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dd.animation.resort.custom.duration"), value);
            });
        }
        if (p_dd.animation.resort.custom.duration != oldValue) {
            Q_EMIT dd.animation.resort.custom.durationChanged();
        }
    }
    bool dd.animation.resort.enable() const {
        return p_dd.animation.resort.enable;
    }
    void setDd.animation.resort.enable(const bool &value) {
        auto oldValue = p_dd.animation.resort.enable;
        p_dd.animation.resort.enable = value;
        markPropertySet(4);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dd.animation.resort.enable"), value);
            });
        }
        if (p_dd.animation.resort.enable != oldValue) {
            Q_EMIT dd.animation.resort.enableChanged();
        }
    }
    double dfm.animation.detailview.curve() const {
        return p_dfm.animation.detailview.curve;
    }
    void setDfm.animation.detailview.curve(const double &value) {
        auto oldValue = p_dfm.animation.detailview.curve;
        p_dfm.animation.detailview.curve = value;
        markPropertySet(5);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.animation.detailview.curve"), value);
            });
        }
        if (p_dfm.animation.detailview.curve != oldValue) {
            Q_EMIT dfm.animation.detailview.curveChanged();
        }
    }
    double dfm.animation.detailview.duration() const {
        return p_dfm.animation.detailview.duration;
    }
    void setDfm.animation.detailview.duration(const double &value) {
        auto oldValue = p_dfm.animation.detailview.duration;
        p_dfm.animation.detailview.duration = value;
        markPropertySet(6);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.animation.detailview.duration"), value);
            });
        }
        if (p_dfm.animation.detailview.duration != oldValue) {
            Q_EMIT dfm.animation.detailview.durationChanged();
        }
    }
    bool dfm.animation.detailview.enable() const {
        return p_dfm.animation.detailview.enable;
    }
    void setDfm.animation.detailview.enable(const bool &value) {
        auto oldValue = p_dfm.animation.detailview.enable;
        p_dfm.animation.detailview.enable = value;
        markPropertySet(7);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.animation.detailview.enable"), value);
            });
        }
        if (p_dfm.animation.detailview.enable != oldValue) {
            Q_EMIT dfm.animation.detailview.enableChanged();
        }
    }
    bool dfm.animation.enable() const {
        return p_dfm.animation.enable;
    }
    void setDfm.animation.enable(const bool &value) {
        auto oldValue = p_dfm.animation.enable;
        p_dfm.animation.enable = value;
        markPropertySet(8);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.animation.enable"), value);
            });
        }
        if (p_dfm.animation.enable != oldValue) {
            Q_EMIT dfm.animation.enableChanged();
        }
    }
    double dfm.animation.enter.curve() const {
        return p_dfm.animation.enter.curve;
    }
    void setDfm.animation.enter.curve(const double &value) {
        auto oldValue = p_dfm.animation.enter.curve;
        p_dfm.animation.enter.curve = value;
        markPropertySet(9);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.animation.enter.curve"), value);
            });
        }
        if (p_dfm.animation.enter.curve != oldValue) {
            Q_EMIT dfm.animation.enter.curveChanged();
        }
    }
    double dfm.animation.enter.duration() const {
        return p_dfm.animation.enter.duration;
    }
    void setDfm.animation.enter.duration(const double &value) {
        auto oldValue = p_dfm.animation.enter.duration;
        p_dfm.animation.enter.duration = value;
        markPropertySet(10);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.animation.enter.duration"), value);
            });
        }
        if (p_dfm.animation.enter.duration != oldValue) {
            Q_EMIT dfm.animation.enter.durationChanged();
        }
    }
    bool dfm.animation.enter.enable() const {
        return p_dfm.animation.enter.enable;
    }
    void setDfm.animation.enter.enable(const bool &value) {
        auto oldValue = p_dfm.animation.enter.enable;
        p_dfm.animation.enter.enable = value;
        markPropertySet(11);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.animation.enter.enable"), value);
            });
        }
        if (p_dfm.animation.enter.enable != oldValue) {
            Q_EMIT dfm.animation.enter.enableChanged();
        }
    }
    double dfm.animation.enter.opacity() const {
        return p_dfm.animation.enter.opacity;
    }
    void setDfm.animation.enter.opacity(const double &value) {
        auto oldValue = p_dfm.animation.enter.opacity;
        p_dfm.animation.enter.opacity = value;
        markPropertySet(12);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.animation.enter.opacity"), value);
            });
        }
        if (p_dfm.animation.enter.opacity != oldValue) {
            Q_EMIT dfm.animation.enter.opacityChanged();
        }
    }
    double dfm.animation.enter.scale() const {
        return p_dfm.animation.enter.scale;
    }
    void setDfm.animation.enter.scale(const double &value) {
        auto oldValue = p_dfm.animation.enter.scale;
        p_dfm.animation.enter.scale = value;
        markPropertySet(13);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.animation.enter.scale"), value);
            });
        }
        if (p_dfm.animation.enter.scale != oldValue) {
            Q_EMIT dfm.animation.enter.scaleChanged();
        }
    }
    double dfm.animation.layout.curve() const {
        return p_dfm.animation.layout.curve;
    }
    void setDfm.animation.layout.curve(const double &value) {
        auto oldValue = p_dfm.animation.layout.curve;
        p_dfm.animation.layout.curve = value;
        markPropertySet(14);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.animation.layout.curve"), value);
            });
        }
        if (p_dfm.animation.layout.curve != oldValue) {
            Q_EMIT dfm.animation.layout.curveChanged();
        }
    }
    double dfm.animation.layout.duration() const {
        return p_dfm.animation.layout.duration;
    }
    void setDfm.animation.layout.duration(const double &value) {
        auto oldValue = p_dfm.animation.layout.duration;
        p_dfm.animation.layout.duration = value;
        markPropertySet(15);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.animation.layout.duration"), value);
            });
        }
        if (p_dfm.animation.layout.duration != oldValue) {
            Q_EMIT dfm.animation.layout.durationChanged();
        }
    }
    bool dfm.animation.layout.enable() const {
        return p_dfm.animation.layout.enable;
    }
    void setDfm.animation.layout.enable(const bool &value) {
        auto oldValue = p_dfm.animation.layout.enable;
        p_dfm.animation.layout.enable = value;
        markPropertySet(16);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.animation.layout.enable"), value);
            });
        }
        if (p_dfm.animation.layout.enable != oldValue) {
            Q_EMIT dfm.animation.layout.enableChanged();
        }
    }
    double dfm.animation.sidebar.curve() const {
        return p_dfm.animation.sidebar.curve;
    }
    void setDfm.animation.sidebar.curve(const double &value) {
        auto oldValue = p_dfm.animation.sidebar.curve;
        p_dfm.animation.sidebar.curve = value;
        markPropertySet(17);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.animation.sidebar.curve"), value);
            });
        }
        if (p_dfm.animation.sidebar.curve != oldValue) {
            Q_EMIT dfm.animation.sidebar.curveChanged();
        }
    }
    double dfm.animation.sidebar.duration() const {
        return p_dfm.animation.sidebar.duration;
    }
    void setDfm.animation.sidebar.duration(const double &value) {
        auto oldValue = p_dfm.animation.sidebar.duration;
        p_dfm.animation.sidebar.duration = value;
        markPropertySet(18);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.animation.sidebar.duration"), value);
            });
        }
        if (p_dfm.animation.sidebar.duration != oldValue) {
            Q_EMIT dfm.animation.sidebar.durationChanged();
        }
    }
    bool dfm.animation.sidebar.enable() const {
        return p_dfm.animation.sidebar.enable;
    }
    void setDfm.animation.sidebar.enable(const bool &value) {
        auto oldValue = p_dfm.animation.sidebar.enable;
        p_dfm.animation.sidebar.enable = value;
        markPropertySet(19);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("dfm.animation.sidebar.enable"), value);
            });
        }
        if (p_dfm.animation.sidebar.enable != oldValue) {
            Q_EMIT dfm.animation.sidebar.enableChanged();
        }
    }
Q_SIGNALS:
    void dd.animation.resort.align.curveChanged();
    void dd.animation.resort.align.durationChanged();
    void dd.animation.resort.custom.curveChanged();
    void dd.animation.resort.custom.durationChanged();
    void dd.animation.resort.enableChanged();
    void dfm.animation.detailview.curveChanged();
    void dfm.animation.detailview.durationChanged();
    void dfm.animation.detailview.enableChanged();
    void dfm.animation.enableChanged();
    void dfm.animation.enter.curveChanged();
    void dfm.animation.enter.durationChanged();
    void dfm.animation.enter.enableChanged();
    void dfm.animation.enter.opacityChanged();
    void dfm.animation.enter.scaleChanged();
    void dfm.animation.layout.curveChanged();
    void dfm.animation.layout.durationChanged();
    void dfm.animation.layout.enableChanged();
    void dfm.animation.sidebar.curveChanged();
    void dfm.animation.sidebar.durationChanged();
    void dfm.animation.sidebar.enableChanged();
private:
    void initialize(DTK_CORE_NAMESPACE::DConfig *config) {
        Q_ASSERT(!m_config.loadRelaxed());
        m_config.storeRelaxed(config);
        if (testPropertySet(0)) {
            config->setValue(QStringLiteral("dd.animation.resort.align.curve"), QVariant::fromValue(p_dd.animation.resort.align.curve));
        } else {
            updateValue(QStringLiteral("dd.animation.resort.align.curve"), QVariant::fromValue(p_dd.animation.resort.align.curve));
        }
        if (testPropertySet(1)) {
            config->setValue(QStringLiteral("dd.animation.resort.align.duration"), QVariant::fromValue(p_dd.animation.resort.align.duration));
        } else {
            updateValue(QStringLiteral("dd.animation.resort.align.duration"), QVariant::fromValue(p_dd.animation.resort.align.duration));
        }
        if (testPropertySet(2)) {
            config->setValue(QStringLiteral("dd.animation.resort.custom.curve"), QVariant::fromValue(p_dd.animation.resort.custom.curve));
        } else {
            updateValue(QStringLiteral("dd.animation.resort.custom.curve"), QVariant::fromValue(p_dd.animation.resort.custom.curve));
        }
        if (testPropertySet(3)) {
            config->setValue(QStringLiteral("dd.animation.resort.custom.duration"), QVariant::fromValue(p_dd.animation.resort.custom.duration));
        } else {
            updateValue(QStringLiteral("dd.animation.resort.custom.duration"), QVariant::fromValue(p_dd.animation.resort.custom.duration));
        }
        if (testPropertySet(4)) {
            config->setValue(QStringLiteral("dd.animation.resort.enable"), QVariant::fromValue(p_dd.animation.resort.enable));
        } else {
            updateValue(QStringLiteral("dd.animation.resort.enable"), QVariant::fromValue(p_dd.animation.resort.enable));
        }
        if (testPropertySet(5)) {
            config->setValue(QStringLiteral("dfm.animation.detailview.curve"), QVariant::fromValue(p_dfm.animation.detailview.curve));
        } else {
            updateValue(QStringLiteral("dfm.animation.detailview.curve"), QVariant::fromValue(p_dfm.animation.detailview.curve));
        }
        if (testPropertySet(6)) {
            config->setValue(QStringLiteral("dfm.animation.detailview.duration"), QVariant::fromValue(p_dfm.animation.detailview.duration));
        } else {
            updateValue(QStringLiteral("dfm.animation.detailview.duration"), QVariant::fromValue(p_dfm.animation.detailview.duration));
        }
        if (testPropertySet(7)) {
            config->setValue(QStringLiteral("dfm.animation.detailview.enable"), QVariant::fromValue(p_dfm.animation.detailview.enable));
        } else {
            updateValue(QStringLiteral("dfm.animation.detailview.enable"), QVariant::fromValue(p_dfm.animation.detailview.enable));
        }
        if (testPropertySet(8)) {
            config->setValue(QStringLiteral("dfm.animation.enable"), QVariant::fromValue(p_dfm.animation.enable));
        } else {
            updateValue(QStringLiteral("dfm.animation.enable"), QVariant::fromValue(p_dfm.animation.enable));
        }
        if (testPropertySet(9)) {
            config->setValue(QStringLiteral("dfm.animation.enter.curve"), QVariant::fromValue(p_dfm.animation.enter.curve));
        } else {
            updateValue(QStringLiteral("dfm.animation.enter.curve"), QVariant::fromValue(p_dfm.animation.enter.curve));
        }
        if (testPropertySet(10)) {
            config->setValue(QStringLiteral("dfm.animation.enter.duration"), QVariant::fromValue(p_dfm.animation.enter.duration));
        } else {
            updateValue(QStringLiteral("dfm.animation.enter.duration"), QVariant::fromValue(p_dfm.animation.enter.duration));
        }
        if (testPropertySet(11)) {
            config->setValue(QStringLiteral("dfm.animation.enter.enable"), QVariant::fromValue(p_dfm.animation.enter.enable));
        } else {
            updateValue(QStringLiteral("dfm.animation.enter.enable"), QVariant::fromValue(p_dfm.animation.enter.enable));
        }
        if (testPropertySet(12)) {
            config->setValue(QStringLiteral("dfm.animation.enter.opacity"), QVariant::fromValue(p_dfm.animation.enter.opacity));
        } else {
            updateValue(QStringLiteral("dfm.animation.enter.opacity"), QVariant::fromValue(p_dfm.animation.enter.opacity));
        }
        if (testPropertySet(13)) {
            config->setValue(QStringLiteral("dfm.animation.enter.scale"), QVariant::fromValue(p_dfm.animation.enter.scale));
        } else {
            updateValue(QStringLiteral("dfm.animation.enter.scale"), QVariant::fromValue(p_dfm.animation.enter.scale));
        }
        if (testPropertySet(14)) {
            config->setValue(QStringLiteral("dfm.animation.layout.curve"), QVariant::fromValue(p_dfm.animation.layout.curve));
        } else {
            updateValue(QStringLiteral("dfm.animation.layout.curve"), QVariant::fromValue(p_dfm.animation.layout.curve));
        }
        if (testPropertySet(15)) {
            config->setValue(QStringLiteral("dfm.animation.layout.duration"), QVariant::fromValue(p_dfm.animation.layout.duration));
        } else {
            updateValue(QStringLiteral("dfm.animation.layout.duration"), QVariant::fromValue(p_dfm.animation.layout.duration));
        }
        if (testPropertySet(16)) {
            config->setValue(QStringLiteral("dfm.animation.layout.enable"), QVariant::fromValue(p_dfm.animation.layout.enable));
        } else {
            updateValue(QStringLiteral("dfm.animation.layout.enable"), QVariant::fromValue(p_dfm.animation.layout.enable));
        }
        if (testPropertySet(17)) {
            config->setValue(QStringLiteral("dfm.animation.sidebar.curve"), QVariant::fromValue(p_dfm.animation.sidebar.curve));
        } else {
            updateValue(QStringLiteral("dfm.animation.sidebar.curve"), QVariant::fromValue(p_dfm.animation.sidebar.curve));
        }
        if (testPropertySet(18)) {
            config->setValue(QStringLiteral("dfm.animation.sidebar.duration"), QVariant::fromValue(p_dfm.animation.sidebar.duration));
        } else {
            updateValue(QStringLiteral("dfm.animation.sidebar.duration"), QVariant::fromValue(p_dfm.animation.sidebar.duration));
        }
        if (testPropertySet(19)) {
            config->setValue(QStringLiteral("dfm.animation.sidebar.enable"), QVariant::fromValue(p_dfm.animation.sidebar.enable));
        } else {
            updateValue(QStringLiteral("dfm.animation.sidebar.enable"), QVariant::fromValue(p_dfm.animation.sidebar.enable));
        }

        connect(config, &DTK_CORE_NAMESPACE::DConfig::valueChanged, this, [this](const QString &key) {
            updateValue(key);
        }, Qt::DirectConnection);
    }
    void updateValue(const QString &key, const QVariant &fallback = QVariant()) {
        Q_ASSERT(QThread::currentThread() == m_config.loadRelaxed()->thread());
        const QVariant &value = m_config.loadRelaxed()->value(key, fallback);
        if (key == QStringLiteral("dd.animation.resort.align.curve")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dd.animation.resort.align.curve != newValue) {
                    p_dd.animation.resort.align.curve = newValue;
                    Q_EMIT dd.animation.resort.align.curveChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dd.animation.resort.align.duration")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dd.animation.resort.align.duration != newValue) {
                    p_dd.animation.resort.align.duration = newValue;
                    Q_EMIT dd.animation.resort.align.durationChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dd.animation.resort.custom.curve")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dd.animation.resort.custom.curve != newValue) {
                    p_dd.animation.resort.custom.curve = newValue;
                    Q_EMIT dd.animation.resort.custom.curveChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dd.animation.resort.custom.duration")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dd.animation.resort.custom.duration != newValue) {
                    p_dd.animation.resort.custom.duration = newValue;
                    Q_EMIT dd.animation.resort.custom.durationChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dd.animation.resort.enable")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dd.animation.resort.enable != newValue) {
                    p_dd.animation.resort.enable = newValue;
                    Q_EMIT dd.animation.resort.enableChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.animation.detailview.curve")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.animation.detailview.curve != newValue) {
                    p_dfm.animation.detailview.curve = newValue;
                    Q_EMIT dfm.animation.detailview.curveChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.animation.detailview.duration")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.animation.detailview.duration != newValue) {
                    p_dfm.animation.detailview.duration = newValue;
                    Q_EMIT dfm.animation.detailview.durationChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.animation.detailview.enable")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.animation.detailview.enable != newValue) {
                    p_dfm.animation.detailview.enable = newValue;
                    Q_EMIT dfm.animation.detailview.enableChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.animation.enable")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.animation.enable != newValue) {
                    p_dfm.animation.enable = newValue;
                    Q_EMIT dfm.animation.enableChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.animation.enter.curve")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.animation.enter.curve != newValue) {
                    p_dfm.animation.enter.curve = newValue;
                    Q_EMIT dfm.animation.enter.curveChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.animation.enter.duration")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.animation.enter.duration != newValue) {
                    p_dfm.animation.enter.duration = newValue;
                    Q_EMIT dfm.animation.enter.durationChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.animation.enter.enable")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.animation.enter.enable != newValue) {
                    p_dfm.animation.enter.enable = newValue;
                    Q_EMIT dfm.animation.enter.enableChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.animation.enter.opacity")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.animation.enter.opacity != newValue) {
                    p_dfm.animation.enter.opacity = newValue;
                    Q_EMIT dfm.animation.enter.opacityChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.animation.enter.scale")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.animation.enter.scale != newValue) {
                    p_dfm.animation.enter.scale = newValue;
                    Q_EMIT dfm.animation.enter.scaleChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.animation.layout.curve")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.animation.layout.curve != newValue) {
                    p_dfm.animation.layout.curve = newValue;
                    Q_EMIT dfm.animation.layout.curveChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.animation.layout.duration")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.animation.layout.duration != newValue) {
                    p_dfm.animation.layout.duration = newValue;
                    Q_EMIT dfm.animation.layout.durationChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.animation.layout.enable")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.animation.layout.enable != newValue) {
                    p_dfm.animation.layout.enable = newValue;
                    Q_EMIT dfm.animation.layout.enableChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.animation.sidebar.curve")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.animation.sidebar.curve != newValue) {
                    p_dfm.animation.sidebar.curve = newValue;
                    Q_EMIT dfm.animation.sidebar.curveChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.animation.sidebar.duration")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.animation.sidebar.duration != newValue) {
                    p_dfm.animation.sidebar.duration = newValue;
                    Q_EMIT dfm.animation.sidebar.durationChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("dfm.animation.sidebar.enable")) {
            auto newValue = qvariant_cast<bool>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_dfm.animation.sidebar.enable != newValue) {
                    p_dfm.animation.sidebar.enable = newValue;
                    Q_EMIT dfm.animation.sidebar.enableChanged();
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
    double p_dd.animation.resort.align.curve { 22 };
    double p_dd.animation.resort.align.duration { 366 };
    double p_dd.animation.resort.custom.curve { 22 };
    double p_dd.animation.resort.custom.duration { 366 };
    bool p_dd.animation.resort.enable { true };
    double p_dfm.animation.detailview.curve { 22 };
    double p_dfm.animation.detailview.duration { 366 };
    bool p_dfm.animation.detailview.enable { true };
    bool p_dfm.animation.enable { true };
    double p_dfm.animation.enter.curve { 22 };
    double p_dfm.animation.enter.duration { 200 };
    bool p_dfm.animation.enter.enable { false };
    double p_dfm.animation.enter.opacity { 0 };
    double p_dfm.animation.enter.scale { 0.8 };
    double p_dfm.animation.layout.curve { 22 };
    double p_dfm.animation.layout.duration { 366 };
    bool p_dfm.animation.layout.enable { true };
    double p_dfm.animation.sidebar.curve { 22 };
    double p_dfm.animation.sidebar.duration { 366 };
    bool p_dfm.animation.sidebar.enable { true };
    QAtomicInteger<quint32> m_propertySetStatus0 = 0;
};

#endif // ORG_DEEPIN_DDE_FILE-MANAGER_ANIMATION_H
