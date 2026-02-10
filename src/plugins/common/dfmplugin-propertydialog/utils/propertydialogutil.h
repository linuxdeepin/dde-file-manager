// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
    Q_DISABLE_COPY(PropertyDialogUtil)

public:
    static PropertyDialogUtil *instance();

public slots:
    void showPropertyDialog(const QList<QUrl> &urls, const QVariantHash &option = QVariantHash());
    void showFilePropertyDialog(const QList<QUrl> &urls, const QVariantHash &option = QVariantHash());
    bool showCustomDialog(const QUrl &url);
    void insertExtendedControlFileProperty(const QUrl &url, int index, QWidget *widget);
    void addExtendedControlFileProperty(const QUrl &url, QWidget *widget);
    void closeFilePropertyDialog(const QUrl &url);
    void closeCustomPropertyDialog(const QUrl &url);
    void closeAllFilePropertyDialog();
    void closeAllPropertyDialog();
    void createControlView(const QUrl &url, const QVariantHash &option = QVariantHash());
    void updateCloseIndicator();

public:
    explicit PropertyDialogUtil(QObject *parent = nullptr);
    virtual ~PropertyDialogUtil() override;

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
    QMap<QUrl, QWidget *> customPropertyDialogs;
    CloseAllDialog *closeAllDialog { nullptr };
    QTimer *closeIndicatorTimer { nullptr };

    //    QMap<QUrl, DevicePropertyDialog *> devicePropertyDialogs;
};
}
#endif   // PROPERTYDIALOGUTIL_H
