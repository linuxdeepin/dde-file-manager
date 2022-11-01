/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lixiang<lixianga@uniontech.com>
 *
 * Maintainer: lixiang<lixianga@uniontech.com>
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
#ifndef PROPERTYDIALOGUTIL_H
#define PROPERTYDIALOGUTIL_H

#include "dfmplugin_propertydialog_global.h"

#include "views/filepropertydialog.h"
#include "views/closealldialog.h"

#include <QObject>
#include <QMap>

namespace dfmplugin_propertydialog {
class PropertyDialogUtil : public QObject
{
    Q_OBJECT
public:
    explicit PropertyDialogUtil(QObject *parent = nullptr);

    virtual ~PropertyDialogUtil() override;

public slots:

    void showPropertyDialog(const QList<QUrl> &urls, const QVariantHash &option = QVariantHash());

    void showFilePropertyDialog(const QList<QUrl> &urls, const QVariantHash &option = QVariantHash());

    void insertExtendedControlFileProperty(const QUrl &url, int index, QWidget *widget);

    void addExtendedControlFileProperty(const QUrl &url, QWidget *widget);

    void closeFilePropertyDialog(const QUrl url);

    void closeAllFilePropertyDialog();

    void createControlView(const QUrl &url, const QVariantHash &option = QVariantHash());

    void updateCloseIndicator();

public:
    static PropertyDialogUtil *instance();

private:
    /*!
     * \note: ***Used only by the createView property dialog plugin.***
     * \brief Used to create extended control objects.
     */
    //    QMap<int, QWidget *> createView(const QUrl &url);
    QMap<int, QWidget *> createView(const QUrl &url, const QVariantHash &option = QVariantHash());

    QWidget *createCustomizeView(const QUrl &url);

    QPoint getPropertyPos(int dialogWidth, int dialogHeight);

    QPoint getPerportyPos(int dialogWidth, int dialogHeight, int count, int index);

private:
    QMap<QUrl, FilePropertyDialog *> filePropertyDialogs;
    CloseAllDialog *closeAllDialog { nullptr };
    QTimer *closeIndicatorTimer { nullptr };

    //    QMap<QUrl, DevicePropertyDialog *> devicePropertyDialogs;
};
}
#endif   // PROPERTYDIALOGUTIL_H
