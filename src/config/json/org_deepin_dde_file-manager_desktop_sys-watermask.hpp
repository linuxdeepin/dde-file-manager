// SPDX-FileCopyrightText: 2024 - 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ORG_DEEPIN_DDE_FILE-MANAGER_DESKTOP_SYS-WATERMASK_H
#define ORG_DEEPIN_DDE_FILE-MANAGER_DESKTOP_SYS-WATERMASK_H

#include <QThread>
#include <QVariant>
#include <QDebug>
#include <QAtomicPointer>
#include <QAtomicInteger>
#include <DConfig>

class org_deepin_dde_file-manager_desktop_sys-watermask : public QObject {
    Q_OBJECT

    Q_PROPERTY(double logoBottom READ logoBottom WRITE setLogoBottom NOTIFY logoBottomChanged)
    Q_PROPERTY(double logoHeight READ logoHeight WRITE setLogoHeight NOTIFY logoHeightChanged)
    Q_PROPERTY(double logoRight READ logoRight WRITE setLogoRight NOTIFY logoRightChanged)
    Q_PROPERTY(double logoWidth READ logoWidth WRITE setLogoWidth NOTIFY logoWidthChanged)
    Q_PROPERTY(double textAlign READ textAlign WRITE setTextAlign NOTIFY textAlignChanged)
    Q_PROPERTY(QString textColor READ textColor WRITE setTextColor NOTIFY textColorChanged)
    Q_PROPERTY(double textFontSize READ textFontSize WRITE setTextFontSize NOTIFY textFontSizeChanged)
    Q_PROPERTY(double textHeight READ textHeight WRITE setTextHeight NOTIFY textHeightChanged)
    Q_PROPERTY(double textWidth READ textWidth WRITE setTextWidth NOTIFY textWidthChanged)
    Q_PROPERTY(double textXPos READ textXPos WRITE setTextXPos NOTIFY textXPosChanged)
    Q_PROPERTY(double textYPos READ textYPos WRITE setTextYPos NOTIFY textYPosChanged)
public:
    explicit org_deepin_dde_file-manager_desktop_sys-watermask(QThread *thread, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_desktop_sys-watermask(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &appId, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_desktop_sys-watermask(QThread *thread, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    explicit org_deepin_dde_file-manager_desktop_sys-watermask(QThread *thread, DTK_CORE_NAMESPACE::DConfigBackend *backend, const QString &name, const QString &subpath, QObject *parent = nullptr)
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
    ~org_deepin_dde_file-manager_desktop_sys-watermask() {
        if (m_config.loadRelaxed()) {
            m_config.loadRelaxed()->deleteLater();
        }
    }

    double logoBottom() const {
        return p_logoBottom;
    }
    void setLogoBottom(const double &value) {
        auto oldValue = p_logoBottom;
        p_logoBottom = value;
        markPropertySet(0);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("logoBottom"), value);
            });
        }
        if (p_logoBottom != oldValue) {
            Q_EMIT logoBottomChanged();
        }
    }
    double logoHeight() const {
        return p_logoHeight;
    }
    void setLogoHeight(const double &value) {
        auto oldValue = p_logoHeight;
        p_logoHeight = value;
        markPropertySet(1);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("logoHeight"), value);
            });
        }
        if (p_logoHeight != oldValue) {
            Q_EMIT logoHeightChanged();
        }
    }
    double logoRight() const {
        return p_logoRight;
    }
    void setLogoRight(const double &value) {
        auto oldValue = p_logoRight;
        p_logoRight = value;
        markPropertySet(2);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("logoRight"), value);
            });
        }
        if (p_logoRight != oldValue) {
            Q_EMIT logoRightChanged();
        }
    }
    double logoWidth() const {
        return p_logoWidth;
    }
    void setLogoWidth(const double &value) {
        auto oldValue = p_logoWidth;
        p_logoWidth = value;
        markPropertySet(3);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("logoWidth"), value);
            });
        }
        if (p_logoWidth != oldValue) {
            Q_EMIT logoWidthChanged();
        }
    }
    double textAlign() const {
        return p_textAlign;
    }
    void setTextAlign(const double &value) {
        auto oldValue = p_textAlign;
        p_textAlign = value;
        markPropertySet(4);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("textAlign"), value);
            });
        }
        if (p_textAlign != oldValue) {
            Q_EMIT textAlignChanged();
        }
    }
    QString textColor() const {
        return p_textColor;
    }
    void setTextColor(const QString &value) {
        auto oldValue = p_textColor;
        p_textColor = value;
        markPropertySet(5);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("textColor"), value);
            });
        }
        if (p_textColor != oldValue) {
            Q_EMIT textColorChanged();
        }
    }
    double textFontSize() const {
        return p_textFontSize;
    }
    void setTextFontSize(const double &value) {
        auto oldValue = p_textFontSize;
        p_textFontSize = value;
        markPropertySet(6);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("textFontSize"), value);
            });
        }
        if (p_textFontSize != oldValue) {
            Q_EMIT textFontSizeChanged();
        }
    }
    double textHeight() const {
        return p_textHeight;
    }
    void setTextHeight(const double &value) {
        auto oldValue = p_textHeight;
        p_textHeight = value;
        markPropertySet(7);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("textHeight"), value);
            });
        }
        if (p_textHeight != oldValue) {
            Q_EMIT textHeightChanged();
        }
    }
    double textWidth() const {
        return p_textWidth;
    }
    void setTextWidth(const double &value) {
        auto oldValue = p_textWidth;
        p_textWidth = value;
        markPropertySet(8);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("textWidth"), value);
            });
        }
        if (p_textWidth != oldValue) {
            Q_EMIT textWidthChanged();
        }
    }
    double textXPos() const {
        return p_textXPos;
    }
    void setTextXPos(const double &value) {
        auto oldValue = p_textXPos;
        p_textXPos = value;
        markPropertySet(9);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("textXPos"), value);
            });
        }
        if (p_textXPos != oldValue) {
            Q_EMIT textXPosChanged();
        }
    }
    double textYPos() const {
        return p_textYPos;
    }
    void setTextYPos(const double &value) {
        auto oldValue = p_textYPos;
        p_textYPos = value;
        markPropertySet(10);
        if (auto config = m_config.loadRelaxed()) {
            QMetaObject::invokeMethod(config, [this, value]() {
                m_config.loadRelaxed()->setValue(QStringLiteral("textYPos"), value);
            });
        }
        if (p_textYPos != oldValue) {
            Q_EMIT textYPosChanged();
        }
    }
Q_SIGNALS:
    void logoBottomChanged();
    void logoHeightChanged();
    void logoRightChanged();
    void logoWidthChanged();
    void textAlignChanged();
    void textColorChanged();
    void textFontSizeChanged();
    void textHeightChanged();
    void textWidthChanged();
    void textXPosChanged();
    void textYPosChanged();
private:
    void initialize(DTK_CORE_NAMESPACE::DConfig *config) {
        Q_ASSERT(!m_config.loadRelaxed());
        m_config.storeRelaxed(config);
        if (testPropertySet(0)) {
            config->setValue(QStringLiteral("logoBottom"), QVariant::fromValue(p_logoBottom));
        } else {
            updateValue(QStringLiteral("logoBottom"), QVariant::fromValue(p_logoBottom));
        }
        if (testPropertySet(1)) {
            config->setValue(QStringLiteral("logoHeight"), QVariant::fromValue(p_logoHeight));
        } else {
            updateValue(QStringLiteral("logoHeight"), QVariant::fromValue(p_logoHeight));
        }
        if (testPropertySet(2)) {
            config->setValue(QStringLiteral("logoRight"), QVariant::fromValue(p_logoRight));
        } else {
            updateValue(QStringLiteral("logoRight"), QVariant::fromValue(p_logoRight));
        }
        if (testPropertySet(3)) {
            config->setValue(QStringLiteral("logoWidth"), QVariant::fromValue(p_logoWidth));
        } else {
            updateValue(QStringLiteral("logoWidth"), QVariant::fromValue(p_logoWidth));
        }
        if (testPropertySet(4)) {
            config->setValue(QStringLiteral("textAlign"), QVariant::fromValue(p_textAlign));
        } else {
            updateValue(QStringLiteral("textAlign"), QVariant::fromValue(p_textAlign));
        }
        if (testPropertySet(5)) {
            config->setValue(QStringLiteral("textColor"), QVariant::fromValue(p_textColor));
        } else {
            updateValue(QStringLiteral("textColor"), QVariant::fromValue(p_textColor));
        }
        if (testPropertySet(6)) {
            config->setValue(QStringLiteral("textFontSize"), QVariant::fromValue(p_textFontSize));
        } else {
            updateValue(QStringLiteral("textFontSize"), QVariant::fromValue(p_textFontSize));
        }
        if (testPropertySet(7)) {
            config->setValue(QStringLiteral("textHeight"), QVariant::fromValue(p_textHeight));
        } else {
            updateValue(QStringLiteral("textHeight"), QVariant::fromValue(p_textHeight));
        }
        if (testPropertySet(8)) {
            config->setValue(QStringLiteral("textWidth"), QVariant::fromValue(p_textWidth));
        } else {
            updateValue(QStringLiteral("textWidth"), QVariant::fromValue(p_textWidth));
        }
        if (testPropertySet(9)) {
            config->setValue(QStringLiteral("textXPos"), QVariant::fromValue(p_textXPos));
        } else {
            updateValue(QStringLiteral("textXPos"), QVariant::fromValue(p_textXPos));
        }
        if (testPropertySet(10)) {
            config->setValue(QStringLiteral("textYPos"), QVariant::fromValue(p_textYPos));
        } else {
            updateValue(QStringLiteral("textYPos"), QVariant::fromValue(p_textYPos));
        }

        connect(config, &DTK_CORE_NAMESPACE::DConfig::valueChanged, this, [this](const QString &key) {
            updateValue(key);
        }, Qt::DirectConnection);
    }
    void updateValue(const QString &key, const QVariant &fallback = QVariant()) {
        Q_ASSERT(QThread::currentThread() == m_config.loadRelaxed()->thread());
        const QVariant &value = m_config.loadRelaxed()->value(key, fallback);
        if (key == QStringLiteral("logoBottom")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_logoBottom != newValue) {
                    p_logoBottom = newValue;
                    Q_EMIT logoBottomChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("logoHeight")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_logoHeight != newValue) {
                    p_logoHeight = newValue;
                    Q_EMIT logoHeightChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("logoRight")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_logoRight != newValue) {
                    p_logoRight = newValue;
                    Q_EMIT logoRightChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("logoWidth")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_logoWidth != newValue) {
                    p_logoWidth = newValue;
                    Q_EMIT logoWidthChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("textAlign")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_textAlign != newValue) {
                    p_textAlign = newValue;
                    Q_EMIT textAlignChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("textColor")) {
            auto newValue = qvariant_cast<QString>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_textColor != newValue) {
                    p_textColor = newValue;
                    Q_EMIT textColorChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("textFontSize")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_textFontSize != newValue) {
                    p_textFontSize = newValue;
                    Q_EMIT textFontSizeChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("textHeight")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_textHeight != newValue) {
                    p_textHeight = newValue;
                    Q_EMIT textHeightChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("textWidth")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_textWidth != newValue) {
                    p_textWidth = newValue;
                    Q_EMIT textWidthChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("textXPos")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_textXPos != newValue) {
                    p_textXPos = newValue;
                    Q_EMIT textXPosChanged();
                }
            });
            return;
        }
        if (key == QStringLiteral("textYPos")) {
            auto newValue = qvariant_cast<double>(value);
            QMetaObject::invokeMethod(this, [this, newValue]() {
                if (p_textYPos != newValue) {
                    p_textYPos = newValue;
                    Q_EMIT textYPosChanged();
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
    double p_logoBottom { 98 };
    double p_logoHeight { 30 };
    double p_logoRight { 160 };
    double p_logoWidth { 114 };
    double p_textAlign { 65 };
    QString p_textColor { QStringLiteral("#F5F5F5F5") };
    double p_textFontSize { 11 };
    double p_textHeight { 30 };
    double p_textWidth { 100 };
    double p_textXPos { 114 };
    double p_textYPos { 0 };
    QAtomicInteger<quint32> m_propertySetStatus0 = 0;
};

#endif // ORG_DEEPIN_DDE_FILE-MANAGER_DESKTOP_SYS-WATERMASK_H
