// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DCONFIGMANAGER_H
#define DCONFIGMANAGER_H

#include <dfm-base/dfm_base_global.h>

#include <QObject>
#include <QVariant>

DFMBASE_BEGIN_NAMESPACE

inline constexpr char kDefaultCfgPath[] { "org.deepin.dde.file-manager" };
inline constexpr char kPluginsDConfName[] { "org.deepin.dde.file-manager.plugins" };
inline constexpr char kViewDConfName[] { "org.deepin.dde.file-manager.view" };
inline constexpr char kAnimationDConfName[] {"org.deepin.dde.file-manager.animation"};

inline constexpr char kKeyHideDisk[] { "dfm.disk.hidden" };
inline constexpr char kTreeViewEnable[] { "dfm.treeview.enable" };
inline constexpr char kOpenFolderWindowsInASeparateProcess[] { "dfm.open.in.single.process" };
inline constexpr char kAnimationEnable[] { "dfm.animation.enable" };
inline constexpr char kAnimationEnterDuration[] { "dfm.animation.enter.duration" };
inline constexpr char kAnimationEnterCurve[] { "dfm.animation.enter.curve" };
inline constexpr char kAnimationEnterScale[] { "dfm.animation.enter.scale" };
inline constexpr char kAnimationEnterOpacity[] { "dfm.animation.enter.opacity" };
inline constexpr char kAnimationLayoutDuration[] { "dfm.animation.layout.duration" };
inline constexpr char kAnimationLayoutCurve[] { "dfm.animation.layout.curve" };
inline constexpr char kAnimationDetailviewDuration[] { "dfm.animation.detailview.duration" };
inline constexpr char kAnimationDetailviewCurve[] { "dfm.animation.detailview.curve" };
inline constexpr char kAnimationResortCustomDuration[] { "dd.animation.resort.custom.duration" };
inline constexpr char kAnimationResortCustomCurve[] { "dd.animation.resort.custom.curve" };
inline constexpr char kAnimationResortAlignDuration[] { "dd.animation.resort.align.duration" };
inline constexpr char kAnimationResortAlignCurve[] { "dd.animation.resort.align.curve" };

class DConfigManagerPrivate;
class DConfigManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DConfigManager)

public:
    static DConfigManager *instance();

    bool addConfig(const QString &config, QString *err = nullptr);
    bool removeConfig(const QString &config, QString *err = nullptr);

    QStringList keys(const QString &config) const;
    bool contains(const QString &config, const QString &key) const;
    QVariant value(const QString &config, const QString &key, const QVariant &fallback = QVariant()) const;
    void setValue(const QString &config, const QString &key, const QVariant &value);

    bool validateConfigs(QStringList &invalidConfigs) const;

Q_SIGNALS:
    void valueChanged(const QString &config, const QString &key);

private:
    explicit DConfigManager(QObject *parent = nullptr);
    virtual ~DConfigManager() override;

private:
    QScopedPointer<DConfigManagerPrivate> d;
};

DFMBASE_END_NAMESPACE

#endif   // DCONFIGMANAGER_H
