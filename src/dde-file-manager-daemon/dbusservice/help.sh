#!/bin/bash

echo "=====fileoperation====="
qdbusxml2cpp -i controllers/fileoperation.h  -i dbusservice/dbustype/dbusinforet.h -c FileOperationAdaptor -l FileOperation -a dbusadaptor/fileoperation_adaptor fileoperation.xml
qdbusxml2cpp -i dbusservice/dbustype/dbusinforet.h -c FileOperationInterface -p dbusinterface/fileoperation_interface fileoperation.xml


echo "=====createfolderjob====="
qdbusxml2cpp -i fileoperationjob/createfolderjob.h -c CreateFolderJobAdaptor -l CreateFolderJob -a dbusadaptor/createfolderjob_adaptor createfolderjob.xml
qdbusxml2cpp -c CreateFolderJobInterface -p dbusinterface/createfolderjob_interface createfolderjob.xml


echo "=====createtemplatefilejob====="
qdbusxml2cpp -i fileoperationjob/createtemplatefilejob.h -c CreateTemplateFileAdaptor -l CreateTemplateFileJob -a dbusadaptor/createtemplatefilejob_adaptor createtemplatefilejob.xml
qdbusxml2cpp -c CreateTemplateFileInterface -p dbusinterface/createtemplatefilejob_interface createtemplatefilejob.xml


echo "=====copyjob====="
qdbusxml2cpp -i fileoperationjob/copyjob.h -c CopyJobAdaptor -l CopyJob -a dbusadaptor/copyjob_adaptor copyjob.xml
qdbusxml2cpp -c CopyJobInterface -p dbusinterface/copyjob_interface copyjob.xml


echo "=====movejob====="
qdbusxml2cpp -i fileoperationjob/movejob.h -c MoveJobAdaptor -l MoveJob -a dbusadaptor/movejob_adaptor movejob.xml
qdbusxml2cpp -c MoveJobInterface -p dbusinterface/movejob_interface movejob.xml


echo "=====renamejob====="
qdbusxml2cpp -i fileoperationjob/renamejob.h -c RenameJobAdaptor -l RenameJob -a dbusadaptor/renamejob_adaptor renamejob.xml
qdbusxml2cpp -c RenameJobInterface -p dbusinterface/renamejob_interface renamejob.xml

echo "=====deletejob====="
qdbusxml2cpp -i fileoperationjob/deletejob.h -c DeleteJobAdaptor -l DeleteJob -a dbusadaptor/deletejob_adaptor deletejob.xml
qdbusxml2cpp -c DeleteJobInterface -p dbusinterface/deletejob_interface deletejob.xml


echo "=====usershare====="
qdbusxml2cpp -i usershare/usersharemanager.h -c UserShareAdaptor -l UserShareManager -a dbusadaptor/usershare_adaptor usershare.xml
qdbusxml2cpp -c UserShareInterface -p dbusinterface/usershare_interface usershare.xml

echo "=====accesscontrol====="
#qdbuscpp2xml -M -S ../accesscontrol/accesscontrolmanager.h -o acesscontrol.xml
qdbusxml2cpp -i accesscontrol/accesscontrolmanager.h -c AccessControlAdaptor -l AccessControlManager -a dbusadaptor/accesscontrol_adaptor acesscontrol.xml
qdbusxml2cpp -c AccessControlInterface -p dbusinterface/accesscontrol_interface acesscontrol.xml

echo "=====usbformatter====="
qdbusxml2cpp -i usbformatter/usbformatter.h -c UsbFormatterAdaptor -l UsbFormatter -a dbusadaptor/usbformatter_adaptor usbformatter.xml
qdbusxml2cpp -c UsbFormatterInterface -p dbusinterface/usbformatter_interface usbformatter.xml

echo "=====commandmanager====="
qdbusxml2cpp -i commandmanager/commandmanager.h -c CommandManagerAdaptor -l CommandManager -a dbusadaptor/commandmanager_adaptor commandmanager.xml
qdbusxml2cpp -c CommandManagerInterface -p dbusinterface/commandmanager_interface commandmanager.xml

echo "=====deviceinfomanager====="
qdbusxml2cpp -i deviceinfo/deviceinfomanager.h -c DeviceInfoManagerAdaptor -l DeviceInfoManager -a dbusadaptor/deviceinfomanager_adaptor deviceinfo.xml
qdbusxml2cpp -i ../partman/partition.h -c DeviceInfoManagerInterface -p dbusinterface/deviceinfomanager_interface deviceinfo.xml

echo "====tagmanagerdaemon===="
qdbusxml2cpp -i tag/tagmanagerdaemon.h -c TagManagerDaemonAdaptor -a dbusadaptor/tagmanagerdaemon_adaptor tagmanagerdaemon.xml

echo notice that: the xxx_interface.h/cpp files were put into 'dde-file-manager-lib' not 'dde-file-manager-daemon'.
qdbusxml2cpp -c TagManagerDaemonInterface -p ../../dde-file-manager-lib/controllers/interface/tagmanagerdaemon_interface tagmanagerdaemon.xml

echo "====quicksearchdaemon===="
qdbusxml2cpp -i quicksearch/quicksearchdaemon.h -c QuickSearchDaemonAdaptor -a dbusadaptor/quicksearchdaemon_adaptor quicksearchdaemon.xml

echo notice that: the xxx_interface.h/cpp files were put into 'dde-file-manager-lib' not 'dde-file-manager-daemon'.
qdbusxml2cpp -c QuickSearchDaemonInterface -p ../../dde-file-manger-lib/interface/quicksearchdaemon_interface quicksearchdaemon.xml
