<a name="4.4.9.2"></a>
## 4.4.9.2 (2018-08-15)


#### Bug Fixes

*   can not get the system disk size ([bad7b1e0](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/bad7b1e02c2b2caebeac7834b1839eca69b010d3))

#### Features

* **disk:**  disk i10n by label name ([b715d6f3](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b715d6f35a64909f8de89ecf3a10d4a8e4ad91ef))



<a name="4.4.9.1"></a>
## 4.4.9.1 (2018-06-22)


#### Bug Fixes

*   crash in DFileDialog:: selectedUrls ([72051847](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/72051847569aae55a35d27b8cacabdd49f5a57f9))



<a name="4.4.9"></a>
## 4.4.9 (2018-06-22)


#### Features

*   support touch event for DFileView ([fc0254b6](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/fc0254b6749865d3590b75728fcd74b12cc35f7e))
*   install default config files for dde-desktop&dde-file-manager ([241e5b3c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/241e5b3cbeb8ee0d18f9c9b52ab73f342c4d8744))
*   support custom the non allowable characters of file name ([2458c411](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2458c4112ff42939656559462d21486b28be399c))

#### Bug Fixes

*   Can't type trailing space when editing file name ([76a6f587](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/76a6f587dd9b460c3781faa15e942a18df0de99d))
*   file name length exceeds limit ([860fc8c6](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/860fc8c6b7b0acfc9761207b2a63bc21c9d4fd02))
*   optimize pixmap get of file icon ([0a7a455c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/0a7a455c6a5d02f7207af17ae1ea8af0e1faac83))
*   do nothing if the new file name is empty ([74bf1923](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/74bf1923a2d794dce0110adf8046272f56ded0b1))

#### Performance

* **cache:**  fix pixmap cache not working ([1ed4f9ff](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1ed4f9ff59e3619503a811d715e1578e5196ac8a))



<a name="4.4.8.7"></a>
## 4.4.8.7 (2018-05-23)


#### Bug Fixes

*   now you can choose to hide the suffix of file, when you are renaming a file. ([be2f4e99](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/be2f4e99d4bd97b2c08b6635dece7eeda1b41f84))
* **wallpaper:**
  *  thumbnail support HiDPI. ([62600ed7](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/62600ed71d35644b07e68c265e84a98a9578c3eb))
  *  delayed loading wallpaper ([ac97bb3c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ac97bb3c9da2bc5d4d31c7f273bddb9fad9c50f0))



<a name="4.4.8.6"></a>
## 4.4.8.6 (2018-05-04)


#### Bug Fixes

*   remove qdiskinfo include ([0a3ccd36](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/0a3ccd36fc3818e5421e66e2481a5264d50d1ad4))
*   can not drag move file to trash ([dc58b030](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/dc58b030e8b72f2a04cfba7a6869d0c48ce7e22d))
*   Enter smb scheme and cd to real path. ([71f0e8ec](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/71f0e8ecbf77ac628c034400c223aef119daba22))
*   carsh in close preview directory at destory size worker thread ([a2cac10f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a2cac10f1c9b333d85576becdbb0e6e4c2b79f11))
*   ComputerViewItem not correct issue ([168dddc0](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/168dddc07c4d906178401ba1311918c4cff91179))
*   can not open help in dde-desktop ([326f60fe](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/326f60fe3407b7ba53981de53f242fbf80fd85fd))
*   reset configs not apply on file view ([5c57a230](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/5c57a2305111e4674ac70b4ef154009619da90ae))
*   keep the trash file view column count to 2 ([a1bf65ab](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a1bf65ab82ba6190e0e881d3c2027f7aa985cb55))
*   Ctrl + Shift + Tab can not toggle previous tab ([59d30eca](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/59d30ecaa64ab7a42b28fcd45c51a26696525659))
*   trash file view font is wrong ([154d5e45](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/154d5e4591d1c3fd7369ccca0fc3e7c1ccd2714e))
*   can not share file on search file view ([7aea91cc](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/7aea91cc269a3ae4d112849960e08b6c179b39f4))
*   destory file preview resources on dialog close ([dc37d5a2](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/dc37d5a25009fde18ec9bf6f525727844fac0443))
*   crash when clean trash by dock trash plugin ([7b10ede9](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/7b10ede9f3d057a71c163d51411e875c02b23301))
*   open search file view on mount device by left sidebar ([75ddfad5](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/75ddfad53e64223861ccc0d94b9df709b91e9758))
*   tab label is empty of computer view ([ec8a8e59](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ec8a8e59fbee31e6d8fd2e9a08b29ffb21a2fae7))
*   the remove bookmark dialog can be popped many times ([0227d99f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/0227d99f586a3eef4aebad057e9fb526a84397c2))
*   list view will to compact mode when previous list view is search file view ([e88e6fc8](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e88e6fc81da14b8c2b6bc6cf9b29103e98b84fb2))
*   mounted volume name change when execute gparted ([c5abb7e1](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c5abb7e1bd584c9b0a0472c62d0b1c48dec58118))
*   file view no update ([cf5e8bc5](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/cf5e8bc5a7df4265b5e5ec840844aab6437008de))
*   remove repeated recommend app item on open file of menu ([a60704db](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a60704dba66455d4d77f55bb85bd4679fd9ab527))
*   file view content label is wrong when load files before ([1ce54c97](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1ce54c9706312bff7b1b3d89b20a5d027c04ddd2))
*   can not select last item by PageDown/Down shortcut ([61aa19f6](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/61aa19f6b4594ecff7373e9ef96e13f7202dfdb4))
*   icon view can not scroll to end ([55fa2148](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/55fa21481b3c0bddcb506452e41db15a012ef374))
*   list mode file icon size ([5a4146da](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/5a4146dac7e2b0cd53bd457e562c4f6397d5e18b))
*   same desktop display name is gibberish ([2fe6d562](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2fe6d562b5259d4b1024464b31d515ffa1b5d37c))
*   file name is wrong ([85e3519b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/85e3519b8dcdfee26580809986c29d70ceecdb16))
*   can not drag read only files to folder ([33031c9d](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/33031c9db668ec7405dd3a26ac59ce5e046d3d46))
*   crash in DFMSettingDialog ([48c8ba1f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/48c8ba1f50ddd8877f17fd40b6d17fe21f91f3cc))
*   file icon is empty when icon theme update ([d04a0ab9](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/d04a0ab9a401cbac38edb2a6880aa9a653a97669))
*   can not clean trash ([c691b32d](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c691b32d48577599e2ad5f089c388474922dadb0))
*   can preview the ico image ([a47c522c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a47c522ca193f736413ba25c106a8419d01ab905))
*   crash at FileJob::copyDir ([ac43d8a3](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ac43d8a32c957b173c3e3bb9045ab2aea841f831))
*   can not click to open the file on computer view ([e6b4033c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e6b4033ce9502498813ed098ba39b8e3cb47dce1))
*   the file chooser dialog window type is Qt::Window ([4b53f22c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4b53f22c9f4e8f98b377c40d62e12889dad55de2))
*   disable open files by Ctrl+N ([06729f39](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/06729f39c658a0e5e0b4df4f241ce57d2949cefb))
*   can not search in android phone mount path ([4be8bfc9](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4be8bfc918b357568b512bf5ec1643e5dd546ce3))
*   stop copy action when error occurred reading from file ([7d13a879](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/7d13a879feb37ba068581f11c444d6c6ed752bc8))
*   can not move a symlink to trash if the symlink target size out of 1GB and the symlink target in other disk ([b8b80d35](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b8b80d353d5a5681972f1f794b8ae39ad98864fc))
*   crash on preview music ([16f57989](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/16f579898185967935ac0cc13b1e47da05fe427d))
*   video stop when press Super+D ([15de66aa](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/15de66aab5335606dd25da434ecd3d26648fc7e0))
*   list all symlink files include broken symlink by QDirIterator with QDir::System ([2d12ff78](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2d12ff78395a3404b4b56276b80ae923bd33d223))
*   can not restore trash files if target file is exist ([08ea0526](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/08ea0526bfc7b74f0bed2a4bb2d432fd0be72b95))
*   can not move the invalid symlink to trash ([50be2a2f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/50be2a2f66e613631a03f4a7da7e49bb1dd68a6c))
*   check dock rect failed ([a5d07f4a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a5d07f4aeb27cf193cabc64f9fabe9eaf9e4cf9d))
*   can not close computer property dialog  by CloseAllDialogIndicator ([6842e967](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/6842e9674cd637a2b3d906c70533f78c80c75256))
*   the copy file dialog position is wrong when close ([902f7707](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/902f770725ebcab99b655b20e9a69d6cc5c1260f))
*   the files icon position offset on high-DPI screen ([7c1fc4bc](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/7c1fc4bc4d37008562fb3de31afa4d00f70f21e3))
*   the icon will become larger when the file is selected ([343e3162](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/343e31625177f48ea69dbb65dd20872244e4c9c6))
*   skip moving or copying of nonexistent files ([2b0e05f5](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2b0e05f5817a183a96310374722cbf8668c00e35))
*   file icon blurred on hiDPI ([d911e1ad](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/d911e1ad3e04909b63731f502738e8bd481d2dd1))
*   watermark picture blurred on high-DPI screen ([27edfcc1](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/27edfcc138b0226900775677e0f6b6d01b4afd62))
*   the window tabs title follow other window current url to change ([7894196e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/7894196e34c0e2ffde017764c3e4f72a4075656f))
*   remove the file chooser dialog menu button ([39b9f9ca](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/39b9f9caa88a79b4770b6fd6a2d68e7f04b34ab3))
*   the title bar background color for dialogs ([49ff1f97](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/49ff1f97a47176d1790d69a7ce3dc34477a016f6))
*   keep plugins order ([2ac78b25](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2ac78b25f984eca066e585dd5844322f3dec3464))
*   adjust click area ([c3f5b814](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c3f5b8141338d65b42689f14bda2ac7c8c13c5d2))
*   all scroll bar widget width ([cf928b29](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/cf928b29b05d3ebee7c620660dde649b52c48e28))
*   the width of scroll bar of file view ([1e3c5323](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1e3c53237c3f82e5649554fde3db617d37b145dd))
*   disk mount plugin crash when disk total is 0 ([73e47995](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/73e479950230d867858a89e49191e6514ff7edf4))
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
* **desktop:**
  *  delay show wallpaper choose frame ([568bcb05](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/568bcb057e2c1c204338b1a1bca37ad899ea0dc2))
  *  remove repeated item of selection indexs ([969df4eb](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/969df4eb6b5ee041e4032eac369308323bf6cfb3))
  *  enable DDE_COMPUTER_TRASH on professional system ([2ff10bc4](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2ff10bc41acf5af37be8241cbab6fc353eee809a))

#### Features

*   support disable menu actions by config file ([e1eb2d79](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e1eb2d797f00b061f12e22c7dac6dafc7ad1204a))
*   support custom hidden files ([cec10f93](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/cec10f93f27d9bc29e320fa448dd131718072e58))
*   add compact mode for list view ([07b9aa5e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/07b9aa5eea64e3f9ef660b578a8bb7b5d2ce3fb1))
*   show current path on the window title ([b7f8f796](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b7f8f796315aea7846ec1af4c760349f171450ba))
*   add overwrite confirm dialog for DFileDialog ([949fe0a4](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/949fe0a4ed9fc99d59582cae5683c194753da792))
*   prohibit file names from beginning or ending with spaces ([4d60ee49](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4d60ee49e60797e84047d11dc2c77a6091f3c117))
*   support preview the content files of dim file ([963cd828](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/963cd82859096b1912b706f7433774aebce131d5))
*   add "QIODevice *createIODevice()" for DAbstractFileInfo ([91903dea](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/91903deaa28300089e72dd2c66aaa511d8ae3d6e))
*   direct map filesystem to config file ([aaa6c8cf](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/aaa6c8cfe45354651cf1d728db2d58bf7d3c1f36))
*   use env to debug ([93bb7280](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/93bb7280f1f1bd8b8999980af95e57f234e5c2db))
*   support multi struct parial ([ecfb6388](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ecfb6388d1edc771d642965b19a2cd67cdc01346))
* **dock disk plugin:**  update disk info when show ([a6b12a69](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a6b12a69d80e39535713e30f1baf3b7bca5f1d94))
* **usb-device-formatter:**  support HI-DPI ([1e7fc948](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1e7fc948d894552a4571c9afd73d5594036e8e0b))



<a name="4.4.8.5"></a>
## 4.4.8.5 (2018-04-10)


#### Features

*   add compact mode for list view ([07b9aa5e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/07b9aa5eea64e3f9ef660b578a8bb7b5d2ce3fb1))
*   show current path on the window title ([b7f8f796](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b7f8f796315aea7846ec1af4c760349f171450ba))
*   add overwrite confirm dialog for DFileDialog ([949fe0a4](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/949fe0a4ed9fc99d59582cae5683c194753da792))

#### Bug Fixes

*   can not drag read only files to folder ([33031c9d](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/33031c9db668ec7405dd3a26ac59ce5e046d3d46))
*   crash in DFMSettingDialog ([48c8ba1f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/48c8ba1f50ddd8877f17fd40b6d17fe21f91f3cc))
*   file icon is empty when icon theme update ([d04a0ab9](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/d04a0ab9a401cbac38edb2a6880aa9a653a97669))
*   can not clean trash ([c691b32d](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c691b32d48577599e2ad5f089c388474922dadb0))
*   can preview the ico image ([a47c522c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a47c522ca193f736413ba25c106a8419d01ab905))
*   crash at FileJob::copyDir ([ac43d8a3](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ac43d8a32c957b173c3e3bb9045ab2aea841f831))
*   can not click to open the file on computer view ([e6b4033c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e6b4033ce9502498813ed098ba39b8e3cb47dce1))
*   the file chooser dialog window type is Qt::Window ([4b53f22c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4b53f22c9f4e8f98b377c40d62e12889dad55de2))
*   disable open files by Ctrl+N ([06729f39](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/06729f39c658a0e5e0b4df4f241ce57d2949cefb))
*   can not search in android phone mount path ([4be8bfc9](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4be8bfc918b357568b512bf5ec1643e5dd546ce3))
*   stop copy action when error occurred reading from file ([7d13a879](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/7d13a879feb37ba068581f11c444d6c6ed752bc8))
*   can not move a symlink to trash if the symlink target size out of 1GB and the symlink target in other disk ([b8b80d35](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b8b80d353d5a5681972f1f794b8ae39ad98864fc))
*   crash on preview music ([16f57989](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/16f579898185967935ac0cc13b1e47da05fe427d))
*   video stop when press Super+D ([15de66aa](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/15de66aab5335606dd25da434ecd3d26648fc7e0))



<a name="4.4.8.3"></a>
##  (2018-03-22)


#### Bug Fixes

*   list all symlink files include broken symlink by QDirIterator with QDir::System ([2d12ff78](4.4.8.3/commit/2d12ff78395a3404b4b56276b80ae923bd33d223))
*   can not restore trash files if target file is exist ([08ea0526](4.4.8.3/commit/08ea0526bfc7b74f0bed2a4bb2d432fd0be72b95))
*   can not move the invalid symlink to trash ([50be2a2f](4.4.8.3/commit/50be2a2f66e613631a03f4a7da7e49bb1dd68a6c))
*   check dock rect failed ([a5d07f4a](4.4.8.3/commit/a5d07f4aeb27cf193cabc64f9fabe9eaf9e4cf9d))
*   can not close computer property dialog  by CloseAllDialogIndicator ([6842e967](4.4.8.3/commit/6842e9674cd637a2b3d906c70533f78c80c75256))
*   the copy file dialog position is wrong when close ([902f7707](4.4.8.3/commit/902f770725ebcab99b655b20e9a69d6cc5c1260f))

#### Features

*   prohibit file names from beginning or ending with spaces ([4d60ee49](4.4.8.3/commit/4d60ee49e60797e84047d11dc2c77a6091f3c117))



<a name="4.4.8.2"></a>
## 4.4.8.2 (2018-03-15)


#### Bug Fixes

*   the files icon position offset on high-DPI screen ([7c1fc4bc](4.4.8.2/commit/7c1fc4bc4d37008562fb3de31afa4d00f70f21e3))
*   the icon will become larger when the file is selected ([343e3162](4.4.8.2/commit/343e31625177f48ea69dbb65dd20872244e4c9c6))
*   skip moving or copying of nonexistent files ([2b0e05f5](4.4.8.2/commit/2b0e05f5817a183a96310374722cbf8668c00e35))
*   file icon blurred on hiDPI ([d911e1ad](4.4.8.2/commit/d911e1ad3e04909b63731f502738e8bd481d2dd1))
*   watermark picture blurred on high-DPI screen ([27edfcc1](4.4.8.2/commit/27edfcc138b0226900775677e0f6b6d01b4afd62))
*   the window tabs title follow other window current url to change ([7894196e](4.4.8.2/commit/7894196e34c0e2ffde017764c3e4f72a4075656f))
*   remove the file chooser dialog menu button ([39b9f9ca](4.4.8.2/commit/39b9f9caa88a79b4770b6fd6a2d68e7f04b34ab3))
*   the title bar background color for dialogs ([49ff1f97](4.4.8.2/commit/49ff1f97a47176d1790d69a7ce3dc34477a016f6))
*   keep plugins order ([2ac78b25](4.4.8.2/commit/2ac78b25f984eca066e585dd5844322f3dec3464))
* **desktop:**  enable DDE_COMPUTER_TRASH on professional system ([2ff10bc4](4.4.8.2/commit/2ff10bc41acf5af37be8241cbab6fc353eee809a))

#### Features

*   support preview the content files of dim file ([963cd828](4.4.8.2/commit/963cd82859096b1912b706f7433774aebce131d5))
*   add "QIODevice *createIODevice()" for DAbstractFileInfo ([91903dea](4.4.8.2/commit/91903deaa28300089e72dd2c66aaa511d8ae3d6e))



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



