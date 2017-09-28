#ifndef DMULTIFILEPROPERTYDIALOG_H
#define DMULTIFILEPROPERTYDIALOG_H

#include "durl.h"
#include "ddialog.h"


#include <tuple>
#include <memory>

#include <QList>
#include <QFrame>
#include <QPainter>
#include <QSharedPointer>
#include <QGraphicsItem>

class QWidget;
class QLabel;
class QVBoxLayout;
class QGridLayout;

class DMultiFilePropertyDialogPrivate;

using namespace  Dtk::Widget;

class DMultiFilePropertyDialog : public DDialog
{
    Q_OBJECT

public:

    explicit DMultiFilePropertyDialog(const QList<DUrl>& urlList, QWidget* const parent = nullptr);
    virtual ~DMultiFilePropertyDialog()=default;


    DMultiFilePropertyDialog(const DMultiFilePropertyDialog&)=delete;
    DMultiFilePropertyDialog& operator=(const DMultiFilePropertyDialog&)=delete;

    void startComputingFolderSize();

    static std::size_t getRandomNumber(const std::pair<std::size_t, std::size_t>& scope);


signals:
    void requestToComputeFlodersSize();

private slots:
   void updateFolderSizeLabel(const quint64& size)noexcept;

private:

   void initConnect();

   static std::pair<std::size_t, std::size_t> getTheQuantityOfFilesAndFolders(const QList<DUrl>& urlList)noexcept;

    QSharedPointer<DMultiFilePropertyDialogPrivate> d_ptr{ nullptr };
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DMultiFilePropertyDialog)

};


#endif // DMULTIFILEPROPERTYDIALOG_H
