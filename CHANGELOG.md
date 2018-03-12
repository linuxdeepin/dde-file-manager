<a name="4.4.8.1"></a>
## 4.4.8.1 (2018-03-12)


#### Bug Fixes

*   adjust click area ([c3f5b814](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c3f5b8141338d65b42689f14bda2ac7c8c13c5d2))
*   all scroll bar widget width ([cf928b29](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/cf928b29b05d3ebee7c620660dde649b52c48e28))
*   the width of scroll bar of file view ([1e3c5323](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1e3c53237c3f82e5649554fde3db617d37b145dd))
*   disk mount plugin crash when disk total is 0 ([73e47995](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/73e479950230d867858a89e49191e6514ff7edf4))



<a name="4.4.8"></a>
## 4.4.8 (2018-03-08)


#### Bug Fixes

*   use DTitlebar instead of custom titlebar ([8f522e26](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/8f522e264c4ecab19e14f9659d56707e81da9704))
*   set QProgressBar value type to int instead of qulonglong ([10ed207f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/10ed207f87ea0aef472b686708d7df5e10252a14))
*   support search file from root / when current view is computer view ([d6949f06](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/d6949f0655cc33af265e15d5810ed4cb87c4d5e7))
*   open file by desktop app when drop file on dekstop app file ([4df8d44c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4df8d44c35c0c747989bd93dc9d5e19f0175931d))
*   remove fsync operation when copy action is finished ([bdb4af97](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/bdb4af975b437aaed7815943816daa9d4251f93d))
*   file counts of trash property dialog ([73811339](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/73811339c6bc65a9cba1bc866c6bc6c7fca3be17))
*   hide/show refresh too show ([0c77a75e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/0c77a75e32704da6d30e1bb501a2d45bcc8e7226))
*   move temp file case profile sync failed ([36676b17](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/36676b177edfaceb587aa56b202afadd8dc873c4))
*   the BaseDialog window type not is Qt::Dialog ([b7c28aeb](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b7c28aeb7a0a713b7ed101de28a4828aaa84c0fc))
*   the file chooser dialog window type is Qt::Window ([94b88480](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/94b88480992d7cb4bc383542a1824341842af12e))
*   resend desktop not update ([a6771c52](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a6771c524ff84d23e49f357dd41881ac7bcb66ce))
*   rename file missing ([a5034898](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a5034898aec019da63b4561ff44019b780f7c4c1))
*   shitf cannot select old files ([195d756c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/195d756cb532344dbe9c6a698169ba4be017c0d2))
*   clear select when mouse release ([edf0c118](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/edf0c1189fb8aa85d2a604699218129f44b0ae8c))
*   visual rect error ([08f6a1d9](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/08f6a1d9a0fe236840c63f5844c79940999720b8))
*   sync grid config when file move ([c7f906eb](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c7f906ebf2ccb3dce8434fec3a880cbea248daaa))
*   update geometry when dock mode change ([6f5b9b1a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/6f5b9b1aa39985eb151a66d70515a2422d4c0c15))
*   use DAbstractDialog instead of QDialog in DTaskDialog ([07e93b31](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/07e93b31d0ee9c8facadac2065b350c252cc6ed0))
*   show dialog in screen center when window id is 0 ([90f0367e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/90f0367eed7f38aa3a42d07bcfe871352b4eddb4))
*   hidpi with three screen crash ([15ada352](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/15ada3529a3b61562f9efa5e286165b3f036530a))
*   get correct icon and name which show in openwith menu ([748ab8a8](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/748ab8a85c6f7cb1bd73698c4e02a7065a5f0e3a))
*   add dde-file-manager in DE group ([fdfa7b12](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/fdfa7b12dd04a67d86688c53ada9138c892285bf))
*   disable restart samba service when file managerstart ([9472b2f3](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/9472b2f32dd16cdf1e2487fcf7c44ba53ef8d02d))
*   Adapt lintian ([63abc36f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/63abc36f81a62a6b1c45139f633d33a665c96eeb))
*   menu plugin path ([a9581c76](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a9581c76842f7b25e8425c109c95f356dfa4a725))
*    desktop file filename same as displayname and single selection behavior ([15f8b558](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/15f8b5580fe8c096b4cfc539ba4c23156e597393))
*   dde-xdg-user-dirs-update.sh ([24f31df2](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/24f31df29130aeadfd8461523fda76c44f919ecc))
* **DDE zone:**  visible lines at window background on menu popup ([070823d0](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/070823d061397c9ffab35a30b4c4e5cac100c50a))

#### Features

*   direct map filesystem to config file ([aaa6c8cf](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/aaa6c8cfe45354651cf1d728db2d58bf7d3c1f36))
*   use env to debug ([93bb7280](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/93bb7280f1f1bd8b8999980af95e57f234e5c2db))
*   support multi struct parial ([ecfb6388](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ecfb6388d1edc771d642965b19a2cd67cdc01346))
*   add swicth in dfm-platform to hide root system ([a7da63f1](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a7da63f151db789e43dc75a9310b9dd701957dbf))
*   disbale unmount function in platform of x86 pro ([ad91a51b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ad91a51b2429a877c64a24078bc36d3e34433296))
*   add build parameter BUILD_MINIMUM for live system ([fc0628de](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/fc0628de7f08d3cccebd49e3e35bbd6e9408b7eb))
*   add copy/cut/paste action in DFileDiloag ([853e01c4](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/853e01c49cd9c5d2d1335cc4e3882f11f4709017))
*   add "monitorFiles" for dbus file dialog manager ([de4429e2](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/de4429e25fe0f40970fcc9c84da4987096e6a826))
*   add function "windowFlags" for dbus file dialog ([301dabb9](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/301dabb940158c63c053976054d7bcad6ba92891))



<a name=""></a>
## 4.4.7 (2017-11-29)


#### Bug Fixes

*   don't disconnect menu connection of ToolBarSettingsMenu ([f9a7fefc](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f9a7fefcf318c5e834e647e60a7f1ae7b5ec2820))



<a name=""></a>
## 4.4.6 (2017-11-29)


#### Bug Fixes

*   copy large files in different window error by disconnect menu triggered connection when action is triggered ([72079c2e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/72079c2e5827f441990cdb6f98953cc846b761fb))



<a name=""></a>
## 4.4.5 (2017-11-28)


#### Bug Fixes

* **zone:**  video size error ([aac90e55](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/aac90e556d450d865ca787b34f26d5f7c091e085))

#### Features

*   add desktop geometry debug info ([5037a497](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/5037a497c1603b49a36c96154656e75767703e1c))



<a name=""></a>
## 4.4.4 (2017-11-20)

#### Bug Fixes

*   update translations ([1e18fc14](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4b31cc5a1418de87b56c26c8b69c27b48794958d))

<a name=""></a>
## 4.4.3 (2017-11-16)


#### Bug Fixes

*   replocale when override file ([1e18fc14](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1e18fc14d150e6bf17769510671ead84648e9997))
*   avoid strip in debian build. ([a9e113a2](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a9e113a232aa61c29471539dc3805d5a07f67f60))
*   sync all profile at the same time ([b4f1b01c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b4f1b01ccfa8d9b828c7737a3d730ce470dedf15))
*   can't write file in anonymity writable mode ([5d1a95fe](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/5d1a95fe33917f45ea9be78d91b3e5e77490c99c))
*   fix build error in sw for security label ([f4bbf131](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f4bbf131a1f19cbe18d56a8f560df5418c75ed2f))
*   update translations ([aa98aabc](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/aa98aabc08ec9e4affda6aaaaec7918987539b19))
*   update translations ([27e0d624](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/27e0d624794a2fb03d7d392760c2edc67f9cb579))
*   expandWidget show when drag ([c28129ac](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c28129ac9bcf89acd422d92f51d062fc6933b30a))



<a name=""></a>
## 4.4.2 (2017-11-09)


#### Bug Fixes

*   update link file icon in desktop when device is mounted ([3b3bfcba](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/3b3bfcba3855bd36192fed51f4021fd3e26f5628))
*   fix volume custom letter support ([7d870833](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/7d870833c957b2bd238d9f381c526ec78daf6776))
*   refactor trash property dialog by  DDialog ([76c7f390](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/76c7f3908bc0508378eb80b5cfea4565d755454a))
* **diskmount:**  add hdpi support ([35823b59](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/35823b59b1b3a3d73065cbf367eb4ecd522a6863))



<a name=""></a>
## 4.4.1 (2017-11-07)


#### Bug Fixes

*   crash when show computer property dialogby contextmenu of computer bookmark item in leftbar ([68dd3961](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/68dd39617479abb2432832db27cf86a00cda8ef0))
*   double delete crash ([53db4247](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/53db4247f4b5eaca1781624dcdd0107db43cc612))
*   add cryptsetup depends ([a57f1fd5](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a57f1fd5c55c2c02b630969f7e9fe7c3e554fc86))

#### Features

*   add dfm platform config file to disable unmount function in x86 pro ([8514d873](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/8514d873c7b9fea410d323c05f0aad2f764db1e8))



<a name=""></a>
## 4.4.0 (2017-11-02)


#### Bug Fixes

*   update editing select area ([4a5463a2](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4a5463a2c320fe66d48a7938f661f7f1692626ca))
*   fix no mini button ([30391ab2](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/30391ab2dd25c637e1cd867b0db4b178fd2ce4fb))
*   update auto mount and open checkbox state in dfm setting dialog ([bb750190](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/bb750190240ada372fb85d8466d68b8377835ec6))
*   auto mount luks device when bookmark of luks is pressed ([f07cbd0c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f07cbd0cdac649285bd091321526744044f854c8))
*   show conflicted dialog when cut same file from smb to native  by gio ([062fd218](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/062fd2184c41198eb82af1b244ee4627a2f369f3))
*   abandon code for delete temp copying file ([25e6cf2a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/25e6cf2a7d0a8c54508a0284cfc520599ef811ae))
*   set window FramelessWindowHint flag for pro ([59f7d6dc](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/59f7d6dc2722a06dfd37de0e2468f0dc0d54da26))
* **dde-desktop:**  fix move to trash by delete key press event ([11856a31](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/11856a31c4cba32c2ce500f82896092bf6ef645d))

#### Features

*   update help document ([779f5d71](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/779f5d7141c4467313b3d7dea1e264133b55cebb))



<a name=""></a>
## 4.3.5 (2017-10-30)


#### Bug Fixes

*   don't draw additionalIcon when do rename action ([da774c0b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/da774c0b9e76ad6cccc5ae3138767c11b18f3930))
*   update translations ([3051bcd5](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/3051bcd51f64ed28c256d266bb3a8d17f4be339c))
*   do copy action when do drop action in same disk if ctrl key is pressed ([5248ca1b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/5248ca1be1a61b669ec6417ea11dc5593925d448))
*   remove dde-file-manager-intergation recommends ([831e65d3](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/831e65d3c77c64979f786ffd8cebbf6d5576a98b))
*   show computer and trash property dialog not  as normal desktop file ([d7bae00f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/d7bae00f490db147aeff86b4a249520f90a15b3c))
*   close preview dialog when open preivew file ([171ef041](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/171ef041c44e1fa14d3253a996ee7397dce41332))
*   show and rasie verticalScrollBar of leftbar 100ms later after leftbar resize normally ([ca847cd1](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ca847cd1564ae2bc5e2d2acda78efb9424af9615))

#### Features

*   new view style of search and trash ([5ea6c63f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/5ea6c63f08128ff3484d2c9247affacc35216983))
*   auto play music when preview and stop when privewhide ([2d33daca](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2d33daca9fd638e69f55e4aaf7e2c3d52ec054ed))
*   add support for windows url shortcut support ([9cd6f16f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/9cd6f16fc76da4c663970097eb54f3e124791f11))
*   add clog support for changlog ([1ab524b6](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1ab524b6140ad20c7e8d13f9e95eabcc11f08c74))
*   wallpaper thumbnail support pixel ratio cache folder ([605df99f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/605df99f2b11e2ecd50940a9d397d9f53511acf4))
*   add flatpak applications path support ([72c9a7a5](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/72c9a7a5dd955273f7cbb08c2ef89b351805206b))
*   hotzone update corner support hiDPI ([f1822680](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f182268083ae54f94190465f2bbe0bb32284726d))
*   hot zone support hidpi and update new demo video ([28e353ff](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/28e353ffe8e094f4db90893feba9e15694c8495c))
*   wallpaper chooser use DRegionMonitor and support hiDPI ([5a0f8db1](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/5a0f8db14565dc73682afaef0532ed6be4c817ec))
*   wallpaper chooser use DRegionMonitor and support hiDPI ([079c36f4](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/079c36f4f31d792479fb015f696fc5a833461f2b))
*   hot zone support hidpi and update new demo video ([c3b6c7a5](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c3b6c7a5c7108cdd12e0a76cc22ce87cd14d5017))
* **desktop:**
  *  keep file position when rename ([9afbe8bb](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/9afbe8bb9d83acf27563b604be1b8e84c1a200e1))
  *  drag animation ([9697270e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/9697270e617747e5858606899cc0e3f5563c7f9b))



