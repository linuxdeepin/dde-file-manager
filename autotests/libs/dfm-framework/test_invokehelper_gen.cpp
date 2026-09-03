// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Auto-generated coverage driver: invokes dpf event-framework template
// functions (makeVariantList / packParamsHelper / dispatch / push / send /
// traversal / run / publish) with matching arg-type combinations so their
// instantiations merge with production code in lcov. No production
// callbacks run (handlers/channels/sequences are empty), so this is safe.

#include <dfm-framework/event/invokehelper.h>
#include <dfm-framework/event/event.h>
#include <dfm-framework/event/eventdispatcher.h>
#include <dfm-framework/event/eventchannel.h>
#include <dfm-framework/event/eventsequence.h>
#include <QVariantList>
#include <QUrl>
#include <QString>
#include <QList>
#include <QMap>
#include <QHash>
#include <QWidget>
#include <QFrame>
#include <QPainter>
#include <QMimeData>
#include <QPoint>
#include <QRect>
#include <QRectF>
#include <QModelIndex>
#include <QItemSelection>
#include <QStyleOptionViewItem>
#include <QByteArray>
#include <QFlags>
#include <QDir>
#include <QAbstractItemView>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/dfm_event_defines.h>
#include <dfm-base/interfaces/abstractjobhandler.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/elidetextlayout.h>
#include <utility>
#include <gtest/gtest.h>
using namespace dfmbase;
using namespace dpf;
TEST(InvokeHelperCov, AllTemplates){
    QFrame v_0_QFrame{QFrame()};
    QString v_1_QString{QString()};
    int v_2_int{int(0)};
    QPoint v_3_QPoint{QPoint()};
    QList<QString> v_4_QList_QString_{QList<QString>()};
    QHash<QString, QVariant> v_5_QHash_QString__QVariant_{QHash<QString, QVariant>()};
    QList<QUrl> v_6_QList_QUrl_{QList<QUrl>()};
    QList<QVariant> v_7_QList_QVariant_{QList<QVariant>()};
    bool v_8_bool{false};
    Qt::DropAction v_9_Qt__DropAction{};
    QUrl v_10_QUrl{QUrl()};
    QPainter v_11_QPainter{QPainter()};
    QRectF v_12_QRectF{QRectF()};
    QSharedPointer<dfmbase::FileInfo> v_13_QSharedPointer_dfmbase__FileInfo_{QSharedPointer<dfmbase::FileInfo>()};
    QMap<QString, QVariant> v_14_QMap_QString__QVariant_{QMap<QString, QVariant>()};
    QRect v_15_QRect{QRect()};
    QVariant v_16_QVariant{QVariant()};
    std::function<void (unsigned long long, QUrl const&, std::function<void ()>)> v_17_std__function_void__unsigned_long_long__QUrl_const___std__function_void______{std::function<void (unsigned long long, QUrl const&, std::function<void ()>)>()};
    QList<QMap<QString, QVariant> > v_18_QList_QMap_QString__QVariant___{QList<QMap<QString, QVariant> >()};
    QList<int> v_19_QList_int_{QList<int>()};
    QWidget v_20_QWidget{QWidget()};
    std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)> v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____{std::function<void (QSharedPointer<QMap<dfmbase::AbstractJobHandler::CallbackKey, QVariant> >)>()};
    std::function<void (QWidget*, QUrl const&)> v_22_std__function_void__QWidget___QUrl_const___{std::function<void (QWidget*, QUrl const&)>()};
    QAbstractItemView::SelectionMode v_23_QAbstractItemView__SelectionMode{static_cast<QAbstractItemView::SelectionMode>(0)};
    QFlags<QDir::Filter> v_24_QFlags_QDir__Filter_{QFlags<QDir::Filter>()};
    QItemSelection v_25_QItemSelection{QItemSelection()};
    QList<QAbstractItemView::SelectionMode> v_26_QList_QAbstractItemView__SelectionMode_{QList<QAbstractItemView::SelectionMode>()};
    QFlags<dfmbase::AbstractJobHandler::JobFlag> v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_{QFlags<dfmbase::AbstractJobHandler::JobFlag>()};
    std::pair<QString, QString> v_28_std__pair_QString__QString_{std::pair<QString, QString>()};
    std::pair<QString, dfmbase::AbstractJobHandler::FileNameAddFlag> v_29_std__pair_QString__dfmbase__AbstractJobHandler__FileNameAddFlag_{std::pair<QString, dfmbase::AbstractJobHandler::FileNameAddFlag>()};
    QMap<QUrl, QUrl> v_30_QMap_QUrl__QUrl_{QMap<QUrl, QUrl>()};
    dfmbase::Global::CreateFileType v_31_dfmbase__Global__CreateFileType{dfmbase::Global::CreateFileType::kCreateFileTypeUnknow};
    QFlags<QFileDevice::Permission> v_32_QFlags_QFileDevice__Permission_{QFlags<QFileDevice::Permission>()};
    dfmbase::AbstractJobHandler::JobFlag v_33_dfmbase__AbstractJobHandler__JobFlag{dfmbase::AbstractJobHandler::JobFlag::kNoHint};
    dfmbase::Global::ItemRoles v_34_dfmbase__Global__ItemRoles{dfmbase::Global::ItemRoles::kItemDisplayRole};
    dfmbase::ClipBoard::ClipboardAction v_35_dfmbase__ClipBoard__ClipboardAction{dfmbase::ClipBoard::ClipboardAction(0)};
    std::function<void (QSharedPointer<dfmbase::AbstractJobHandler>)> v_36_std__function_void__QSharedPointer_dfmbase__AbstractJobHandler___{std::function<void (QSharedPointer<dfmbase::AbstractJobHandler>)>()};
    QModelIndex v_37_QModelIndex{QModelIndex()};
    unsigned long long v_38_unsigned_long_long{quint64(0)};
    void *pv_0_void{nullptr};
    QMimeData *pv_1_QMimeData{nullptr};
    Qt::DropAction *pv_2_Qt__DropAction{nullptr};
    dfmbase::ElideTextLayout *pv_3_dfmbase__ElideTextLayout{nullptr};
    QMimeData const *pv_4_QMimeData_const{nullptr};
    QPoint *pv_5_QPoint{nullptr};
    QPainter *pv_6_QPainter{nullptr};
    QStyleOptionViewItem const *pv_7_QStyleOptionViewItem_const{nullptr};
    QWidget *pv_8_QWidget{nullptr};
    QList<QAbstractItemView::SelectionMode> *pv_9_QList_QAbstractItemView__SelectionMode_{nullptr};
    QList<QIcon> *pv_10_QList_QIcon_{nullptr};
    dfmbase::Global::TransparentStatus *pv_11_dfmbase__Global__TransparentStatus{nullptr};
    QVariant *pv_12_QVariant{nullptr};
    QRectF *pv_13_QRectF{nullptr};
    QList<QUrl> *pv_14_QList_QUrl_{nullptr};
    QString *pv_15_QString{nullptr};
    EventDispatcher _disp;
    EventChannel _ch;
    EventSequence _sq;
    auto *_chm = Event::instance()->channel();
    auto *_dm = Event::instance()->dispatcher();
    auto *_sm = Event::instance()->sequence();
    EventType _et(1);
    // makeVariantList
    { QVariantList _l; makeVariantList(&_l, QByteArray()); }
    { QVariantList _l; makeVariantList(&_l, &v_0_QFrame, "xxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, QHash<QString, QVariant>()); }
    { QVariantList _l; makeVariantList(&_l, QList<QString>()); }
    { QVariantList _l; makeVariantList(&_l, QList<QString>(), static_cast<QString const&>(v_1_QString)); }
    { QVariantList _l; makeVariantList(&_l, QList<QString>(), ""); }
    { QVariantList _l; makeVariantList(&_l, QList<QString>(), v_2_int, static_cast<QPoint const&>(v_3_QPoint)); }
    { QVariantList _l; makeVariantList(&_l, &v_4_QList_QString_, pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, QList<QUrl>()); }
    { QVariantList _l; makeVariantList(&_l, QList<QUrl>(), QHash<QString, QVariant>()); }
    { QVariantList _l; makeVariantList(&_l, QList<QUrl>(), static_cast<QHash<QString, QVariant> const&>(v_5_QHash_QString__QVariant_)); }
    { QVariantList _l; makeVariantList(&_l, QList<QUrl>(), v_5_QHash_QString__QVariant_); }
    { QVariantList _l; makeVariantList(&_l, QList<QUrl>(), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QList<QVariant> const&>(v_7_QList_QVariant_), v_8_bool, static_cast<QString const&>(v_1_QString)); }
    { QVariantList _l; makeVariantList(&_l, QList<QUrl>(), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_8_bool, static_cast<QString const&>(v_1_QString)); }
    { QVariantList _l; makeVariantList(&_l, QList<QUrl>(), &v_6_QList_QUrl_); }
    { QVariantList _l; makeVariantList(&_l, QList<QUrl>(), pv_1_QMimeData, pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, QList<QUrl>(), QUrl(), &v_9_Qt__DropAction); }
    { QVariantList _l; makeVariantList(&_l, QList<QUrl>(), static_cast<QUrl const&>(v_10_QUrl)); }
    { QVariantList _l; makeVariantList(&_l, QList<QUrl>(), static_cast<QUrl const&>(v_10_QUrl), pv_2_Qt__DropAction); }
    { QVariantList _l; makeVariantList(&_l, QList<QUrl>(), static_cast<QUrl const&>(v_10_QUrl), v_8_bool); }
    { QVariantList _l; makeVariantList(&_l, QList<QUrl>(), v_10_QUrl); }
    { QVariantList _l; makeVariantList(&_l, QList<QUrl>(), v_10_QUrl, &v_9_Qt__DropAction); }
    { QVariantList _l; makeVariantList(&_l, QList<QUrl>(), v_8_bool, static_cast<QString const&>(v_1_QString)); }
    { QVariantList _l; makeVariantList(&_l, &v_6_QList_QUrl_); }
    { QVariantList _l; makeVariantList(&_l, &v_6_QList_QUrl_, pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, QModelIndex()); }
    { QVariantList _l; makeVariantList(&_l, &v_11_QPainter, static_cast<QRectF const&>(v_12_QRectF), static_cast<QSharedPointer<dfmbase::FileInfo> const&>(v_13_QSharedPointer_dfmbase__FileInfo_)); }
    { QVariantList _l; makeVariantList(&_l, QSharedPointer<dfmbase::FileInfo>(), pv_3_dfmbase__ElideTextLayout); }
    { QVariantList _l; makeVariantList(&_l, QString(), v_4_QList_QString_); }
    { QVariantList _l; makeVariantList(&_l, QString(), v_6_QList_QUrl_); }
    { QVariantList _l; makeVariantList(&_l, QString(), QMap<QString, QVariant>()); }
    { QVariantList _l; makeVariantList(&_l, QString(), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { QVariantList _l; makeVariantList(&_l, QString(), v_14_QMap_QString__QVariant_); }
    { QVariantList _l; makeVariantList(&_l, QString(), pv_4_QMimeData_const, static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, QString(), pv_5_QPoint); }
    { QVariantList _l; makeVariantList(&_l, QString(), v_15_QRect); }
    { QVariantList _l; makeVariantList(&_l, QString(), QString(), QUrl(), QUrl()); }
    { QVariantList _l; makeVariantList(&_l, QString(), static_cast<QString const&>(v_1_QString)); }
    { QVariantList _l; makeVariantList(&_l, QString(), static_cast<QString const&>(v_1_QString), pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, QString(), v_1_QString); }
    { QVariantList _l; makeVariantList(&_l, QString(), &v_1_QString, &v_8_bool); }
    { QVariantList _l; makeVariantList(&_l, QString(), static_cast<QUrl const&>(v_10_QUrl)); }
    { QVariantList _l; makeVariantList(&_l, QString(), static_cast<QUrl const&>(v_10_QUrl), pv_6_QPainter, pv_7_QStyleOptionViewItem_const, pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, QString(), static_cast<QUrl const&>(v_10_QUrl), int(0), false); }
    { QVariantList _l; makeVariantList(&_l, QString(), QVariant()); }
    { QVariantList _l; makeVariantList(&_l, QString(), v_16_QVariant); }
    { QVariantList _l; makeVariantList(&_l, QString(), v_8_bool, pv_8_QWidget); }
    { QVariantList _l; makeVariantList(&_l, QString(), "xxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, QString(), static_cast<dfmbase::Global::DirectoryLoadStrategy>(0)); }
    { QVariantList _l; makeVariantList(&_l, QString(), dfmbase::Global::ViewMode::kIconMode); }
    { QVariantList _l; makeVariantList(&_l, QString(), v_2_int, v_2_int, pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, QString(), v_2_int, pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, QString(), v_17_std__function_void__unsigned_long_long__QUrl_const___std__function_void______); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), pv_9_QList_QAbstractItemView__SelectionMode_); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), pv_10_QList_QIcon_); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), &v_18_QList_QMap_QString__QVariant___); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), static_cast<QList<int> const&>(v_19_QList_int_), pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), v_14_QMap_QString__QVariant_); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), pv_5_QPoint); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), static_cast<QString const&>(v_1_QString)); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), &v_1_QString); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), QUrl()); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), static_cast<QUrl const&>(v_10_QUrl)); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), static_cast<QUrl const&>(v_10_QUrl), pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), &v_10_QUrl); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), false); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), static_cast<bool const&>(v_8_bool)); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), v_8_bool); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), &v_8_bool); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), pv_11_dfmbase__Global__TransparentStatus); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), v_2_int, pv_12_QVariant, pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, QUrl(), pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, &v_10_QUrl); }
    { QVariantList _l; makeVariantList(&_l, QVariant(), QVariant(), QVariant(), int(0)); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, &v_20_QWidget, "xxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, false); }
    { QVariantList _l; makeVariantList(&_l, false, int(0), false); }
    { QVariantList _l; makeVariantList(&_l, false, v_2_int, v_8_bool); }
    { QVariantList _l; makeVariantList(&_l, int(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_4_QList_QString_); }
    { QVariantList _l; makeVariantList(&_l, int(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { QVariantList _l; makeVariantList(&_l, int(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { QVariantList _l; makeVariantList(&_l, int(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { QVariantList _l; makeVariantList(&_l, int(0), v_6_QList_QUrl_, QList<QString>()); }
    { QVariantList _l; makeVariantList(&_l, int(0), v_6_QList_QUrl_, static_cast<QUrl const&>(v_10_QUrl), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { QVariantList _l; makeVariantList(&_l, int(0), v_6_QList_QUrl_, v_10_QUrl, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { QVariantList _l; makeVariantList(&_l, int(0), v_6_QList_QUrl_, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { QVariantList _l; makeVariantList(&_l, int(0), pv_4_QMimeData_const, static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, int(0), pv_4_QMimeData_const, pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, int(0), static_cast<QPoint const&>(v_3_QPoint)); }
    { QVariantList _l; makeVariantList(&_l, int(0), static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, int(0), v_15_QRect); }
    { QVariantList _l; makeVariantList(&_l, int(0), static_cast<QSharedPointer<dfmbase::FileInfo> const&>(v_13_QSharedPointer_dfmbase__FileInfo_), pv_6_QPainter, pv_13_QRectF); }
    { QVariantList _l; makeVariantList(&_l, int(0), static_cast<QString const&>(v_1_QString), pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, int(0), QUrl(), v_14_QMap_QString__QVariant_); }
    { QVariantList _l; makeVariantList(&_l, int(0), static_cast<QUrl const&>(v_10_QUrl)); }
    { QVariantList _l; makeVariantList(&_l, int(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, int(0), static_cast<QUrl const&>(v_10_QUrl), pv_6_QPainter, pv_7_QStyleOptionViewItem_const, pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, int(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), dfmbase::AbstractJobHandler::JobFlag::kNoHint); }
    { QVariantList _l; makeVariantList(&_l, int(0), int(0), int(0), nullptr); }
    { QVariantList _l; makeVariantList(&_l, int(0), v_2_int, pv_14_QList_QUrl_, pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, int(0), v_2_int, static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, int(0), v_2_int, v_2_int, pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, int(0), v_2_int, pv_0_void); }
    { QVariantList _l; makeVariantList(&_l, std::function<QMap<QString, QMultiMap<QString, std::pair<QString, QString> > > (QUrl const&)>(), QString()); }
    { QVariantList _l; makeVariantList(&_l, std::function<QWidget* (QUrl const&)>(), QString()); }
    { QVariantList _l; makeVariantList(&_l, std::function<QWidget* (QUrl const&)>(), v_1_QString, v_2_int); }
    { QVariantList _l; makeVariantList(&_l, std::function<QWidget* (QUrl const&)>(), v_2_int); }
    { QVariantList _l; makeVariantList(&_l, std::function<QWidget* (QUrl const&)>(), v_22_std__function_void__QWidget___QUrl_const___, QString(), int(0)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QAbstractItemView::DragDropMode>(0)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QAbstractItemView::SelectionMode const&>(v_23_QAbstractItemView__SelectionMode)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_24_QFlags_QDir__Filter_); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QItemSelection const&>(v_25_QItemSelection), static_cast<QItemSelection const&>(v_25_QItemSelection)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QAbstractItemView::SelectionMode> const&>(v_26_QList_QAbstractItemView__SelectionMode_)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), QList<QString>()); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_4_QList_QString_); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), QList<QUrl>()); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), QList<QUrl>(), static_cast<dfmbase::AbstractJobHandler::DeleteDialogNoticeType>(0), nullptr); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), QList<QUrl>(), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_), nullptr); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QList<QString> const&>(v_4_QList_QString_)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_4_QList_QString_); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_6_QList_QUrl_); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), QUrl()); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), QUrl(), static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_), nullptr); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl), static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_), nullptr); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), false, v_1_QString); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_8_bool); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_8_bool, v_1_QString); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<dfmbase::AbstractJobHandler::DeleteDialogNoticeType>(0), nullptr); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<std::pair<QString, QString> const&>(v_28_std__pair_QString__QString_), false); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<std::pair<QString, QString> const&>(v_28_std__pair_QString__QString_), static_cast<bool const&>(v_8_bool)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<std::pair<QString, QString> const&>(v_28_std__pair_QString__QString_), static_cast<bool const&>(v_8_bool), v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<std::pair<QString, dfmbase::AbstractJobHandler::FileNameAddFlag> const&>(v_29_std__pair_QString__dfmbase__AbstractJobHandler__FileNameAddFlag_)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<std::pair<QString, dfmbase::AbstractJobHandler::FileNameAddFlag> const&>(v_29_std__pair_QString__dfmbase__AbstractJobHandler__FileNameAddFlag_), v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_6_QList_QUrl_, static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_), nullptr); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_6_QList_QUrl_, static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_), nullptr, QVariant(), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_6_QList_QUrl_, v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_, nullptr, QVariant(), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_6_QList_QUrl_, v_6_QList_QUrl_); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_6_QList_QUrl_, QUrl()); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_6_QList_QUrl_, QUrl(), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_6_QList_QUrl_, QUrl(), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_6_QList_QUrl_, static_cast<QUrl const&>(v_10_QUrl), static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_6_QList_QUrl_, static_cast<QUrl const&>(v_10_QUrl), v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_, nullptr); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_6_QList_QUrl_, v_10_QUrl, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_6_QList_QUrl_, v_10_QUrl, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, nullptr, QVariant(), nullptr); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_6_QList_QUrl_, false, QString()); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_6_QList_QUrl_, false, v_1_QString); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_6_QList_QUrl_, false, "xxxxxxxxxxxxxxxxxxxx"); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_6_QList_QUrl_, v_8_bool, v_1_QString); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_6_QList_QUrl_, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_6_QList_QUrl_, static_cast<std::pair<QString, QString> const&>(v_28_std__pair_QString__QString_), v_8_bool); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_6_QList_QUrl_, static_cast<std::pair<QString, dfmbase::AbstractJobHandler::FileNameAddFlag> const&>(v_29_std__pair_QString__dfmbase__AbstractJobHandler__FileNameAddFlag_)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_30_QMap_QUrl__QUrl_, false, QString()); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_30_QMap_QUrl__QUrl_, false, v_1_QString); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_30_QMap_QUrl__QUrl_, false, ""); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_30_QMap_QUrl__QUrl_, v_8_bool, v_1_QString); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), QString()); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), QString(), v_8_bool); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QString const&>(v_1_QString)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QString const&>(v_1_QString), static_cast<QString const&>(v_1_QString)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QString const&>(v_1_QString), v_8_bool); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_1_QString, v_6_QList_QUrl_); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), pv_15_QString); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), QUrl()); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), QUrl(), static_cast<QUrl const&>(v_10_QUrl), QString(), v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), QUrl(), v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), QUrl(), dfmbase::Global::CreateFileType::kCreateFileTypeUnknow, QString(), QVariant(), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), QUrl(), static_cast<dfmbase::Global::CreateFileType const&>(v_31_dfmbase__Global__CreateFileType), v_1_QString, v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), QUrl(), dfmbase::GlobalEventType(1), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QFlags<QFileDevice::Permission> const&>(v_32_QFlags_QFileDevice__Permission_)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QFlags<QFileDevice::Permission> const&>(v_32_QFlags_QFileDevice__Permission_), &v_8_bool, &v_1_QString); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_8_bool); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), QString(), dfmbase::GlobalEventType(1), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), static_cast<QString const&>(v_1_QString)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), static_cast<bool const&>(v_8_bool), static_cast<bool const&>(v_8_bool)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), dfmbase::AbstractJobHandler::JobFlag::kNoHint); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), static_cast<dfmbase::AbstractJobHandler::JobFlag const&>(v_33_dfmbase__AbstractJobHandler__JobFlag)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QUrl const&>(v_10_QUrl), v_10_QUrl, static_cast<QUrl const&>(v_10_QUrl), static_cast<QString const&>(v_1_QString), static_cast<QVariant const&>(v_16_QVariant), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____, &v_1_QString); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QUrl const&>(v_10_QUrl), v_10_QUrl, static_cast<QVariant const&>(v_16_QVariant), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QUrl const&>(v_10_QUrl), v_10_QUrl, false, false); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QUrl const&>(v_10_QUrl), v_10_QUrl, static_cast<dfmbase::Global::CreateFileType const&>(v_31_dfmbase__Global__CreateFileType), static_cast<QString const&>(v_1_QString), static_cast<QVariant const&>(v_16_QVariant), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____, &v_1_QString); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QUrl const&>(v_10_QUrl), QVariant()); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<dfmbase::Global::CreateFileType const&>(v_31_dfmbase__Global__CreateFileType), static_cast<QString const&>(v_1_QString)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<dfmbase::Global::CreateFileType const&>(v_31_dfmbase__Global__CreateFileType), v_1_QString, dfmbase::GlobalEventType(1), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<dfmbase::Global::ItemRoles const&>(v_34_dfmbase__Global__ItemRoles)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_10_QUrl); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_10_QUrl, QUrl(), false, false); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_10_QUrl, QUrl(), ""); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_10_QUrl, v_10_QUrl); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_10_QUrl, v_10_QUrl, dfmbase::AbstractJobHandler::JobFlag::kNoHint); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_10_QUrl, QVariant()); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_10_QUrl, dfmbase::Global::CreateFileType::kCreateFileTypeUnknow, QString()); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), false); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_8_bool); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_8_bool, v_8_bool); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), nullptr); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), dfmbase::ClipBoard::ClipboardAction(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), dfmbase::ClipBoard::ClipboardAction(0), v_6_QList_QUrl_); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<dfmbase::ClipBoard::ClipboardAction const&>(v_35_dfmbase__ClipBoard__ClipboardAction), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), static_cast<dfmbase::ClipBoard::ClipboardAction const&>(v_35_dfmbase__ClipBoard__ClipboardAction), v_6_QList_QUrl_); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), dfmbase::Global::ItemRoles::kItemDisplayRole); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_34_dfmbase__Global__ItemRoles); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_2_int); }
    { QVariantList _l; makeVariantList(&_l, quint64(0), v_36_std__function_void__QSharedPointer_dfmbase__AbstractJobHandler___); }
    // dispatch
    { _disp.dispatch(QList<QString>()); }
    { _disp.dispatch(QList<QUrl>()); }
    { _disp.dispatch(QList<QUrl>(), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QList<QVariant> const&>(v_7_QList_QVariant_), v_8_bool, static_cast<QString const&>(v_1_QString)); }
    { _disp.dispatch(QList<QUrl>(), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_8_bool, static_cast<QString const&>(v_1_QString)); }
    { _disp.dispatch(QList<QUrl>(), v_8_bool, static_cast<QString const&>(v_1_QString)); }
    { _disp.dispatch(QString(), v_6_QList_QUrl_); }
    { _disp.dispatch(QString(), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _disp.dispatch(QString(), v_14_QMap_QString__QVariant_); }
    { _disp.dispatch(QString(), v_1_QString); }
    { _disp.dispatch(QString(), QVariant()); }
    { _disp.dispatch(QString(), v_16_QVariant); }
    { _disp.dispatch(QUrl(), static_cast<QString const&>(v_1_QString)); }
    { _disp.dispatch(QUrl(), QUrl()); }
    { _disp.dispatch(QUrl(), static_cast<QUrl const&>(v_10_QUrl)); }
    { _disp.dispatch(QUrl(), false); }
    { _disp.dispatch(QUrl(), static_cast<bool const&>(v_8_bool)); }
    { _disp.dispatch(false); }
    { _disp.dispatch(int(0)); }
    { _disp.dispatch(int(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_4_QList_QString_); }
    { _disp.dispatch(int(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _disp.dispatch(int(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _disp.dispatch(int(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _disp.dispatch(int(0), v_6_QList_QUrl_, QList<QString>()); }
    { _disp.dispatch(int(0), v_6_QList_QUrl_, static_cast<QUrl const&>(v_10_QUrl), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _disp.dispatch(int(0), v_6_QList_QUrl_, v_10_QUrl, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _disp.dispatch(int(0), v_6_QList_QUrl_, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _disp.dispatch(int(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), dfmbase::AbstractJobHandler::JobFlag::kNoHint); }
    { _disp.dispatch(quint64(0)); }
    { _disp.dispatch(quint64(0), static_cast<QItemSelection const&>(v_25_QItemSelection), static_cast<QItemSelection const&>(v_25_QItemSelection)); }
    { _disp.dispatch(quint64(0), QList<QUrl>()); }
    { _disp.dispatch(quint64(0), QList<QUrl>(), static_cast<dfmbase::AbstractJobHandler::DeleteDialogNoticeType>(0), nullptr); }
    { _disp.dispatch(quint64(0), QList<QUrl>(), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _disp.dispatch(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    { _disp.dispatch(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_), nullptr); }
    { _disp.dispatch(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QList<QString> const&>(v_4_QList_QString_)); }
    { _disp.dispatch(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_4_QList_QString_); }
    { _disp.dispatch(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), QUrl(), static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_), nullptr); }
    { _disp.dispatch(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl), static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_), nullptr); }
    { _disp.dispatch(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _disp.dispatch(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _disp.dispatch(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), false, v_1_QString); }
    { _disp.dispatch(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_8_bool); }
    { _disp.dispatch(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_8_bool, v_1_QString); }
    { _disp.dispatch(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<dfmbase::AbstractJobHandler::DeleteDialogNoticeType>(0), nullptr); }
    { _disp.dispatch(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _disp.dispatch(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<std::pair<QString, QString> const&>(v_28_std__pair_QString__QString_), false); }
    { _disp.dispatch(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<std::pair<QString, QString> const&>(v_28_std__pair_QString__QString_), static_cast<bool const&>(v_8_bool), v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _disp.dispatch(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<std::pair<QString, dfmbase::AbstractJobHandler::FileNameAddFlag> const&>(v_29_std__pair_QString__dfmbase__AbstractJobHandler__FileNameAddFlag_)); }
    { _disp.dispatch(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<std::pair<QString, dfmbase::AbstractJobHandler::FileNameAddFlag> const&>(v_29_std__pair_QString__dfmbase__AbstractJobHandler__FileNameAddFlag_), v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _disp.dispatch(quint64(0), v_6_QList_QUrl_, static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_), nullptr); }
    { _disp.dispatch(quint64(0), v_6_QList_QUrl_, static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_), nullptr, QVariant(), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _disp.dispatch(quint64(0), v_6_QList_QUrl_, v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_, nullptr, QVariant(), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _disp.dispatch(quint64(0), v_6_QList_QUrl_, QUrl(), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _disp.dispatch(quint64(0), v_6_QList_QUrl_, QUrl(), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _disp.dispatch(quint64(0), v_6_QList_QUrl_, static_cast<QUrl const&>(v_10_QUrl), v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_, nullptr); }
    { _disp.dispatch(quint64(0), v_6_QList_QUrl_, v_10_QUrl, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _disp.dispatch(quint64(0), v_6_QList_QUrl_, v_10_QUrl, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, nullptr, QVariant(), nullptr); }
    { _disp.dispatch(quint64(0), v_6_QList_QUrl_, false, QString()); }
    { _disp.dispatch(quint64(0), v_6_QList_QUrl_, false, v_1_QString); }
    { _disp.dispatch(quint64(0), v_6_QList_QUrl_, false, "xxxxxxxxxxxxxxxxxxxx"); }
    { _disp.dispatch(quint64(0), v_6_QList_QUrl_, v_8_bool, v_1_QString); }
    { _disp.dispatch(quint64(0), v_6_QList_QUrl_, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _disp.dispatch(quint64(0), v_6_QList_QUrl_, static_cast<std::pair<QString, QString> const&>(v_28_std__pair_QString__QString_), v_8_bool); }
    { _disp.dispatch(quint64(0), v_6_QList_QUrl_, static_cast<std::pair<QString, dfmbase::AbstractJobHandler::FileNameAddFlag> const&>(v_29_std__pair_QString__dfmbase__AbstractJobHandler__FileNameAddFlag_)); }
    { _disp.dispatch(quint64(0), v_30_QMap_QUrl__QUrl_, false, QString()); }
    { _disp.dispatch(quint64(0), v_30_QMap_QUrl__QUrl_, false, v_1_QString); }
    { _disp.dispatch(quint64(0), v_30_QMap_QUrl__QUrl_, false, ""); }
    { _disp.dispatch(quint64(0), v_30_QMap_QUrl__QUrl_, v_8_bool, v_1_QString); }
    { _disp.dispatch(quint64(0), QString()); }
    { _disp.dispatch(quint64(0), static_cast<QString const&>(v_1_QString)); }
    { _disp.dispatch(quint64(0), static_cast<QString const&>(v_1_QString), static_cast<QString const&>(v_1_QString)); }
    { _disp.dispatch(quint64(0), v_1_QString, v_6_QList_QUrl_); }
    { _disp.dispatch(quint64(0), pv_15_QString); }
    { _disp.dispatch(quint64(0), QUrl()); }
    { _disp.dispatch(quint64(0), QUrl(), static_cast<QUrl const&>(v_10_QUrl), QString(), v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _disp.dispatch(quint64(0), QUrl(), v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _disp.dispatch(quint64(0), QUrl(), dfmbase::Global::CreateFileType::kCreateFileTypeUnknow, QString(), QVariant(), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _disp.dispatch(quint64(0), QUrl(), static_cast<dfmbase::Global::CreateFileType const&>(v_31_dfmbase__Global__CreateFileType), v_1_QString, v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _disp.dispatch(quint64(0), QUrl(), dfmbase::GlobalEventType(1), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _disp.dispatch(quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QFlags<QFileDevice::Permission> const&>(v_32_QFlags_QFileDevice__Permission_)); }
    { _disp.dispatch(quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_8_bool); }
    { _disp.dispatch(quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_)); }
    { _disp.dispatch(quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), QString(), dfmbase::GlobalEventType(1), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _disp.dispatch(quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), static_cast<QString const&>(v_1_QString)); }
    { _disp.dispatch(quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), dfmbase::AbstractJobHandler::JobFlag::kNoHint); }
    { _disp.dispatch(quint64(0), static_cast<QUrl const&>(v_10_QUrl), v_10_QUrl, false, false); }
    { _disp.dispatch(quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<dfmbase::Global::CreateFileType const&>(v_31_dfmbase__Global__CreateFileType), static_cast<QString const&>(v_1_QString)); }
    { _disp.dispatch(quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<dfmbase::Global::CreateFileType const&>(v_31_dfmbase__Global__CreateFileType), v_1_QString, dfmbase::GlobalEventType(1), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _disp.dispatch(quint64(0), v_10_QUrl); }
    { _disp.dispatch(quint64(0), v_10_QUrl, QUrl(), false, false); }
    { _disp.dispatch(quint64(0), v_10_QUrl, QUrl(), ""); }
    { _disp.dispatch(quint64(0), v_10_QUrl, v_10_QUrl, dfmbase::AbstractJobHandler::JobFlag::kNoHint); }
    { _disp.dispatch(quint64(0), v_10_QUrl, dfmbase::Global::CreateFileType::kCreateFileTypeUnknow, QString()); }
    { _disp.dispatch(quint64(0), v_8_bool); }
    { _disp.dispatch(quint64(0), nullptr); }
    { _disp.dispatch(quint64(0), dfmbase::ClipBoard::ClipboardAction(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    { _disp.dispatch(quint64(0), dfmbase::ClipBoard::ClipboardAction(0), v_6_QList_QUrl_); }
    { _disp.dispatch(quint64(0), static_cast<dfmbase::ClipBoard::ClipboardAction const&>(v_35_dfmbase__ClipBoard__ClipboardAction), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    { _disp.dispatch(quint64(0), static_cast<dfmbase::ClipBoard::ClipboardAction const&>(v_35_dfmbase__ClipBoard__ClipboardAction), v_6_QList_QUrl_); }
    { _disp.dispatch(quint64(0), v_36_std__function_void__QSharedPointer_dfmbase__AbstractJobHandler___); }
    // push (EventType variant)
    { _chm->push(_et, QByteArray()); }
    { _chm->push(_et, &v_0_QFrame, "xxxxxxxxxx"); }
    { _chm->push(_et, QHash<QString, QVariant>()); }
    { _chm->push(_et, QList<QString>(), static_cast<QString const&>(v_1_QString)); }
    { _chm->push(_et, QList<QString>(), ""); }
    { _chm->push(_et, QList<QString>(), v_2_int, static_cast<QPoint const&>(v_3_QPoint)); }
    { _chm->push(_et, static_cast<QList<QString> const&>(v_4_QList_QString_), static_cast<QString const&>(v_1_QString)); }
    { _chm->push(_et, QList<QUrl>()); }
    { _chm->push(_et, QList<QUrl>(), QHash<QString, QVariant>()); }
    { _chm->push(_et, QList<QUrl>(), static_cast<QHash<QString, QVariant> const&>(v_5_QHash_QString__QVariant_)); }
    { _chm->push(_et, QList<QUrl>(), v_5_QHash_QString__QVariant_); }
    { _chm->push(_et, QList<QUrl>(), static_cast<QUrl const&>(v_10_QUrl), v_8_bool); }
    { _chm->push(_et, static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QHash<QString, QVariant> const&>(v_5_QHash_QString__QVariant_)); }
    { _chm->push(_et, QMap<QString, QVariant>()); }
    { _chm->push(_et, static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _chm->push(_et, v_14_QMap_QString__QVariant_); }
    { _chm->push(_et, QModelIndex()); }
    { _chm->push(_et, static_cast<QModelIndex const&>(v_37_QModelIndex)); }
    { _chm->push(_et, &v_11_QPainter, static_cast<QRectF const&>(v_12_QRectF), static_cast<QSharedPointer<dfmbase::FileInfo> const&>(v_13_QSharedPointer_dfmbase__FileInfo_)); }
    { _chm->push(_et, QString()); }
    { _chm->push(_et, QString(), v_4_QList_QString_); }
    { _chm->push(_et, QString(), QMap<QString, QVariant>()); }
    { _chm->push(_et, QString(), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _chm->push(_et, QString(), v_14_QMap_QString__QVariant_); }
    { _chm->push(_et, QString(), pv_5_QPoint); }
    { _chm->push(_et, QString(), v_15_QRect); }
    { _chm->push(_et, QString(), static_cast<QString const&>(v_1_QString)); }
    { _chm->push(_et, QString(), v_1_QString); }
    { _chm->push(_et, QString(), static_cast<QUrl const&>(v_10_QUrl)); }
    { _chm->push(_et, QString(), static_cast<QUrl const&>(v_10_QUrl), int(0), false); }
    { _chm->push(_et, QString(), v_8_bool, pv_8_QWidget); }
    { _chm->push(_et, QString(), "xxxxxxxxxxxx"); }
    { _chm->push(_et, QString(), static_cast<dfmbase::Global::DirectoryLoadStrategy>(0)); }
    { _chm->push(_et, QString(), dfmbase::Global::ViewMode::kIconMode); }
    { _chm->push(_et, QString(), v_17_std__function_void__unsigned_long_long__QUrl_const___std__function_void______); }
    { _chm->push(_et, static_cast<QString const&>(v_1_QString)); }
    { _chm->push(_et, static_cast<QString const&>(v_1_QString), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _chm->push(_et, static_cast<QString const&>(v_1_QString), static_cast<QString const&>(v_1_QString)); }
    { _chm->push(_et, QUrl()); }
    { _chm->push(_et, QUrl(), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _chm->push(_et, QUrl(), v_14_QMap_QString__QVariant_); }
    { _chm->push(_et, QUrl(), pv_5_QPoint); }
    { _chm->push(_et, QUrl(), static_cast<QString const&>(v_1_QString)); }
    { _chm->push(_et, QUrl(), v_8_bool); }
    { _chm->push(_et, static_cast<QUrl const&>(v_10_QUrl)); }
    { _chm->push(_et, static_cast<QUrl const&>(v_10_QUrl), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _chm->push(_et, QVariant(), QVariant(), QVariant(), int(0)); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(_et, &v_20_QWidget, "xxxxxxx"); }
    { _chm->push(_et, false); }
    { _chm->push(_et, false, int(0), false); }
    { _chm->push(_et, false, v_2_int, v_8_bool); }
    { _chm->push(_et, static_cast<char const*>(nullptr)); }
    { _chm->push(_et, int(0), static_cast<QPoint const&>(v_3_QPoint)); }
    { _chm->push(_et, int(0), v_15_QRect); }
    { _chm->push(_et, int(0), QUrl(), v_14_QMap_QString__QVariant_); }
    { _chm->push(_et, int(0), static_cast<QUrl const&>(v_10_QUrl)); }
    { _chm->push(_et, v_2_int); }
    { _chm->push(_et, v_2_int, static_cast<QPoint const&>(v_3_QPoint)); }
    { _chm->push(_et, std::function<QMap<QString, QMultiMap<QString, std::pair<QString, QString> > > (QUrl const&)>(), QString()); }
    { _chm->push(_et, std::function<QWidget* (QUrl const&)>(), QString()); }
    { _chm->push(_et, std::function<QWidget* (QUrl const&)>(), v_1_QString, v_2_int); }
    { _chm->push(_et, std::function<QWidget* (QUrl const&)>(), v_2_int); }
    { _chm->push(_et, std::function<QWidget* (QUrl const&)>(), v_22_std__function_void__QWidget___QUrl_const___, QString(), int(0)); }
    { _chm->push(_et, quint64(0)); }
    { _chm->push(_et, quint64(0), static_cast<QAbstractItemView::DragDropMode>(0)); }
    { _chm->push(_et, quint64(0), static_cast<QAbstractItemView::SelectionMode const&>(v_23_QAbstractItemView__SelectionMode)); }
    { _chm->push(_et, quint64(0), v_24_QFlags_QDir__Filter_); }
    { _chm->push(_et, quint64(0), static_cast<QList<QAbstractItemView::SelectionMode> const&>(v_26_QList_QAbstractItemView__SelectionMode_)); }
    { _chm->push(_et, quint64(0), QList<QString>()); }
    { _chm->push(_et, quint64(0), v_4_QList_QString_); }
    { _chm->push(_et, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    { _chm->push(_et, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    { _chm->push(_et, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_6_QList_QUrl_); }
    { _chm->push(_et, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl)); }
    { _chm->push(_et, quint64(0), v_6_QList_QUrl_); }
    { _chm->push(_et, quint64(0), v_6_QList_QUrl_, v_6_QList_QUrl_); }
    { _chm->push(_et, quint64(0), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _chm->push(_et, quint64(0), QString()); }
    { _chm->push(_et, quint64(0), QString(), v_8_bool); }
    { _chm->push(_et, quint64(0), static_cast<QString const&>(v_1_QString)); }
    { _chm->push(_et, quint64(0), static_cast<QString const&>(v_1_QString), v_8_bool); }
    { _chm->push(_et, quint64(0), static_cast<QUrl const&>(v_10_QUrl)); }
    { _chm->push(_et, quint64(0), static_cast<QUrl const&>(v_10_QUrl), QVariant()); }
    { _chm->push(_et, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<dfmbase::Global::ItemRoles const&>(v_34_dfmbase__Global__ItemRoles)); }
    { _chm->push(_et, quint64(0), v_10_QUrl); }
    { _chm->push(_et, quint64(0), v_10_QUrl, QVariant()); }
    { _chm->push(_et, quint64(0), false); }
    { _chm->push(_et, quint64(0), v_8_bool); }
    { _chm->push(_et, quint64(0), v_8_bool, v_8_bool); }
    { _chm->push(_et, quint64(0), dfmbase::Global::ItemRoles::kItemDisplayRole); }
    { _chm->push(_et, quint64(0), v_34_dfmbase__Global__ItemRoles); }
    { _chm->push(_et, quint64(0), v_2_int); }
    { _chm->push(_et, static_cast<unsigned long long const&>(v_38_unsigned_long_long), v_34_dfmbase__Global__ItemRoles); }
    // push (space+topic variant, topic starts with "slot")
    { _chm->push(QString("x"), QString("slot_t"), QByteArray()); }
    { _chm->push(QString("x"), QString("slot_t"), &v_0_QFrame, "xxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), QHash<QString, QVariant>()); }
    { _chm->push(QString("x"), QString("slot_t"), QList<QString>(), static_cast<QString const&>(v_1_QString)); }
    { _chm->push(QString("x"), QString("slot_t"), QList<QString>(), ""); }
    { _chm->push(QString("x"), QString("slot_t"), QList<QString>(), v_2_int, static_cast<QPoint const&>(v_3_QPoint)); }
    { _chm->push(QString("x"), QString("slot_t"), static_cast<QList<QString> const&>(v_4_QList_QString_), static_cast<QString const&>(v_1_QString)); }
    { _chm->push(QString("x"), QString("slot_t"), QList<QUrl>()); }
    { _chm->push(QString("x"), QString("slot_t"), QList<QUrl>(), QHash<QString, QVariant>()); }
    { _chm->push(QString("x"), QString("slot_t"), QList<QUrl>(), static_cast<QHash<QString, QVariant> const&>(v_5_QHash_QString__QVariant_)); }
    { _chm->push(QString("x"), QString("slot_t"), QList<QUrl>(), v_5_QHash_QString__QVariant_); }
    { _chm->push(QString("x"), QString("slot_t"), QList<QUrl>(), static_cast<QUrl const&>(v_10_QUrl), v_8_bool); }
    { _chm->push(QString("x"), QString("slot_t"), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QHash<QString, QVariant> const&>(v_5_QHash_QString__QVariant_)); }
    { _chm->push(QString("x"), QString("slot_t"), QMap<QString, QVariant>()); }
    { _chm->push(QString("x"), QString("slot_t"), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _chm->push(QString("x"), QString("slot_t"), v_14_QMap_QString__QVariant_); }
    { _chm->push(QString("x"), QString("slot_t"), QModelIndex()); }
    { _chm->push(QString("x"), QString("slot_t"), static_cast<QModelIndex const&>(v_37_QModelIndex)); }
    { _chm->push(QString("x"), QString("slot_t"), &v_11_QPainter, static_cast<QRectF const&>(v_12_QRectF), static_cast<QSharedPointer<dfmbase::FileInfo> const&>(v_13_QSharedPointer_dfmbase__FileInfo_)); }
    { _chm->push(QString("x"), QString("slot_t"), QString()); }
    { _chm->push(QString("x"), QString("slot_t"), QString(), v_4_QList_QString_); }
    { _chm->push(QString("x"), QString("slot_t"), QString(), QMap<QString, QVariant>()); }
    { _chm->push(QString("x"), QString("slot_t"), QString(), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _chm->push(QString("x"), QString("slot_t"), QString(), v_14_QMap_QString__QVariant_); }
    { _chm->push(QString("x"), QString("slot_t"), QString(), pv_5_QPoint); }
    { _chm->push(QString("x"), QString("slot_t"), QString(), v_15_QRect); }
    { _chm->push(QString("x"), QString("slot_t"), QString(), static_cast<QString const&>(v_1_QString)); }
    { _chm->push(QString("x"), QString("slot_t"), QString(), v_1_QString); }
    { _chm->push(QString("x"), QString("slot_t"), QString(), static_cast<QUrl const&>(v_10_QUrl)); }
    { _chm->push(QString("x"), QString("slot_t"), QString(), static_cast<QUrl const&>(v_10_QUrl), int(0), false); }
    { _chm->push(QString("x"), QString("slot_t"), QString(), v_8_bool, pv_8_QWidget); }
    { _chm->push(QString("x"), QString("slot_t"), QString(), "xxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), QString(), static_cast<dfmbase::Global::DirectoryLoadStrategy>(0)); }
    { _chm->push(QString("x"), QString("slot_t"), QString(), dfmbase::Global::ViewMode::kIconMode); }
    { _chm->push(QString("x"), QString("slot_t"), QString(), v_17_std__function_void__unsigned_long_long__QUrl_const___std__function_void______); }
    { _chm->push(QString("x"), QString("slot_t"), static_cast<QString const&>(v_1_QString)); }
    { _chm->push(QString("x"), QString("slot_t"), static_cast<QString const&>(v_1_QString), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _chm->push(QString("x"), QString("slot_t"), static_cast<QString const&>(v_1_QString), static_cast<QString const&>(v_1_QString)); }
    { _chm->push(QString("x"), QString("slot_t"), QUrl()); }
    { _chm->push(QString("x"), QString("slot_t"), QUrl(), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _chm->push(QString("x"), QString("slot_t"), QUrl(), v_14_QMap_QString__QVariant_); }
    { _chm->push(QString("x"), QString("slot_t"), QUrl(), pv_5_QPoint); }
    { _chm->push(QString("x"), QString("slot_t"), QUrl(), static_cast<QString const&>(v_1_QString)); }
    { _chm->push(QString("x"), QString("slot_t"), QUrl(), v_8_bool); }
    { _chm->push(QString("x"), QString("slot_t"), static_cast<QUrl const&>(v_10_QUrl)); }
    { _chm->push(QString("x"), QString("slot_t"), static_cast<QUrl const&>(v_10_QUrl), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _chm->push(QString("x"), QString("slot_t"), QVariant(), QVariant(), QVariant(), int(0)); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), &v_20_QWidget, "xxxxxxx"); }
    { _chm->push(QString("x"), QString("slot_t"), false); }
    { _chm->push(QString("x"), QString("slot_t"), false, int(0), false); }
    { _chm->push(QString("x"), QString("slot_t"), false, v_2_int, v_8_bool); }
    { _chm->push(QString("x"), QString("slot_t"), static_cast<char const*>(nullptr)); }
    { _chm->push(QString("x"), QString("slot_t"), int(0), static_cast<QPoint const&>(v_3_QPoint)); }
    { _chm->push(QString("x"), QString("slot_t"), int(0), v_15_QRect); }
    { _chm->push(QString("x"), QString("slot_t"), int(0), QUrl(), v_14_QMap_QString__QVariant_); }
    { _chm->push(QString("x"), QString("slot_t"), int(0), static_cast<QUrl const&>(v_10_QUrl)); }
    { _chm->push(QString("x"), QString("slot_t"), v_2_int); }
    { _chm->push(QString("x"), QString("slot_t"), v_2_int, static_cast<QPoint const&>(v_3_QPoint)); }
    { _chm->push(QString("x"), QString("slot_t"), std::function<QMap<QString, QMultiMap<QString, std::pair<QString, QString> > > (QUrl const&)>(), QString()); }
    { _chm->push(QString("x"), QString("slot_t"), std::function<QWidget* (QUrl const&)>(), QString()); }
    { _chm->push(QString("x"), QString("slot_t"), std::function<QWidget* (QUrl const&)>(), v_1_QString, v_2_int); }
    { _chm->push(QString("x"), QString("slot_t"), std::function<QWidget* (QUrl const&)>(), v_2_int); }
    { _chm->push(QString("x"), QString("slot_t"), std::function<QWidget* (QUrl const&)>(), v_22_std__function_void__QWidget___QUrl_const___, QString(), int(0)); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0)); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), static_cast<QAbstractItemView::DragDropMode>(0)); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), static_cast<QAbstractItemView::SelectionMode const&>(v_23_QAbstractItemView__SelectionMode)); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), v_24_QFlags_QDir__Filter_); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), static_cast<QList<QAbstractItemView::SelectionMode> const&>(v_26_QList_QAbstractItemView__SelectionMode_)); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), QList<QString>()); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), v_4_QList_QString_); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_6_QList_QUrl_); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl)); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), v_6_QList_QUrl_); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), v_6_QList_QUrl_, v_6_QList_QUrl_); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), QString()); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), QString(), v_8_bool); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), static_cast<QString const&>(v_1_QString)); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), static_cast<QString const&>(v_1_QString), v_8_bool); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), static_cast<QUrl const&>(v_10_QUrl)); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), static_cast<QUrl const&>(v_10_QUrl), QVariant()); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<dfmbase::Global::ItemRoles const&>(v_34_dfmbase__Global__ItemRoles)); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), v_10_QUrl); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), v_10_QUrl, QVariant()); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), false); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), v_8_bool); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), v_8_bool, v_8_bool); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), dfmbase::Global::ItemRoles::kItemDisplayRole); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), v_34_dfmbase__Global__ItemRoles); }
    { _chm->push(QString("x"), QString("slot_t"), quint64(0), v_2_int); }
    { _chm->push(QString("x"), QString("slot_t"), static_cast<unsigned long long const&>(v_38_unsigned_long_long), v_34_dfmbase__Global__ItemRoles); }
    // send (direct EventChannel)
    { _ch.send(QByteArray()); }
    { _ch.send(&v_0_QFrame, "xxxxxxxxxx"); }
    { _ch.send(QHash<QString, QVariant>()); }
    { _ch.send(QList<QString>(), static_cast<QString const&>(v_1_QString)); }
    { _ch.send(QList<QString>(), ""); }
    { _ch.send(QList<QString>(), v_2_int, static_cast<QPoint const&>(v_3_QPoint)); }
    { _ch.send(static_cast<QList<QString> const&>(v_4_QList_QString_), static_cast<QString const&>(v_1_QString)); }
    { _ch.send(QList<QUrl>()); }
    { _ch.send(QList<QUrl>(), QHash<QString, QVariant>()); }
    { _ch.send(QList<QUrl>(), static_cast<QHash<QString, QVariant> const&>(v_5_QHash_QString__QVariant_)); }
    { _ch.send(QList<QUrl>(), v_5_QHash_QString__QVariant_); }
    { _ch.send(QList<QUrl>(), static_cast<QUrl const&>(v_10_QUrl), v_8_bool); }
    { _ch.send(static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QHash<QString, QVariant> const&>(v_5_QHash_QString__QVariant_)); }
    { _ch.send(QMap<QString, QVariant>()); }
    { _ch.send(static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _ch.send(v_14_QMap_QString__QVariant_); }
    { _ch.send(QModelIndex()); }
    { _ch.send(static_cast<QModelIndex const&>(v_37_QModelIndex)); }
    { _ch.send(&v_11_QPainter, static_cast<QRectF const&>(v_12_QRectF), static_cast<QSharedPointer<dfmbase::FileInfo> const&>(v_13_QSharedPointer_dfmbase__FileInfo_)); }
    { _ch.send(QString()); }
    { _ch.send(QString(), v_4_QList_QString_); }
    { _ch.send(QString(), QMap<QString, QVariant>()); }
    { _ch.send(QString(), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _ch.send(QString(), v_14_QMap_QString__QVariant_); }
    { _ch.send(QString(), pv_5_QPoint); }
    { _ch.send(QString(), v_15_QRect); }
    { _ch.send(QString(), static_cast<QString const&>(v_1_QString)); }
    { _ch.send(QString(), v_1_QString); }
    { _ch.send(QString(), static_cast<QUrl const&>(v_10_QUrl)); }
    { _ch.send(QString(), static_cast<QUrl const&>(v_10_QUrl), int(0), false); }
    { _ch.send(QString(), v_8_bool, pv_8_QWidget); }
    { _ch.send(QString(), "xxxxxxxxxxxx"); }
    { _ch.send(QString(), static_cast<dfmbase::Global::DirectoryLoadStrategy>(0)); }
    { _ch.send(QString(), dfmbase::Global::ViewMode::kIconMode); }
    { _ch.send(QString(), v_17_std__function_void__unsigned_long_long__QUrl_const___std__function_void______); }
    { _ch.send(static_cast<QString const&>(v_1_QString)); }
    { _ch.send(static_cast<QString const&>(v_1_QString), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _ch.send(static_cast<QString const&>(v_1_QString), static_cast<QString const&>(v_1_QString)); }
    { _ch.send(QUrl()); }
    { _ch.send(QUrl(), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _ch.send(QUrl(), v_14_QMap_QString__QVariant_); }
    { _ch.send(QUrl(), pv_5_QPoint); }
    { _ch.send(QUrl(), static_cast<QString const&>(v_1_QString)); }
    { _ch.send(QUrl(), v_8_bool); }
    { _ch.send(static_cast<QUrl const&>(v_10_QUrl)); }
    { _ch.send(static_cast<QUrl const&>(v_10_QUrl), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _ch.send(QVariant(), QVariant(), QVariant(), int(0)); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"); }
    { _ch.send(&v_20_QWidget, "xxxxxxx"); }
    { _ch.send(false); }
    { _ch.send(false, int(0), false); }
    { _ch.send(false, v_2_int, v_8_bool); }
    { _ch.send(static_cast<char const*>(nullptr)); }
    { _ch.send(int(0), static_cast<QPoint const&>(v_3_QPoint)); }
    { _ch.send(int(0), v_15_QRect); }
    { _ch.send(int(0), QUrl(), v_14_QMap_QString__QVariant_); }
    { _ch.send(int(0), static_cast<QUrl const&>(v_10_QUrl)); }
    { _ch.send(v_2_int); }
    { _ch.send(v_2_int, static_cast<QPoint const&>(v_3_QPoint)); }
    { _ch.send(std::function<QMap<QString, QMultiMap<QString, std::pair<QString, QString> > > (QUrl const&)>(), QString()); }
    { _ch.send(std::function<QWidget* (QUrl const&)>(), QString()); }
    { _ch.send(std::function<QWidget* (QUrl const&)>(), v_1_QString, v_2_int); }
    { _ch.send(std::function<QWidget* (QUrl const&)>(), v_2_int); }
    { _ch.send(std::function<QWidget* (QUrl const&)>(), v_22_std__function_void__QWidget___QUrl_const___, QString(), int(0)); }
    { _ch.send(quint64(0)); }
    { _ch.send(quint64(0), static_cast<QAbstractItemView::DragDropMode>(0)); }
    { _ch.send(quint64(0), static_cast<QAbstractItemView::SelectionMode const&>(v_23_QAbstractItemView__SelectionMode)); }
    { _ch.send(quint64(0), v_24_QFlags_QDir__Filter_); }
    { _ch.send(quint64(0), static_cast<QList<QAbstractItemView::SelectionMode> const&>(v_26_QList_QAbstractItemView__SelectionMode_)); }
    { _ch.send(quint64(0), QList<QString>()); }
    { _ch.send(quint64(0), v_4_QList_QString_); }
    { _ch.send(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    { _ch.send(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    { _ch.send(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_6_QList_QUrl_); }
    { _ch.send(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl)); }
    { _ch.send(quint64(0), v_6_QList_QUrl_); }
    { _ch.send(quint64(0), v_6_QList_QUrl_, v_6_QList_QUrl_); }
    { _ch.send(quint64(0), static_cast<QMap<QString, QVariant> const&>(v_14_QMap_QString__QVariant_)); }
    { _ch.send(quint64(0), QString()); }
    { _ch.send(quint64(0), QString(), v_8_bool); }
    { _ch.send(quint64(0), static_cast<QString const&>(v_1_QString)); }
    { _ch.send(quint64(0), static_cast<QString const&>(v_1_QString), v_8_bool); }
    { _ch.send(quint64(0), static_cast<QUrl const&>(v_10_QUrl)); }
    { _ch.send(quint64(0), static_cast<QUrl const&>(v_10_QUrl), QVariant()); }
    { _ch.send(quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<dfmbase::Global::ItemRoles const&>(v_34_dfmbase__Global__ItemRoles)); }
    { _ch.send(quint64(0), v_10_QUrl); }
    { _ch.send(quint64(0), v_10_QUrl, QVariant()); }
    { _ch.send(quint64(0), false); }
    { _ch.send(quint64(0), v_8_bool); }
    { _ch.send(quint64(0), v_8_bool, v_8_bool); }
    { _ch.send(quint64(0), dfmbase::Global::ItemRoles::kItemDisplayRole); }
    { _ch.send(quint64(0), v_34_dfmbase__Global__ItemRoles); }
    { _ch.send(quint64(0), v_2_int); }
    { _ch.send(static_cast<unsigned long long const&>(v_38_unsigned_long_long), v_34_dfmbase__Global__ItemRoles); }
    // run (EventType variant)
    { _sm->run(_et, &v_4_QList_QString_, pv_0_void); }
    { _sm->run(_et, QList<QUrl>()); }
    { _sm->run(_et, QList<QUrl>(), &v_6_QList_QUrl_); }
    { _sm->run(_et, QList<QUrl>(), pv_1_QMimeData, pv_0_void); }
    { _sm->run(_et, QList<QUrl>(), QUrl(), &v_9_Qt__DropAction); }
    { _sm->run(_et, QList<QUrl>(), static_cast<QUrl const&>(v_10_QUrl)); }
    { _sm->run(_et, QList<QUrl>(), static_cast<QUrl const&>(v_10_QUrl), pv_2_Qt__DropAction); }
    { _sm->run(_et, QList<QUrl>(), v_10_QUrl); }
    { _sm->run(_et, QList<QUrl>(), v_10_QUrl, &v_9_Qt__DropAction); }
    { _sm->run(_et, static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl)); }
    { _sm->run(_et, &v_6_QList_QUrl_); }
    { _sm->run(_et, &v_6_QList_QUrl_, pv_0_void); }
    { _sm->run(_et, QSharedPointer<dfmbase::FileInfo>(), pv_3_dfmbase__ElideTextLayout); }
    { _sm->run(_et, QString()); }
    { _sm->run(_et, QString(), pv_4_QMimeData_const, static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { _sm->run(_et, QString(), QString(), QUrl(), QUrl()); }
    { _sm->run(_et, QString(), static_cast<QString const&>(v_1_QString), pv_0_void); }
    { _sm->run(_et, QString(), &v_1_QString, &v_8_bool); }
    { _sm->run(_et, QString(), static_cast<QUrl const&>(v_10_QUrl), pv_6_QPainter, pv_7_QStyleOptionViewItem_const, pv_0_void); }
    { _sm->run(_et, QString(), v_2_int, v_2_int, pv_0_void); }
    { _sm->run(_et, QString(), v_2_int, pv_0_void); }
    { _sm->run(_et, QUrl()); }
    { _sm->run(_et, QUrl(), pv_9_QList_QAbstractItemView__SelectionMode_); }
    { _sm->run(_et, QUrl(), pv_10_QList_QIcon_); }
    { _sm->run(_et, QUrl(), &v_18_QList_QMap_QString__QVariant___); }
    { _sm->run(_et, QUrl(), static_cast<QList<int> const&>(v_19_QList_int_), pv_0_void); }
    { _sm->run(_et, QUrl(), &v_1_QString); }
    { _sm->run(_et, QUrl(), QUrl()); }
    { _sm->run(_et, QUrl(), static_cast<QUrl const&>(v_10_QUrl)); }
    { _sm->run(_et, QUrl(), static_cast<QUrl const&>(v_10_QUrl), pv_0_void); }
    { _sm->run(_et, QUrl(), &v_10_QUrl); }
    { _sm->run(_et, QUrl(), &v_8_bool); }
    { _sm->run(_et, QUrl(), pv_11_dfmbase__Global__TransparentStatus); }
    { _sm->run(_et, QUrl(), v_2_int, pv_12_QVariant, pv_0_void); }
    { _sm->run(_et, QUrl(), pv_0_void); }
    { _sm->run(_et, static_cast<QUrl const&>(v_10_QUrl)); }
    { _sm->run(_et, &v_10_QUrl); }
    { _sm->run(_et, int(0), pv_4_QMimeData_const, static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { _sm->run(_et, int(0), pv_4_QMimeData_const, pv_0_void); }
    { _sm->run(_et, int(0), static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { _sm->run(_et, int(0), static_cast<QSharedPointer<dfmbase::FileInfo> const&>(v_13_QSharedPointer_dfmbase__FileInfo_), pv_6_QPainter, pv_13_QRectF); }
    { _sm->run(_et, int(0), static_cast<QString const&>(v_1_QString), pv_0_void); }
    { _sm->run(_et, int(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { _sm->run(_et, int(0), static_cast<QUrl const&>(v_10_QUrl), pv_6_QPainter, pv_7_QStyleOptionViewItem_const, pv_0_void); }
    { _sm->run(_et, int(0), int(0), int(0), nullptr); }
    { _sm->run(_et, int(0), v_2_int, pv_14_QList_QUrl_, pv_0_void); }
    { _sm->run(_et, int(0), v_2_int, static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { _sm->run(_et, int(0), v_2_int, v_2_int, pv_0_void); }
    { _sm->run(_et, int(0), v_2_int, pv_0_void); }
    { _sm->run(_et, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    { _sm->run(_et, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_)); }
    { _sm->run(_et, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QList<QString> const&>(v_4_QList_QString_)); }
    { _sm->run(_et, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), QUrl()); }
    { _sm->run(_et, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<std::pair<QString, QString> const&>(v_28_std__pair_QString__QString_), static_cast<bool const&>(v_8_bool)); }
    { _sm->run(_et, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<std::pair<QString, dfmbase::AbstractJobHandler::FileNameAddFlag> const&>(v_29_std__pair_QString__dfmbase__AbstractJobHandler__FileNameAddFlag_)); }
    { _sm->run(_et, quint64(0), v_6_QList_QUrl_, QUrl()); }
    { _sm->run(_et, quint64(0), v_6_QList_QUrl_, static_cast<QUrl const&>(v_10_QUrl), static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_)); }
    { _sm->run(_et, quint64(0), static_cast<QUrl const&>(v_10_QUrl)); }
    { _sm->run(_et, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QFlags<QFileDevice::Permission> const&>(v_32_QFlags_QFileDevice__Permission_), &v_8_bool, &v_1_QString); }
    { _sm->run(_et, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), static_cast<bool const&>(v_8_bool), static_cast<bool const&>(v_8_bool)); }
    { _sm->run(_et, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), static_cast<dfmbase::AbstractJobHandler::JobFlag const&>(v_33_dfmbase__AbstractJobHandler__JobFlag)); }
    { _sm->run(_et, quint64(0), static_cast<QUrl const&>(v_10_QUrl), v_10_QUrl, static_cast<QUrl const&>(v_10_QUrl), static_cast<QString const&>(v_1_QString), static_cast<QVariant const&>(v_16_QVariant), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____, &v_1_QString); }
    { _sm->run(_et, quint64(0), static_cast<QUrl const&>(v_10_QUrl), v_10_QUrl, static_cast<QVariant const&>(v_16_QVariant), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _sm->run(_et, quint64(0), static_cast<QUrl const&>(v_10_QUrl), v_10_QUrl, static_cast<dfmbase::Global::CreateFileType const&>(v_31_dfmbase__Global__CreateFileType), static_cast<QString const&>(v_1_QString), static_cast<QVariant const&>(v_16_QVariant), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____, &v_1_QString); }
    { _sm->run(_et, quint64(0), v_10_QUrl, v_10_QUrl); }
    { _sm->run(_et, quint64(0), static_cast<dfmbase::ClipBoard::ClipboardAction const&>(v_35_dfmbase__ClipBoard__ClipboardAction), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    // run (space+topic variant, topic starts with "hook")
    { _sm->run(QString("x"), QString("hook_t"), &v_4_QList_QString_, pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), QList<QUrl>()); }
    { _sm->run(QString("x"), QString("hook_t"), QList<QUrl>(), &v_6_QList_QUrl_); }
    { _sm->run(QString("x"), QString("hook_t"), QList<QUrl>(), pv_1_QMimeData, pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), QList<QUrl>(), QUrl(), &v_9_Qt__DropAction); }
    { _sm->run(QString("x"), QString("hook_t"), QList<QUrl>(), static_cast<QUrl const&>(v_10_QUrl)); }
    { _sm->run(QString("x"), QString("hook_t"), QList<QUrl>(), static_cast<QUrl const&>(v_10_QUrl), pv_2_Qt__DropAction); }
    { _sm->run(QString("x"), QString("hook_t"), QList<QUrl>(), v_10_QUrl); }
    { _sm->run(QString("x"), QString("hook_t"), QList<QUrl>(), v_10_QUrl, &v_9_Qt__DropAction); }
    { _sm->run(QString("x"), QString("hook_t"), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl)); }
    { _sm->run(QString("x"), QString("hook_t"), &v_6_QList_QUrl_); }
    { _sm->run(QString("x"), QString("hook_t"), &v_6_QList_QUrl_, pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), QSharedPointer<dfmbase::FileInfo>(), pv_3_dfmbase__ElideTextLayout); }
    { _sm->run(QString("x"), QString("hook_t"), QString()); }
    { _sm->run(QString("x"), QString("hook_t"), QString(), pv_4_QMimeData_const, static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), QString(), QString(), QUrl(), QUrl()); }
    { _sm->run(QString("x"), QString("hook_t"), QString(), static_cast<QString const&>(v_1_QString), pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), QString(), &v_1_QString, &v_8_bool); }
    { _sm->run(QString("x"), QString("hook_t"), QString(), static_cast<QUrl const&>(v_10_QUrl), pv_6_QPainter, pv_7_QStyleOptionViewItem_const, pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), QString(), v_2_int, v_2_int, pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), QString(), v_2_int, pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), QUrl()); }
    { _sm->run(QString("x"), QString("hook_t"), QUrl(), pv_9_QList_QAbstractItemView__SelectionMode_); }
    { _sm->run(QString("x"), QString("hook_t"), QUrl(), pv_10_QList_QIcon_); }
    { _sm->run(QString("x"), QString("hook_t"), QUrl(), &v_18_QList_QMap_QString__QVariant___); }
    { _sm->run(QString("x"), QString("hook_t"), QUrl(), static_cast<QList<int> const&>(v_19_QList_int_), pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), QUrl(), &v_1_QString); }
    { _sm->run(QString("x"), QString("hook_t"), QUrl(), QUrl()); }
    { _sm->run(QString("x"), QString("hook_t"), QUrl(), static_cast<QUrl const&>(v_10_QUrl)); }
    { _sm->run(QString("x"), QString("hook_t"), QUrl(), static_cast<QUrl const&>(v_10_QUrl), pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), QUrl(), &v_10_QUrl); }
    { _sm->run(QString("x"), QString("hook_t"), QUrl(), &v_8_bool); }
    { _sm->run(QString("x"), QString("hook_t"), QUrl(), pv_11_dfmbase__Global__TransparentStatus); }
    { _sm->run(QString("x"), QString("hook_t"), QUrl(), v_2_int, pv_12_QVariant, pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), QUrl(), pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), static_cast<QUrl const&>(v_10_QUrl)); }
    { _sm->run(QString("x"), QString("hook_t"), &v_10_QUrl); }
    { _sm->run(QString("x"), QString("hook_t"), int(0), pv_4_QMimeData_const, static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), int(0), pv_4_QMimeData_const, pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), int(0), static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), int(0), static_cast<QSharedPointer<dfmbase::FileInfo> const&>(v_13_QSharedPointer_dfmbase__FileInfo_), pv_6_QPainter, pv_13_QRectF); }
    { _sm->run(QString("x"), QString("hook_t"), int(0), static_cast<QString const&>(v_1_QString), pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), int(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), int(0), static_cast<QUrl const&>(v_10_QUrl), pv_6_QPainter, pv_7_QStyleOptionViewItem_const, pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), int(0), int(0), int(0), nullptr); }
    { _sm->run(QString("x"), QString("hook_t"), int(0), v_2_int, pv_14_QList_QUrl_, pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), int(0), v_2_int, static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), int(0), v_2_int, v_2_int, pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), int(0), v_2_int, pv_0_void); }
    { _sm->run(QString("x"), QString("hook_t"), quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    { _sm->run(QString("x"), QString("hook_t"), quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_)); }
    { _sm->run(QString("x"), QString("hook_t"), quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QList<QString> const&>(v_4_QList_QString_)); }
    { _sm->run(QString("x"), QString("hook_t"), quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), QUrl()); }
    { _sm->run(QString("x"), QString("hook_t"), quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<std::pair<QString, QString> const&>(v_28_std__pair_QString__QString_), static_cast<bool const&>(v_8_bool)); }
    { _sm->run(QString("x"), QString("hook_t"), quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<std::pair<QString, dfmbase::AbstractJobHandler::FileNameAddFlag> const&>(v_29_std__pair_QString__dfmbase__AbstractJobHandler__FileNameAddFlag_)); }
    { _sm->run(QString("x"), QString("hook_t"), quint64(0), v_6_QList_QUrl_, QUrl()); }
    { _sm->run(QString("x"), QString("hook_t"), quint64(0), v_6_QList_QUrl_, static_cast<QUrl const&>(v_10_QUrl), static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_)); }
    { _sm->run(QString("x"), QString("hook_t"), quint64(0), static_cast<QUrl const&>(v_10_QUrl)); }
    { _sm->run(QString("x"), QString("hook_t"), quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QFlags<QFileDevice::Permission> const&>(v_32_QFlags_QFileDevice__Permission_), &v_8_bool, &v_1_QString); }
    { _sm->run(QString("x"), QString("hook_t"), quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), static_cast<bool const&>(v_8_bool), static_cast<bool const&>(v_8_bool)); }
    { _sm->run(QString("x"), QString("hook_t"), quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), static_cast<dfmbase::AbstractJobHandler::JobFlag const&>(v_33_dfmbase__AbstractJobHandler__JobFlag)); }
    { _sm->run(QString("x"), QString("hook_t"), quint64(0), static_cast<QUrl const&>(v_10_QUrl), v_10_QUrl, static_cast<QUrl const&>(v_10_QUrl), static_cast<QString const&>(v_1_QString), static_cast<QVariant const&>(v_16_QVariant), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____, &v_1_QString); }
    { _sm->run(QString("x"), QString("hook_t"), quint64(0), static_cast<QUrl const&>(v_10_QUrl), v_10_QUrl, static_cast<QVariant const&>(v_16_QVariant), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _sm->run(QString("x"), QString("hook_t"), quint64(0), static_cast<QUrl const&>(v_10_QUrl), v_10_QUrl, static_cast<dfmbase::Global::CreateFileType const&>(v_31_dfmbase__Global__CreateFileType), static_cast<QString const&>(v_1_QString), static_cast<QVariant const&>(v_16_QVariant), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____, &v_1_QString); }
    { _sm->run(QString("x"), QString("hook_t"), quint64(0), v_10_QUrl, v_10_QUrl); }
    { _sm->run(QString("x"), QString("hook_t"), quint64(0), static_cast<dfmbase::ClipBoard::ClipboardAction const&>(v_35_dfmbase__ClipBoard__ClipboardAction), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    // traversal (direct EventSequence)
    { _sq.traversal(&v_4_QList_QString_, pv_0_void); }
    { _sq.traversal(QList<QUrl>()); }
    { _sq.traversal(QList<QUrl>(), &v_6_QList_QUrl_); }
    { _sq.traversal(QList<QUrl>(), pv_1_QMimeData, pv_0_void); }
    { _sq.traversal(QList<QUrl>(), QUrl(), &v_9_Qt__DropAction); }
    { _sq.traversal(QList<QUrl>(), static_cast<QUrl const&>(v_10_QUrl)); }
    { _sq.traversal(QList<QUrl>(), static_cast<QUrl const&>(v_10_QUrl), pv_2_Qt__DropAction); }
    { _sq.traversal(QList<QUrl>(), v_10_QUrl); }
    { _sq.traversal(QList<QUrl>(), v_10_QUrl, &v_9_Qt__DropAction); }
    { _sq.traversal(static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl)); }
    { _sq.traversal(&v_6_QList_QUrl_); }
    { _sq.traversal(&v_6_QList_QUrl_, pv_0_void); }
    { _sq.traversal(QSharedPointer<dfmbase::FileInfo>(), pv_3_dfmbase__ElideTextLayout); }
    { _sq.traversal(QString()); }
    { _sq.traversal(QString(), pv_4_QMimeData_const, static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { _sq.traversal(QString(), QString(), QUrl(), QUrl()); }
    { _sq.traversal(QString(), static_cast<QString const&>(v_1_QString), pv_0_void); }
    { _sq.traversal(QString(), &v_1_QString, &v_8_bool); }
    { _sq.traversal(QString(), static_cast<QUrl const&>(v_10_QUrl), pv_6_QPainter, pv_7_QStyleOptionViewItem_const, pv_0_void); }
    { _sq.traversal(QString(), v_2_int, v_2_int, pv_0_void); }
    { _sq.traversal(QString(), v_2_int, pv_0_void); }
    { _sq.traversal(QUrl()); }
    { _sq.traversal(QUrl(), pv_9_QList_QAbstractItemView__SelectionMode_); }
    { _sq.traversal(QUrl(), pv_10_QList_QIcon_); }
    { _sq.traversal(QUrl(), &v_18_QList_QMap_QString__QVariant___); }
    { _sq.traversal(QUrl(), static_cast<QList<int> const&>(v_19_QList_int_), pv_0_void); }
    { _sq.traversal(QUrl(), &v_1_QString); }
    { _sq.traversal(QUrl(), QUrl()); }
    { _sq.traversal(QUrl(), static_cast<QUrl const&>(v_10_QUrl)); }
    { _sq.traversal(QUrl(), static_cast<QUrl const&>(v_10_QUrl), pv_0_void); }
    { _sq.traversal(QUrl(), &v_10_QUrl); }
    { _sq.traversal(QUrl(), &v_8_bool); }
    { _sq.traversal(QUrl(), pv_11_dfmbase__Global__TransparentStatus); }
    { _sq.traversal(QUrl(), v_2_int, pv_12_QVariant, pv_0_void); }
    { _sq.traversal(QUrl(), pv_0_void); }
    { _sq.traversal(static_cast<QUrl const&>(v_10_QUrl)); }
    { _sq.traversal(&v_10_QUrl); }
    { _sq.traversal(int(0), pv_4_QMimeData_const, static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { _sq.traversal(int(0), pv_4_QMimeData_const, pv_0_void); }
    { _sq.traversal(int(0), static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { _sq.traversal(int(0), static_cast<QSharedPointer<dfmbase::FileInfo> const&>(v_13_QSharedPointer_dfmbase__FileInfo_), pv_6_QPainter, pv_13_QRectF); }
    { _sq.traversal(int(0), static_cast<QString const&>(v_1_QString), pv_0_void); }
    { _sq.traversal(int(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { _sq.traversal(int(0), static_cast<QUrl const&>(v_10_QUrl), pv_6_QPainter, pv_7_QStyleOptionViewItem_const, pv_0_void); }
    { _sq.traversal(int(0), int(0), int(0), nullptr); }
    { _sq.traversal(int(0), v_2_int, pv_14_QList_QUrl_, pv_0_void); }
    { _sq.traversal(int(0), v_2_int, static_cast<QPoint const&>(v_3_QPoint), pv_0_void); }
    { _sq.traversal(int(0), v_2_int, v_2_int, pv_0_void); }
    { _sq.traversal(int(0), v_2_int, pv_0_void); }
    { _sq.traversal(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    { _sq.traversal(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_)); }
    { _sq.traversal(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QList<QString> const&>(v_4_QList_QString_)); }
    { _sq.traversal(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), QUrl()); }
    { _sq.traversal(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<std::pair<QString, QString> const&>(v_28_std__pair_QString__QString_), static_cast<bool const&>(v_8_bool)); }
    { _sq.traversal(quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<std::pair<QString, dfmbase::AbstractJobHandler::FileNameAddFlag> const&>(v_29_std__pair_QString__dfmbase__AbstractJobHandler__FileNameAddFlag_)); }
    { _sq.traversal(quint64(0), v_6_QList_QUrl_, QUrl()); }
    { _sq.traversal(quint64(0), v_6_QList_QUrl_, static_cast<QUrl const&>(v_10_QUrl), static_cast<QFlags<dfmbase::AbstractJobHandler::JobFlag> const&>(v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_)); }
    { _sq.traversal(quint64(0), static_cast<QUrl const&>(v_10_QUrl)); }
    { _sq.traversal(quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QFlags<QFileDevice::Permission> const&>(v_32_QFlags_QFileDevice__Permission_), &v_8_bool, &v_1_QString); }
    { _sq.traversal(quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), static_cast<bool const&>(v_8_bool), static_cast<bool const&>(v_8_bool)); }
    { _sq.traversal(quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), static_cast<dfmbase::AbstractJobHandler::JobFlag const&>(v_33_dfmbase__AbstractJobHandler__JobFlag)); }
    { _sq.traversal(quint64(0), static_cast<QUrl const&>(v_10_QUrl), v_10_QUrl, static_cast<QUrl const&>(v_10_QUrl), static_cast<QString const&>(v_1_QString), static_cast<QVariant const&>(v_16_QVariant), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____, &v_1_QString); }
    { _sq.traversal(quint64(0), static_cast<QUrl const&>(v_10_QUrl), v_10_QUrl, static_cast<QVariant const&>(v_16_QVariant), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _sq.traversal(quint64(0), static_cast<QUrl const&>(v_10_QUrl), v_10_QUrl, static_cast<dfmbase::Global::CreateFileType const&>(v_31_dfmbase__Global__CreateFileType), static_cast<QString const&>(v_1_QString), static_cast<QVariant const&>(v_16_QVariant), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____, &v_1_QString); }
    { _sq.traversal(quint64(0), v_10_QUrl, v_10_QUrl); }
    { _sq.traversal(quint64(0), static_cast<dfmbase::ClipBoard::ClipboardAction const&>(v_35_dfmbase__ClipBoard__ClipboardAction), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_)); }
    // publish (EventType variant)
    { _dm->publish(_et, QList<QUrl>(), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QList<QVariant> const&>(v_7_QList_QVariant_), v_8_bool, static_cast<QString const&>(v_1_QString)); }
    { _dm->publish(_et, QMap<QString, QVariant>()); }
    { _dm->publish(_et, QString(), v_6_QList_QUrl_); }
    { _dm->publish(_et, QString(), v_14_QMap_QString__QVariant_); }
    { _dm->publish(_et, QString(), v_1_QString); }
    { _dm->publish(_et, QUrl(), QUrl()); }
    { _dm->publish(_et, QUrl(), false); }
    { _dm->publish(_et, static_cast<QUrl const&>(v_10_QUrl)); }
    { _dm->publish(_et, int(0), v_6_QList_QUrl_, QList<QString>()); }
    { _dm->publish(_et, int(0), v_6_QList_QUrl_, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _dm->publish(_et, quint64(0)); }
    { _dm->publish(_et, quint64(0), QList<QUrl>()); }
    { _dm->publish(_et, quint64(0), QList<QUrl>(), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _dm->publish(_et, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_4_QList_QString_); }
    { _dm->publish(_et, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _dm->publish(_et, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _dm->publish(_et, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), false, v_1_QString); }
    { _dm->publish(_et, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _dm->publish(_et, quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<std::pair<QString, dfmbase::AbstractJobHandler::FileNameAddFlag> const&>(v_29_std__pair_QString__dfmbase__AbstractJobHandler__FileNameAddFlag_)); }
    { _dm->publish(_et, quint64(0), v_6_QList_QUrl_, v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_, nullptr, QVariant(), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _dm->publish(_et, quint64(0), v_6_QList_QUrl_, QUrl(), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _dm->publish(_et, quint64(0), v_6_QList_QUrl_, v_10_QUrl, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _dm->publish(_et, quint64(0), v_6_QList_QUrl_, v_10_QUrl, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, nullptr, QVariant(), nullptr); }
    { _dm->publish(_et, quint64(0), v_6_QList_QUrl_, false, QString()); }
    { _dm->publish(_et, quint64(0), v_6_QList_QUrl_, false, "xxxxxxxxxxxxxxxxxxxx"); }
    { _dm->publish(_et, quint64(0), v_30_QMap_QUrl__QUrl_, false, QString()); }
    { _dm->publish(_et, quint64(0), v_30_QMap_QUrl__QUrl_, false, v_1_QString); }
    { _dm->publish(_et, quint64(0), v_30_QMap_QUrl__QUrl_, false, ""); }
    { _dm->publish(_et, quint64(0), QString()); }
    { _dm->publish(_et, quint64(0), static_cast<QString const&>(v_1_QString), static_cast<QString const&>(v_1_QString)); }
    { _dm->publish(_et, quint64(0), v_1_QString, v_6_QList_QUrl_); }
    { _dm->publish(_et, quint64(0), pv_15_QString); }
    { _dm->publish(_et, quint64(0), QUrl(), static_cast<QUrl const&>(v_10_QUrl), QString(), v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _dm->publish(_et, quint64(0), QUrl(), v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _dm->publish(_et, quint64(0), QUrl(), dfmbase::Global::CreateFileType::kCreateFileTypeUnknow, QString(), QVariant(), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _dm->publish(_et, quint64(0), QUrl(), static_cast<dfmbase::Global::CreateFileType const&>(v_31_dfmbase__Global__CreateFileType), v_1_QString, v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _dm->publish(_et, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_8_bool); }
    { _dm->publish(_et, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), QString(), dfmbase::GlobalEventType(1), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _dm->publish(_et, quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<dfmbase::Global::CreateFileType const&>(v_31_dfmbase__Global__CreateFileType), v_1_QString, dfmbase::GlobalEventType(1), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _dm->publish(_et, quint64(0), v_10_QUrl, v_10_QUrl, dfmbase::AbstractJobHandler::JobFlag::kNoHint); }
    { _dm->publish(_et, quint64(0), dfmbase::ClipBoard::ClipboardAction(0), v_6_QList_QUrl_); }
    // publish (space+topic variant, topic starts with "signal")
    { _dm->publish(QString("x"), QString("signal_t"), QList<QUrl>(), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QList<QVariant> const&>(v_7_QList_QVariant_), v_8_bool, static_cast<QString const&>(v_1_QString)); }
    { _dm->publish(QString("x"), QString("signal_t"), QMap<QString, QVariant>()); }
    { _dm->publish(QString("x"), QString("signal_t"), QString(), v_6_QList_QUrl_); }
    { _dm->publish(QString("x"), QString("signal_t"), QString(), v_14_QMap_QString__QVariant_); }
    { _dm->publish(QString("x"), QString("signal_t"), QString(), v_1_QString); }
    { _dm->publish(QString("x"), QString("signal_t"), QUrl(), QUrl()); }
    { _dm->publish(QString("x"), QString("signal_t"), QUrl(), false); }
    { _dm->publish(QString("x"), QString("signal_t"), static_cast<QUrl const&>(v_10_QUrl)); }
    { _dm->publish(QString("x"), QString("signal_t"), int(0), v_6_QList_QUrl_, QList<QString>()); }
    { _dm->publish(QString("x"), QString("signal_t"), int(0), v_6_QList_QUrl_, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0)); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), QList<QUrl>()); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), QList<QUrl>(), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_4_QList_QString_); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<QUrl const&>(v_10_QUrl), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), false, v_1_QString); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), static_cast<std::pair<QString, dfmbase::AbstractJobHandler::FileNameAddFlag> const&>(v_29_std__pair_QString__dfmbase__AbstractJobHandler__FileNameAddFlag_)); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), v_6_QList_QUrl_, v_27_QFlags_dfmbase__AbstractJobHandler__JobFlag_, nullptr, QVariant(), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), v_6_QList_QUrl_, QUrl(), dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), v_6_QList_QUrl_, v_10_QUrl, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), v_6_QList_QUrl_, v_10_QUrl, dfmbase::AbstractJobHandler::JobFlag::kNoHint, nullptr, nullptr, QVariant(), nullptr); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), v_6_QList_QUrl_, false, QString()); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), v_6_QList_QUrl_, false, "xxxxxxxxxxxxxxxxxxxx"); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), v_30_QMap_QUrl__QUrl_, false, QString()); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), v_30_QMap_QUrl__QUrl_, false, v_1_QString); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), v_30_QMap_QUrl__QUrl_, false, ""); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), QString()); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), static_cast<QString const&>(v_1_QString), static_cast<QString const&>(v_1_QString)); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), v_1_QString, v_6_QList_QUrl_); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), pv_15_QString); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), QUrl(), static_cast<QUrl const&>(v_10_QUrl), QString(), v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), QUrl(), v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), QUrl(), dfmbase::Global::CreateFileType::kCreateFileTypeUnknow, QString(), QVariant(), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), QUrl(), static_cast<dfmbase::Global::CreateFileType const&>(v_31_dfmbase__Global__CreateFileType), v_1_QString, v_16_QVariant, v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QList<QUrl> const&>(v_6_QList_QUrl_), v_8_bool); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<QUrl const&>(v_10_QUrl), QString(), dfmbase::GlobalEventType(1), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), static_cast<QUrl const&>(v_10_QUrl), static_cast<dfmbase::Global::CreateFileType const&>(v_31_dfmbase__Global__CreateFileType), v_1_QString, dfmbase::GlobalEventType(1), v_21_std__function_void__QSharedPointer_QMap_dfmbase__AbstractJobHandler__CallbackKey__QVariant_____); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), v_10_QUrl, v_10_QUrl, dfmbase::AbstractJobHandler::JobFlag::kNoHint); }
    { _dm->publish(QString("x"), QString("signal_t"), quint64(0), dfmbase::ClipBoard::ClipboardAction(0), v_6_QList_QUrl_); }
}
