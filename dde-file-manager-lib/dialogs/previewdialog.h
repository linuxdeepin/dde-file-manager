#ifndef PREVIEWDIALOG_H
#define PREVIEWDIALOG_H

#include <QDialog>
#include <QSharedDataPointer>
#include <QLabel>
#include <QFrame>

#include "durl.h"

#include "ddialog.h"

class QGraphicsBlurEffect;

#define DEFAULT_MIN_SIZE QSize(600, 300)

DWIDGET_USE_NAMESPACE
class UnknownPreviewWidget;
class PreviewDialogPrivate;
class QVBoxLayout;
class QHBoxLayout;
class PreviewDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PreviewDialog(const DUrlList& urls = DUrlList(), QWidget *parent = 0);

    void initInterfaces();

    void initUI();
    void initConnections();

signals:

public slots:
    void showForward();
    void showBackward();
    void openFile();

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
//    bool event(QEvent *event) Q_DECL_OVERRIDE;

private:

    void updatePreview(const DUrl& url);
    void updateNavigateButtons();
    void showNavigateButtons();
    void hideNavigateButtons();
    void showUnknownPreview(const DUrl& url);

    void updateDialogGeometry();
//    void updatePopupWidgetsGeometry();

    void registerPreviewWidget(QWidget* w);
    void registerToolbarItem(QWidget* item);

    QSharedPointer<PreviewDialogPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), PreviewDialog)
};

class BlurWidget: public QLabel{
    Q_OBJECT
public:
    explicit BlurWidget(QWidget* parent = 0);

private:
    QGraphicsBlurEffect* m_effect = NULL;
};

class UnknownPreviewWidget: public QFrame{
    Q_OBJECT

public:
    explicit UnknownPreviewWidget(const DUrl& url = DUrl(), QWidget* parent = 0);
    void initUI();

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

private:
    DUrl m_url;
    QLabel* m_nameLabel;
    QLabel* m_imgLabel;
    QString m_fileName;
    QVBoxLayout* m_messageLayout;
    QHBoxLayout* m_mainLayout;

};

#endif // PREVIEWDIALOG_H
