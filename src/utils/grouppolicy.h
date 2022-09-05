// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GROUPPOLICY_H
#define GROUPPOLICY_H

#include <dtkcore_global.h>

#define DTK_POLICY_SUPPORT (DTK_VERSION >= DTK_VERSION_CHECK(5, 5, 30, 0))
#if (DTK_POLICY_SUPPORT)
    #include <DConfig>
#define ENABLE_GROUP_POLICY
#endif

#include <QObject>
#include <QVariant>

class GroupPolicy : public QObject
{
    Q_OBJECT
public:
    static GroupPolicy* instance();
    QStringList getKeys();
    bool containKey(const QString &key);
    QVariant getValue(const QString &key, const QVariant &fallback = QVariant());
    void setValue(const QString &key, const QVariant &value);

signals:
    void valueChanged(const QString &key);

protected:
    explicit GroupPolicy(QObject *parent = nullptr);
private:

#if (DTK_POLICY_SUPPORT)
    Dtk::Core::DConfig *m_config;
#endif
};
#endif // GROUPPOLICY_H
