// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "propertydialogutil.h"
#include "views/multifilepropertydialog.h"
#include "propertydialogmanager.h"

#include <dfm-base/widgets/filemanagerwindowsmanager.h>
#include <dfm-base/utils/windowutils.h>
#include <dfm-framework/event/event.h>
#include <DArrowLineDrawer>

#include <QApplication>
#include <QScreen>
#include <QTimer>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_propertydialog;
static constexpr int kMaxPropertyDialogNumber { 16 };
static constexpr int kBottomReserveHeight { 40 };
PropertyDialogUtil::PropertyDialogUtil(QObject *parent)
    : QObject(parent)
{
    closeIndicatorTimer = new QTimer(this);
    closeIndicatorTimer->setInterval(1000);
    closeAllDialog = new CloseAllDialog;
    closeAllDialog->setWindowIcon(QIcon::fromTheme("dde-file-manager"));
    connect(closeAllDialog, &CloseAllDialog::allClosed, this, &PropertyDialogUtil::closeAllPropertyDialog);
    connect(&FMWindowsIns, &FileManagerWindowsManager::lastWindowClosed, this, &PropertyDialogUtil::closeAllPropertyDialog);
    connect(closeIndicatorTimer, &QTimer::timeout, this, &PropertyDialogUtil::updateCloseIndicator);
}

PropertyDialogUtil::~PropertyDialogUtil()
{
    filePropertyDialogs.clear();
    customPropertyDialogs.clear();

    if (closeAllDialog) {
        closeAllDialog->deleteLater();
    }
}

void PropertyDialogUtil::showPropertyDialog(const QList<QUrl> &urls, const QVariantHash &option)
{
    bool disableCustomDlg { false };
    if (option.contains(kOption_Key_DisableCustomDialog)) {
        disableCustomDlg = option.value(kOption_Key_DisableCustomDialog).toBool();
    }

    int count = urls.count();
    if (count < kMaxPropertyDialogNumber) {
        QList<QUrl> fileUrls;
        foreach (const QUrl &url, urls) {
            bool ret = dpfHookSequence->run("dfmplugin_propertydialog", "hook_PropertyDialog_Disable", url);
            if (ret)
                continue;

            if (disableCustomDlg || !showCustomDialog(url))
                fileUrls.append(url);
        }
        if (!fileUrls.empty())
            showFilePropertyDialog(fileUrls, option);
    } else {
        MultiFilePropertyDialog *multiFilePropertyDialog = new MultiFilePropertyDialog(urls);
        multiFilePropertyDialog->show();
        multiFilePropertyDialog->moveToCenter();
        multiFilePropertyDialog->raise();
    }
}

void PropertyDialogUtil::showFilePropertyDialog(const QList<QUrl> &urls, const QVariantHash &option)
{
    int count = urls.count();
    for (const QUrl &url : urls) {
        int index = urls.indexOf(url);
        if (!filePropertyDialogs.contains(url)) {
            FilePropertyDialog *dialog = new FilePropertyDialog();
            dialog->selectFileUrl(url);
            dialog->filterControlView();
            filePropertyDialogs.insert(url, dialog);
            if (!option.isEmpty()) {   // The expand state of basic widget mybe ajusted.
                bool expand = option.value(kOption_Key_BasicInfoExpand).toBool();
                dialog->setBasicInfoExpand(expand);
            }
            createControlView(url, option);
            connect(dialog, &FilePropertyDialog::closed, this, &PropertyDialogUtil::closeFilePropertyDialog);
            if (1 == count) {
                QPoint pos = getPropertyPos(dialog->size().width(), dialog->initalHeightOfView());
                dialog->move(pos);
            } else {
                QPoint pos = getPerportyPos(dialog->size().width(), dialog->size().height(), count, index);
                dialog->move(pos);
            }
            dialog->show();
        } else {
            filePropertyDialogs.value(url)->show();
            filePropertyDialogs.value(url)->activateWindow();
        }
    }

    if (urls.count() >= 2) {
        closeAllDialog->show();
        closeIndicatorTimer->start();
    }
}

bool PropertyDialogUtil::showCustomDialog(const QUrl &url)
{
    if (customPropertyDialogs.contains(url)) {
        customPropertyDialogs[url]->show();
        customPropertyDialogs[url]->activateWindow();
        return true;
    } else {
        QWidget *widget = createCustomizeView(url);
        if (widget) {
            customPropertyDialogs.insert(url, widget);
            connect(widget, &QWidget::destroyed, this, [this, url] {
                closeCustomPropertyDialog(url);
            });

            widget->show();
            widget->activateWindow();
            QRect qr = qApp->primaryScreen()->geometry();
            QPoint pt = qr.center();
            pt.setX(pt.x() - widget->width() / 2);
            int height = widget->height();
            QVariant var = widget->property("ForecastDisplayHeight");
            if (var.isValid()) {
                height = var.toInt();
            }
            pt.setY(pt.y() - height / 2);
            widget->move(pt);

            return true;
        }
    }

    return false;
}

/*!
 * \brief           Normal view control extension
 * \param index     Subscript to be inserted
 * \param widget    The view to be inserted
 */
void PropertyDialogUtil::insertExtendedControlFileProperty(const QUrl &url, int index, QWidget *widget)
{
    if (widget) {
        FilePropertyDialog *dialog = nullptr;
        if (filePropertyDialogs.contains(url)) {
            dialog = filePropertyDialogs.value(url);
        } else {
            dialog = new FilePropertyDialog();
        }
        dialog->insertExtendedControl(index, widget);
    }
}

/*!
 * \brief           Normal view control extension
 * \param widget    The view to be inserted
 */
void PropertyDialogUtil::addExtendedControlFileProperty(const QUrl &url, QWidget *widget)
{
    if (widget) {
        FilePropertyDialog *dialog = nullptr;
        if (filePropertyDialogs.contains(url)) {
            dialog = filePropertyDialogs.value(url);
        } else {
            dialog = new FilePropertyDialog();
        }
        dialog->addExtendedControl(widget);
    }
}

void PropertyDialogUtil::closeFilePropertyDialog(const QUrl &url)
{
    if (filePropertyDialogs.contains(url)) {
        filePropertyDialogs.remove(url);
    }

    if (filePropertyDialogs.isEmpty())
        closeAllDialog->close();
}

void PropertyDialogUtil::closeCustomPropertyDialog(const QUrl &url)
{
    if (customPropertyDialogs.contains(url))
        customPropertyDialogs.remove(url);
}

void PropertyDialogUtil::closeAllFilePropertyDialog()
{
    QList<FilePropertyDialog *> dialogs = filePropertyDialogs.values();
    for (FilePropertyDialog *dialog : dialogs) {
        dialog->close();
    }
    closeIndicatorTimer->stop();
    closeAllDialog->close();
}

void PropertyDialogUtil::closeAllPropertyDialog()
{
    closeAllFilePropertyDialog();

    for (auto w : customPropertyDialogs.values())
        w->close();
}

void PropertyDialogUtil::createControlView(const QUrl &url, const QVariantHash &option)
{
    QMap<int, QWidget *> controlView = createView(url, option);
    int count = controlView.keys().count();
    for (int i = 0; i < count; ++i) {
        QWidget *view = controlView.value(controlView.keys()[i]);
        if (controlView.keys()[i] == -1) {
            addExtendedControlFileProperty(url, view);
        } else {
            insertExtendedControlFileProperty(url, controlView.keys()[i], view);
        }
    }
}

void PropertyDialogUtil::updateCloseIndicator()
{
    qint64 size { 0 };
    int fileCount { 0 };

    for (FilePropertyDialog *d : filePropertyDialogs.values()) {
        size += d->getFileSize();
        fileCount += d->getFileCount();
    }

    closeAllDialog->setTotalMessage(size, fileCount);
}

PropertyDialogUtil *PropertyDialogUtil::instance()
{
    static PropertyDialogUtil propertyManager;
    return &propertyManager;
}

QMap<int, QWidget *> PropertyDialogUtil::createView(const QUrl &url, const QVariantHash &option)
{
    return PropertyDialogManager::instance().createExtensionView(url, option);
}

QWidget *PropertyDialogUtil::createCustomizeView(const QUrl &url)
{
    return PropertyDialogManager::instance().createCustomView(url);
}

QPoint PropertyDialogUtil::getPropertyPos(int dialogWidth, int dialogHeight)
{
    const QScreen *cursor_screen = WindowUtils::cursorScreen();

    int x = (cursor_screen->availableSize().width() - dialogWidth) / 2;
    int y = (cursor_screen->availableSize().height() - kBottomReserveHeight - dialogHeight) / 2;

    return QPoint(x, y) + cursor_screen->geometry().topLeft();
}

QPoint PropertyDialogUtil::getPerportyPos(int dialogWidth, int dialogHeight, int count, int index)
{
    Q_UNUSED(dialogHeight)
    const QScreen *cursor_screen = WindowUtils::cursorScreen();

    int desktopWidth = cursor_screen->size().width();
    //    int desktopHeight = cursor_screen->size().height();//后面未用，注释掉
    int SpaceWidth = 20;
    int SpaceHeight = 70;
    int row, x, y;
    int numberPerRow = desktopWidth / (dialogWidth + SpaceWidth);
    Q_ASSERT(numberPerRow != 0);
    if (count % numberPerRow == 0) {
        row = count / numberPerRow;
    } else {
        row = count / numberPerRow + 1;
    }
    Q_UNUSED(row)
    int dialogsWidth;
    if (count / numberPerRow > 0) {
        dialogsWidth = dialogWidth * numberPerRow + SpaceWidth * (numberPerRow - 1);
    } else {
        dialogsWidth = dialogWidth * (count % numberPerRow) + SpaceWidth * (count % numberPerRow - 1);
    }

    //    int dialogsHeight = dialogHeight + SpaceHeight * (row - 1);//未用注释掉

    x = (desktopWidth - dialogsWidth) / 2 + (dialogWidth + SpaceWidth) * (index % numberPerRow);

    y = 5 + (index / numberPerRow) * SpaceHeight;
    return QPoint(x, y) + cursor_screen->geometry().topLeft();
}
