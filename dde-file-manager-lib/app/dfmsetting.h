/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DFMSETTING_H
#define DFMSETTING_H

#include <QObject>
#include <dtkcore_global.h>
#include "durl.h"
#include <QJsonObject>
#include <QDir>


class DAbstractFileWatcher;

DCORE_BEGIN_NAMESPACE

class DSettings;

DCORE_END_NAMESPACE

DTK_USE_NAMESPACE
class DFMSetting : public QObject
{
    Q_OBJECT

public:

    static DFMSetting* instance();

    explicit DFMSetting(QObject *parent = 0);
    void initConnections();
    QVariant getValueByKey(const QString& key);
    bool isAllwayOpenOnNewWindow();
    int iconSizeIndex();
    int viewMode();
    int openFileAction();
    QString defaultWindowPath();
    QString newTabPath();
    QString getConfigFilePath();
    bool isQuickSearch();
    bool isCompressFilePreview();
    bool isTextFilePreview();
    bool isDocumentFilePreview();
    bool isImageFilePreview();
    bool isVideoFilePreview();
    bool isAutoMount();
    bool isAutoMountAndOpen();
    bool isDefaultChooserDialog();
    bool isShowedHiddenOnSearch();
    bool isShowedHiddenOnView();
    bool isShowedFileSuffix() noexcept;

    QPointer<DTK_CORE_NAMESPACE::DSettings> settings();

signals:
    void showHiddenChanged(bool isShowedHiddenFile);
public slots:
    void onValueChanged(const QString& key, const QVariant& value);

private:
    DTK_CORE_NAMESPACE::DSettings* m_settings;
    QStringList m_newTabOptionPaths;
    QStringList m_defaultWindowOptionPaths;
};

#endif // DFMSETTING_H
