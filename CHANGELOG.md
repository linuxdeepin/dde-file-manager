<a name="4.8.6.4"></a>
## 4.8.6.4 (2019-06-12)


#### Bug Fixes

*   DFileDialog disable menu from titlebar ([e2f5c9a9](https://github.com/linuxdeepin/dde-file-manager/commit/e2f5c9a9bef2dadd261d83931985fbcf4f3631a3))
*   typos in multiple DFMEvent subclasses. ([de2e33a6](https://github.com/linuxdeepin/dde-file-manager/commit/de2e33a6aad845f12e529c9786256364f5cafa14))



<a name="4.8.6.3"></a>
## 4.8.6.3 (2019-06-11)


#### Bug Fixes

*   copy dialog disable menu from titlebar ([daa6d016](https://github.com/linuxdeepin/dde-file-manager/commit/daa6d0168da0d17b8dbb92705eef8a81fc192f09))
*   should remember auto merged status ([9eda50be](https://github.com/linuxdeepin/dde-file-manager/commit/9eda50be4f33973fb38bfb34217dc629bbf3529a))



<a name="4.8.6.2"></a>
## 4.8.6.2 (2019-06-05)


#### Bug Fixes

*   should convert to QVariant first ([80ea04c7](https://github.com/linuxdeepin/dde-file-manager/commit/80ea04c79c27f5cbfa66f0aeff68697c0bb78b94))



<a name="4.8.6.1"></a>
## 4.8.6.1 (2019-06-01)


#### Bug Fixes

*   should require auth ([dcc549c2](https://github.com/linuxdeepin/dde-file-manager/commit/dcc549c2b3ac6aa9af1552b3d813e3027464925e))
*   remove com.deepin.filemanager.daemon.policy ([b18c95b2](https://github.com/linuxdeepin/dde-file-manager/commit/b18c95b271b263cb52e26771b80b1bff8ecedbcc))
*   several usb-device-formatter issues ([155fa8cf](https://github.com/linuxdeepin/dde-file-manager/commit/155fa8cf33b26c6dee870af5dbbff4b8e4aa6f6c))
*   lsar -J result changed after util-linux v2.33 ([64247173](https://github.com/linuxdeepin/dde-file-manager/commit/64247173a042283529ee0118e0ad442227bd9a13))
*   wallpaper geometry not updated after switching monitor configuration. ([2e98db25](https://github.com/linuxdeepin/dde-file-manager/commit/2e98db2556eb0fa78ad1d75e8e428f89105f9801))
*   merged desktop HiDPI icon blur ([2d1d1b2d](https://github.com/linuxdeepin/dde-file-manager/commit/2d1d1b2dfc9a0342639081f9c85843651e643678))
*   redirect once for share panel in property dialog ([a5a35225](https://github.com/linuxdeepin/dde-file-manager/commit/a5a3522592822dab38cd74777ee7bcdb7683042b))
*   update the geometry of the background widget before emit the backgroundGeometryChanged signal ([e23460b5](https://github.com/linuxdeepin/dde-file-manager/commit/e23460b5858c0c829a487794d4b1b52824b07484))
*   desktop icons obscured by dock if scaling is enabled ... ([4b64fa31](https://github.com/linuxdeepin/dde-file-manager/commit/4b64fa312192df4d8434b268eb8cd32cf83479e6))
*   also try redirected URL for previewing. ([69e01d9a](https://github.com/linuxdeepin/dde-file-manager/commit/69e01d9a541c49c3012f74e77aeaf4dfdd762898))
*   dde-file-manager/dde-xdg-user-dirs-update set to executable ([10deedc8](https://github.com/linuxdeepin/dde-file-manager/commit/10deedc89191bf683f594ab7748e66fb24589adc))
*   crash if cannot found Delete menu action for Recent scheme ([082a2701](https://github.com/linuxdeepin/dde-file-manager/commit/082a27014fc0617c238a449667a83993387edcd9))
* **dev-formatter:**  use customQtThemeConfigPathByUserHome() ([6da134e7](https://github.com/linuxdeepin/dde-file-manager/commit/6da134e7f9778714d23aec32f510cd29765aa1ef))



<a name="4.8.5.1"></a>
## 4.8.5.1 (2019-05-24)


#### Bug Fixes

*   Anything support disabled for non x86 arch ([990795ff](https://github.com/linuxdeepin/dde-file-manager/commit/990795ff3a9863b52afdd02558864855f7b9d0c7))



<a name="4.8.5"></a>
## 4.8.5 (2019-05-23)


#### Bug Fixes

*   desktop won't start when fileDialogOnly process exist ([a9b91e0c](https://github.com/linuxdeepin/dde-file-manager/commit/a9b91e0cdc6a2658d8abaadc8056c531654f80d9))
*   use customQtThemeConfigPathByUserHome() ([1275fdb0](https://github.com/linuxdeepin/dde-file-manager/commit/1275fdb0e8167bebc978b21510a72cd954ff6899))
*   sidebar bookmark item cannot click after reorder ([1f0e9f3d](https://github.com/linuxdeepin/dde-file-manager/commit/1f0e9f3d49e69a53d35f9961cae662a47141065e))
*   should no longer show virtual entry property dialog ([c41bf089](https://github.com/linuxdeepin/dde-file-manager/commit/c41bf0892ad4364ee346e26cfb94c37a333d7ffa))
*   disk size sometimes can display negative number ([63d08b6a](https://github.com/linuxdeepin/dde-file-manager/commit/63d08b6af80cf42acff0dc3f7f44f4fb961b03fb))
*   drag file to other app from mergeddesktop ([2bd8374d](https://github.com/linuxdeepin/dde-file-manager/commit/2bd8374d8f85b6ed1a5fcfb2096666d240fd96cf))
*   search keyword contains brackets ([b256b5b3](https://github.com/linuxdeepin/dde-file-manager/commit/b256b5b39d2c8504fc5e9f615609f775426eb5a3))

#### Features

*   check if expand entry will make desktop full ([afd87d71](https://github.com/linuxdeepin/dde-file-manager/commit/afd87d71a59d067649e02fe57d16fa20ee6db13b))
* **pro:**  able to disable desktop shortcut and context menu ([3e05fe49](https://github.com/linuxdeepin/dde-file-manager/commit/3e05fe499973d786638108bc1c0b189f68ad2588))



<a name="4.8.4"></a>
## 4.8.4 (2019-05-09)


#### Bug Fixes

*   hide indexing database in NTFS partition for Windows. ([4e8e6282](https://github.com/linuxdeepin/dde-file-manager/commit/4e8e6282654f36d46eca6c4520bc88712e77fc43))
*   loop call of update usershare ([0f58c7dc](https://github.com/linuxdeepin/dde-file-manager/commit/0f58c7dc8c9f7e27165f4245b666d1ef0d566a4b))
*   debug build FTBFS. ([eabfe29c](https://github.com/linuxdeepin/dde-file-manager/commit/eabfe29c7029b08cdb9d548afe83896d4ec513e8))
*   should create fileinfo to get local file ([64a95a81](https://github.com/linuxdeepin/dde-file-manager/commit/64a95a81305d7547374fbf13d1095a2f7ebed80b))
*   always update mergeddesktop file data ([98a20187](https://github.com/linuxdeepin/dde-file-manager/commit/98a201876e992751fb1b605c57a3a34302dd516b))
*   unable to access mtp mount device ([e4468c06](https://github.com/linuxdeepin/dde-file-manager/commit/e4468c06317ce52f51f7e7c59f990af3a2adea0e))
*   file mimetype results are not accurate on gvfs mount device ([55c050ea](https://github.com/linuxdeepin/dde-file-manager/commit/55c050eae00c6a59a8fef74cc98dcc6e2f85ba0d))
*   Update usershare info when getChildren ([aad94bdc](https://github.com/linuxdeepin/dde-file-manager/commit/aad94bdce2782ecb77388d6a0722c6e740e2d14b))
*   should create file info to get permissions ([33aba8ab](https://github.com/linuxdeepin/dde-file-manager/commit/33aba8ab0089a915ce46a7a6d30b098aee4f0a04))
*   fix typo in method name. ([cc715000](https://github.com/linuxdeepin/dde-file-manager/commit/cc715000b39c218538a6e5ba69cceeaa249e4ce1))
*   file permission error ([36d60f88](https://github.com/linuxdeepin/dde-file-manager/commit/36d60f88c03f2c4cd949fa3ec585712da4b9d4c7))
* **dde-desktop:**  the background window size and screen size are inconsistent ([d212babd](https://github.com/linuxdeepin/dde-file-manager/commit/d212babd50e6c4492f8f73d790e38290b153dbe4))



<a name="4.8.3"></a>
## 4.8.3 (2019-04-25)


#### Bug Fixes

*   temporary disable paste on merged desktop ([55d0c9e5](https://github.com/linuxdeepin/dde-file-manager/commit/55d0c9e58bd854b831c14240b8c63f410b5d2bcf))



<a name="4.8.2"></a>
## 4.8.2 (2019-04-25)


#### Bug Fixes

*   workaround for merged desktop ([b7a2f85e](https://github.com/linuxdeepin/dde-file-manager/commit/b7a2f85e907fd04759fb8ca77791778200e3e21f))
*   desktop widget hierarchy caused glitch ([8d329d86](https://github.com/linuxdeepin/dde-file-manager/commit/8d329d864c48d188680d0fd83e27241e997cdac8))
*   avoid save profile when using merged desktop ([1579ac11](https://github.com/linuxdeepin/dde-file-manager/commit/1579ac11ebf722715c00d1478000fb783d347071))
*   copy status not correct ([bd07342d](https://github.com/linuxdeepin/dde-file-manager/commit/bd07342d60f4f3c5fc91ba6e94fc4579684364f8))
* **dde-wallpaper:**  find wallpaper thumbnails with no results (#146) ([854383fb](https://github.com/linuxdeepin/dde-file-manager/commit/854383fb96f4ce1b9d7f257a9440ceec1c53494d))



<a name="4.8.1"></a>
## 4.8.1 (2019-04-22)


#### Features

*   fileItemDisableFlag() for merged desktop ([8e328e33](https://github.com/linuxdeepin/dde-file-manager/commit/8e328e33a334cc3cb5e88415c3408ed3f82ee98e))
*   file watcher support for dfmmd scheme ([ca5d0486](https://github.com/linuxdeepin/dde-file-manager/commit/ca5d048664078627030ff1796e0f0eae091de201))

#### Bug Fixes

*   duplicated items from merged desktop controller ([ad3d514d](https://github.com/linuxdeepin/dde-file-manager/commit/ad3d514d23d46e1abffae7b49198fae88fe12f9e))
*   disable drag on merged desktop ([40d88ae7](https://github.com/linuxdeepin/dde-file-manager/commit/40d88ae7ed33af8f7877456be5848cd0d173eae3))
*   invalid command args quotes for QProcess ([6b27a341](https://github.com/linuxdeepin/dde-file-manager/commit/6b27a3419c05ee4141e98a6d2c45429dbed6e50b))
*   avoid crashing on file stat job. ([846ad1c3](https://github.com/linuxdeepin/dde-file-manager/commit/846ad1c3979d605e416cfec0c3d4305fc2b25cbd))
*   use plural for zero items. ([e0aa1e69](https://github.com/linuxdeepin/dde-file-manager/commit/e0aa1e694694647505ed6b34deda808478a1f5ba))
*   merge order caused compile error ([39a1e331](https://github.com/linuxdeepin/dde-file-manager/commit/39a1e331d00a5cc201376a15a2c0359916ca67bb))
*   some merged desktop check ([7f2acf05](https://github.com/linuxdeepin/dde-file-manager/commit/7f2acf054f894cf141770876c1c7239f989301d8))
*   The desktop background flash when the wallpaper is changed by user setting ([fc112bde](https://github.com/linuxdeepin/dde-file-manager/commit/fc112bdec9de480da2f11e8129dc6b6ce89b5166))
*   use macro ([3a655f5c](https://github.com/linuxdeepin/dde-file-manager/commit/3a655f5cc608d388e68b486fb20fd1f476bdc240))
*   avoid crash in some case ([42abe05b](https://github.com/linuxdeepin/dde-file-manager/commit/42abe05b24b082742ba65f2335956ed553b89962))
*   using keyboard navigation causes wrong number of files in the status bar. ([5b74ac8c](https://github.com/linuxdeepin/dde-file-manager/commit/5b74ac8c8eee521fc360facba76f1db11961cc5d))
*   wrong number of items in disk properties dialog. ([4c78d270](https://github.com/linuxdeepin/dde-file-manager/commit/4c78d2709b2ba98f1537a1a3ebdeed9f535c6d4d))
*   can open terminal under merged desktop scheme ([9795c6eb](https://github.com/linuxdeepin/dde-file-manager/commit/9795c6eb79d7111d02e7336f57b2d895023d14ab))
*   show hidden file on desktop not working ([dd00701e](https://github.com/linuxdeepin/dde-file-manager/commit/dd00701e1ec111a0cc990837220931ae38e602c5))
*   list of recently used files empties unexpectedly. https://github.com/linuxdeepin/internal-discussion/issues/1090 ([d9545d68](https://github.com/linuxdeepin/dde-file-manager/commit/d9545d684ede1ee11ee3fd660375222e5100563c))
*   right mouse button drag on virtual entry lead crash ([e8336f04](https://github.com/linuxdeepin/dde-file-manager/commit/e8336f040946be8ad5302be3adc357113dfdfbf9))
*   unknown mtime should show up properly now. ([e1f85519](https://github.com/linuxdeepin/dde-file-manager/commit/e1f85519c59f35a8ca82cee52c13b19b2f80bcce))
*   disable desktop context menu for merged desktop mode (#121) ([f05f9e5a](https://github.com/linuxdeepin/dde-file-manager/commit/f05f9e5a3a5ef8efdc36ec0ea807d5855c8751ff))
* **desktop:**  will show titlebar in a short time ([496d722c](https://github.com/linuxdeepin/dde-file-manager/commit/496d722c35a75384c39d9b08393764e5822ecdbb))



<a name="4.8.0"></a>
## 4.8.0 (2019-04-09)


#### Bug Fixes

*   also prompt the user to set executable bits for ELF files. ([ad9910df](https://github.com/linuxdeepin/dde-file-manager/commit/ad9910dfcfc3f900972db8a91d1883fdd10183bf))
*   crash on create file (#115) ([74beb59e](https://github.com/linuxdeepin/dde-file-manager/commit/74beb59ebfda31382fee666ccbf99e9a3962770b))
*   stray question mark in string. (#114) ([45029ef3](https://github.com/linuxdeepin/dde-file-manager/commit/45029ef38e149f30e4f0bcf15ea88e54121b09d4))
* **dialog:**  default button and recommend button not same (#112) ([ac591df8](https://github.com/linuxdeepin/dde-file-manager/commit/ac591df87bab4440c71a0a4a143ed9a7dc19c665))

#### Features

*   merged desktop (#118) ([96e69027](https://github.com/linuxdeepin/dde-file-manager/commit/96e69027c6a87ed13b1b834130baae2bdbb01b31))
*   implement clear recent history. (#111) ([e25bca7d](https://github.com/linuxdeepin/dde-file-manager/commit/e25bca7d43cbeaa4b6b2f46560efad426beac59c))



<a name="4.7.9"></a>
## 4.7.9 (2019-04-04)


#### Bug Fixes

*   can not close wallpaper settings view on click empty area ([dab419cb](https://github.com/linuxdeepin/dde-file-manager/commit/dab419cb213bb45c0b645a6337135e0a02c5b730))
*   zero size plain text file is not executable script (#108) ([5707925e](https://github.com/linuxdeepin/dde-file-manager/commit/5707925e0aed73af7f8c0509e1005f3d66fe5482))
*   the mode switch widget overlaps with other widgets ([58b0f7a0](https://github.com/linuxdeepin/dde-file-manager/commit/58b0f7a093155fdc2dfd7915efc424c39358c12b))
*   file search view property dialog is empty (#104) ([11ecac82](https://github.com/linuxdeepin/dde-file-manager/commit/11ecac82d04f082f41788988b41486c637609ba2))
*   dock mount plugin label content as plain text (#101) ([ecb7cbd0](https://github.com/linuxdeepin/dde-file-manager/commit/ecb7cbd07cb4c190f74da8c7fe8d995e16dc2a94))
*   potential crash when an optical media is inserted. (#100) ([52d1cf67](https://github.com/linuxdeepin/dde-file-manager/commit/52d1cf6783b6a9a437a279a96fb4eeb2099b4320))
*   optical drive detection. (#99) ([2a524d63](https://github.com/linuxdeepin/dde-file-manager/commit/2a524d633f172362fe55b4538816a3efbdc9c702))
*   avoid crash when we cannot got device info (#98) ([24d1b3cd](https://github.com/linuxdeepin/dde-file-manager/commit/24d1b3cd5ec810f8443b614a5eae1edfffe93819))
*   Nothing on desktop in multi-screen and copy mode ([46f0028a](https://github.com/linuxdeepin/dde-file-manager/commit/46f0028a43eaeff6f8a20dd827a3245ef84e1ff0))
* **Desktop:**  use right-click menu when use window scale ratio, desktop will draw the content misplaced ([91c00d2d](https://github.com/linuxdeepin/dde-file-manager/commit/91c00d2d0b5f59cc05a7a2c77b28dbdf2bf4dcec))

#### Features

*   mount disk image. (#110) ([9dace08c](https://github.com/linuxdeepin/dde-file-manager/commit/9dace08c2eb697a8b10447f61f974fd16be271fb))
*   context menu icon visible state now configurable (#96) ([e2b4ff1f](https://github.com/linuxdeepin/dde-file-manager/commit/e2b4ff1f90df9d8ce401e0ae4f91ca48e6909b33))



<a name="4.7.8.2"></a>
## 4.7.8.2 (2019-03-29)


#### Bug Fixes

*   crash when open systemdisk property (#93) ([5c8d4b6f](https://github.com/linuxdeepin/dde-file-manager/commit/5c8d4b6f194d8b1b7923dc0435e888530dbd8f24))



<a name="4.7.8.1"></a>
## 4.7.8.1 (2019-03-29)


#### Bug Fixes

*   use setPlainText for computer view items (#91) ([f3a53417](https://github.com/linuxdeepin/dde-file-manager/commit/f3a534178da8fb7a6acd9d80544ff2c0aee84f17))
*   build error if enable DISABLE_ANYTHING ([1358fa39](https://github.com/linuxdeepin/dde-file-manager/commit/1358fa39a3331d1cd4bc7f9879f63133623b21a5))
*   MenuAction::Unknow is no longer the last one (#89) ([931c97f8](https://github.com/linuxdeepin/dde-file-manager/commit/931c97f8c73c6aa535e9979970dec40138ccd6a2))
*   allowed find parent view mode (#88) ([857eb615](https://github.com/linuxdeepin/dde-file-manager/commit/857eb6152d0fe4a7c3daf09f9cccef7df7a3ca78))



<a name="4.7.8"></a>
## 4.7.8 (2019-03-27)


#### Bug Fixes

*   user dirs changed to English in other DEs (#67) ([b84dde56](https://github.com/linuxdeepin/dde-file-manager/commit/b84dde56ffc56a744e92dea802a8da892255ef16))
*   whel not working on ComputerView (#82) ([2191f7d3](https://github.com/linuxdeepin/dde-file-manager/commit/2191f7d3a5d4f0c4ac90f8fd3314260bd9834dbe))
*   disk property display name not correct ([832ffda4](https://github.com/linuxdeepin/dde-file-manager/commit/832ffda4d1654b1bb20cc4646169761be137b84b))
*   set proper working directory when running executables ([4d861c9b](https://github.com/linuxdeepin/dde-file-manager/commit/4d861c9bafa67015d30487162753de5b669b2e13))
*   UI stuck when searching ([266b285a](https://github.com/linuxdeepin/dde-file-manager/commit/266b285a8e02b944dc0faeb2716ea2d052ffc9ac))
*   locker protect for handle xbel file changed ([70021a74](https://github.com/linuxdeepin/dde-file-manager/commit/70021a74462684109eebb86dac8baa1aecbf8a20))
*   use concurrent to handle recent file changed ([e7a23aed](https://github.com/linuxdeepin/dde-file-manager/commit/e7a23aed0333d4a47e0c315f82b758e35a85c671))
*   can't use anything for subdirectory ([79778fa5](https://github.com/linuxdeepin/dde-file-manager/commit/79778fa55201fa0aa1e51bedf7f57c93d49b5550))
*   user share dialog cannot open twice ([92e33ede](https://github.com/linuxdeepin/dde-file-manager/commit/92e33ede74f9603f3ae7de33c08be32f51f63ecc))
* **dde-file-manager:**  crash at getNodeByIndex ([cc5031e0](https://github.com/linuxdeepin/dde-file-manager/commit/cc5031e0bb92d85cb346519d705562df8c673182))
* **dde-wallpaper:**  the mode switch widget overlaps with other widgets ([20d62130](https://github.com/linuxdeepin/dde-file-manager/commit/20d62130f343ad1107d11c7ab40f3fb611459b30))

#### Features

*   menu plugin can know menu is triggered on desktop (#78) ([24bb2f89](https://github.com/linuxdeepin/dde-file-manager/commit/24bb2f892bc0b9b23f678e8794a5bf5a4b5e4cfc))
*   read ~/.dde_env when run via pkexec ([8e04d77b](https://github.com/linuxdeepin/dde-file-manager/commit/8e04d77bc3eda884ebe4ec861a114d1f6dfa4bd5))
*   add backup file type for deepin-clone ([6646405a](https://github.com/linuxdeepin/dde-file-manager/commit/6646405a07a83ceac6f8672665e8dc8f93784c68))
*   support copy file on preview dialog ([92a15496](https://github.com/linuxdeepin/dde-file-manager/commit/92a15496587232e47e7bacdaa69a373beb70d59e))
*   get the search results with segment of quick search ([696a93b1](https://github.com/linuxdeepin/dde-file-manager/commit/696a93b152158e7cb7126aff116a64b260409b66))
*   handle sigterm ([c85a7ba4](https://github.com/linuxdeepin/dde-file-manager/commit/c85a7ba4c83271513aa9a6e7c2efce899b3af144))
* **dde-wallpaper:**  support wallpaper slideshow ([d371c5d1](https://github.com/linuxdeepin/dde-file-manager/commit/d371c5d120a036d3931a08332ae3ca29a30232c8))



<a name="4.7.7.1"></a>
## 4.7.7.1 (2019-03-04)


#### Features

*   notify on device got disconnected ([64e9ce81](https://github.com/linuxdeepin/dde-file-manager/commit/64e9ce81526614df358085e2dc821923ac753749))
*   Ctrl+I to show property dialog on desktop ([fbc650a6](https://github.com/linuxdeepin/dde-file-manager/commit/fbc650a6feea4c88b08e90de76a5da7f8e02269f))
*   show source path on property dialog for trash url ([25a44d7f](https://github.com/linuxdeepin/dde-file-manager/commit/25a44d7f1514af63ac09261b6fe1fef334179f70))

#### Bug Fixes

*   enter ~ navigate to home dir ([e0440d76](https://github.com/linuxdeepin/dde-file-manager/commit/e0440d76daa92e3229b11ad380fb7b4421783ce3))
*   crash in loongson platform ([648ce30d](https://github.com/linuxdeepin/dde-file-manager/commit/648ce30d86ecf760db611987884788f3a1ff6209))
*   fs type for /home ([72e49a99](https://github.com/linuxdeepin/dde-file-manager/commit/72e49a99c0bba445c86354ec4a0d2ebe6abe37de))
*  cannot browse Samsung devices ([ef95c920](https://github.com/linuxdeepin/dde-file-manager/commit/ef95c920cc7b83f35cf7fddb5a1d51a204a465a4))



<a name="4.7.7"></a>
## 4.7.7 (2019-02-25)


#### Features

*   disable deepin anything on arm64(aarch64) ([359dd607](https://github.com/linuxdeepin/dde-file-manager/commit/359dd60722706c47c291c4ac329d007beb462404))
*   crumb item context menu ([3dc9d110](https://github.com/linuxdeepin/dde-file-manager/commit/3dc9d110682611a06c422e6bf10b496f1262b927))
*   hidden option to control crumbbar clickable area visible ([c1faa8e1](https://github.com/linuxdeepin/dde-file-manager/commit/c1faa8e160f3ad5693d906ecbcabf3d9a3edda66))
*   remove addressbar clickable area ([e21c64d7](https://github.com/linuxdeepin/dde-file-manager/commit/e21c64d75906ba3ee68340d71e82bf439ef32a2f))
* **dde-wallpaper:**  enable screensaver function for deepin professional ([4ae45381](https://github.com/linuxdeepin/dde-file-manager/commit/4ae453812c2e00b5017c798146829c06d61ab4af))

#### Bug Fixes

*   only do show filedialog via dbus call ([b6e97adc](https://github.com/linuxdeepin/dde-file-manager/commit/b6e97adcc6472b9de98107521d23c8d5ed283d7e))
*   ComputerPropertyDialog value label width not correct ([927a83aa](https://github.com/linuxdeepin/dde-file-manager/commit/927a83aa5605da714b38a190160653d5db1c69a7))
*   open folder when select a folder and click open in DFileDialog ([882f2295](https://github.com/linuxdeepin/dde-file-manager/commit/882f2295b73f278c7a33292c4b47c26267a90310))
*   wrong search button icon on admin mode ([4233adb0](https://github.com/linuxdeepin/dde-file-manager/commit/4233adb0fee6fd843173d84c76a686d91af90839))
*   use default-terminal provided by deepin-daemon if possible ([3ba6625c](https://github.com/linuxdeepin/dde-file-manager/commit/3ba6625c624e810e27c1a0dea3556955227f8b1d))



<a name="4.7.6"></a>
## 4.7.6 (2019-02-11)

fix: add missing header file

## 4.7.5 (2019-01-25)


#### Bug Fixes

*   won't select file after file rename or create ([8469d743](https://github.com/linuxdeepin/dde-file-manager/commit/8469d743d60ea3b892d9b7590aa127d7c3815105))
*   dbus service exec argv use -d ([414f6c03](https://github.com/linuxdeepin/dde-file-manager/commit/414f6c03a03a074d3b44bb82ac53ebdf893af501))
*   update status bar when fileview data change ([45fd6f1b](https://github.com/linuxdeepin/dde-file-manager/commit/45fd6f1b00cfebae89b82cc4dfccc58219e07c2c))
*   search filter date use range ([ffc3e5bb](https://github.com/linuxdeepin/dde-file-manager/commit/ffc3e5bb6855fa085c4ad98a0ef6609ec4c2e45c))
*   advance search bar i18n ([ca5fd082](https://github.com/linuxdeepin/dde-file-manager/commit/ca5fd0822bc61d23a40f77af7555c0c851cb239f))
*   advance search file size range ([65313b00](https://github.com/linuxdeepin/dde-file-manager/commit/65313b00a2abbb1788070afa425ebb69861e7453))
*   don't watch whole locale share dir for recent file support ([4e4ec08f](https://github.com/linuxdeepin/dde-file-manager/commit/4e4ec08fc13e969047538faabe85acb4ebf054d3))
*   wrong sort order in recentfile view ([7c9b8312](https://github.com/linuxdeepin/dde-file-manager/commit/7c9b8312f6f4f862fa8f9b57696f42fa43ce6b80))
*   avoid using dpkg-parsechangelog ([76189d11](https://github.com/linuxdeepin/dde-file-manager/commit/76189d11eba0d497b7b7cab578292feab6108d9c))
*   update dde-dock-dev package version limit ([bbb3d0e9](https://github.com/linuxdeepin/dde-file-manager/commit/bbb3d0e9279c7fd6a02433b5c5ab6d1e14fa25ea))
*   not apply saved view state in some case ([903bb1fe](https://github.com/linuxdeepin/dde-file-manager/commit/903bb1fe4da34245c0e4853739fdfb6e83b9b0c5))
* **dde-desktop:**  the application will be auto quit on sw_64 platform ([5ac98c53](https://github.com/linuxdeepin/dde-file-manager/commit/5ac98c53bde00169ecec03424632cd4714786343))

#### Features

*   advance search ([419e539e](https://github.com/linuxdeepin/dde-file-manager/commit/419e539e74242bd2ef13dea254182aa510061dcd))



<a name="4.7.4.4"></a>
## 4.7.4.4 (2019-01-15)


#### Bug Fixes

*   chinese garbled ([138c9c08](https://github.com/linuxdeepin/dde-file-manager/commit/138c9c08ff1a5a3dd4186b6816edbecb85d58ba6))
*   remove debug output which should be deleted ([3ba87c74](https://github.com/linuxdeepin/dde-file-manager/commit/3ba87c74953c3198726832ec95e97345a5c480d6))
*   only apply selection change in current view ([9edf74dc](https://github.com/linuxdeepin/dde-file-manager/commit/9edf74dc32879c50c65ffd56225a912ef5ce5c06))
*   avoid crash when paste file to unsupported url ([63273c3e](https://github.com/linuxdeepin/dde-file-manager/commit/63273c3ee988fbb651138d0bd1edb5946ccc38eb))



<a name="4.7.4.3"></a>
## 4.7.4.3 (2019-01-11)


#### Bug Fixes

*   try stop device when do eject from sidebar ([f7d5be54](https://github.com/linuxdeepin/dde-file-manager/commit/f7d5be544e2cee792d3adb4aa12758508a39270b))
*   do powerOff if a external disk can powerOff ([1225cc3b](https://github.com/linuxdeepin/dde-file-manager/commit/1225cc3b1a64809d4bbbe3e09af2703dbf62ef78))



<a name="4.7.4.2"></a>
## 4.7.4.2 (2019-01-10)


#### Bug Fixes

*   detect format by content when generate thumbnail for wallpaper ([e37f55ae](https://github.com/linuxdeepin/dde-file-manager/commit/e37f55aec15b2e75c10eefc77988a9f2b33657f5))
*   crash when copy file ([434e4502](https://github.com/linuxdeepin/dde-file-manager/commit/434e4502a56aff3c1815772e22a32d0824bf67d9))
*   DnD on device sidebar item ([203fc1df](https://github.com/linuxdeepin/dde-file-manager/commit/203fc1dffdd9e47f4f49d340facbe2fe080bc0a3))
* **recent:**  disable drag and drop files to sidebar. ([ff701a37](https://github.com/linuxdeepin/dde-file-manager/commit/ff701a37028a97b37a58f8fcb8f382936eb2e96c))

#### Features

*   display icon at context menu in QT_DEBUG build ([f409c782](https://github.com/linuxdeepin/dde-file-manager/commit/f409c782bb813e81ead8759dab448a476c77f41a))



<a name="4.7.4.1"></a>
## 4.7.4.1 (2019-01-07)


#### Bug Fixes

*   build failed on deepin_professional ([dd86e610](https://github.com/linuxdeepin/dde-file-manager/commit/dd86e6108b210570bbb57b0ff01d1a8c8c730399))
* **dde-wallpaper:**  not stop screensaver preview when switch to set wallpaper mode ([a76b22e0](https://github.com/linuxdeepin/dde-file-manager/commit/a76b22e0d09c3c6e27c5b0cfdcb719c238a44d41))



<a name="4.7.4"></a>
## 4.7.4 (2019-01-07)


#### Bug Fixes

* **dde-desktop:**
  *  fix the "Wallpaper" to "Set Wallpaper" ([279251f2](https://github.com/linuxdeepin/dde-file-manager/commit/279251f2665fcc86339a35ed90383e9551a33311))
  *  translate error: "Wallpaper" on chinese ([dfa567da](https://github.com/linuxdeepin/dde-file-manager/commit/dfa567da1aa5366b271c7e6a540718f186845142))
* **dde-wallpaper:**  the tool widgets and the switch mode widget is overlap ([29d7c09d](https://github.com/linuxdeepin/dde-file-manager/commit/29d7c09d3d1962f215797f54db24595267cbb267))



<a name="4.7.3"></a>
## 4.7.3 (2019-01-03)


#### Bug Fixes

*   mute error dialog for some type ([613ec1e7](https://github.com/linuxdeepin/dde-file-manager/commit/613ec1e733840b1bc1cb79856fbfd0c3f7d878b7))



<a name="4.7.2"></a>
## 4.7.2 (2019-01-02)


#### Bug Fixes

*   can disable screensaver on dde-desktop project ([d0f5393b](https://github.com/linuxdeepin/dde-file-manager/commit/d0f5393baf23947669785d00bdb532bc3cb153c6))
* **dde-wallpaper:**  the svg cover image is blurry on HI-DPI ([3293f4b4](https://github.com/linuxdeepin/dde-file-manager/commit/3293f4b4021256eebffecd07d429947f8a2e893a))



<a name="4.7.1.12"></a>
## 4.7.1.12 (2018-12-29)


#### Bug Fixes

*   no longer treat home dir as template dir ([87662ed9](https://github.com/linuxdeepin/dde-file-manager/commit/87662ed928f795ed12203a1b79a1f437e949ba8f))



<a name="4.7.1.11"></a>
## 4.7.1.11 (2018-12-29)


#### Features

*   disable automount in live system ([1ac5d667](https://github.com/linuxdeepin/dde-file-manager/commit/1ac5d667b57cc67f9846467e7ae860306f694c26))
*   support set/preview screensaver ([edd7de5c](https://github.com/linuxdeepin/dde-file-manager/commit/edd7de5c449215d9699283112642528465652b40))
* **screensaver:**  support set lock screen at awake ([d955143f](https://github.com/linuxdeepin/dde-file-manager/commit/d955143fd0bb7209e26315e99e2e096a7b092288))

#### Bug Fixes

* **dde-wallpaper:**
  *  the svg cover image is blurry on HI-DPI ([433481da](https://github.com/linuxdeepin/dde-file-manager/commit/433481da481f491f03a60edefa5c31f8cde45b3c))
  *  change the screensaver start time list ([559c06dd](https://github.com/linuxdeepin/dde-file-manager/commit/559c06ddd87c126608e606b9a0f333608f54862f))
  *  set the DSegmentedControl button minimum width to 40px ([53ceff8d](https://github.com/linuxdeepin/dde-file-manager/commit/53ceff8df8a34cbc4af8b16fa5ea7e9df7453778))
  *  the delete button position is warong ([ad77c2d9](https://github.com/linuxdeepin/dde-file-manager/commit/ad77c2d904ced42847fb421d6a521117bac0dbc1))



<a name="4.7.1.10"></a>
## 4.7.1.10 (2018-12-21)


#### Bug Fixes

*   add a timeout for TagManagerDaemon/disposeClientData ([5c72e0b8](https://github.com/linuxdeepin/dde-file-manager/commit/5c72e0b8a2bcdfa0d9844d4665e547db66504e7f))
*   dark theme header view section border color ([1b7af3f7](https://github.com/linuxdeepin/dde-file-manager/commit/1b7af3f77ce536bce07948b65eaeb2718446ddfc))
*   OpenWidthDialog support touch scrolling ([a9415fb6](https://github.com/linuxdeepin/dde-file-manager/commit/a9415fb6866e8314811cc1ac77aa6cef02f8659e))
*   ComputerView item vertical align spacing ([ff5a8d9d](https://github.com/linuxdeepin/dde-file-manager/commit/ff5a8d9d88428b39d9b56da54e17dc1a8dc0de17))
*   Device icon not changed when switching theme ([3820e168](https://github.com/linuxdeepin/dde-file-manager/commit/3820e168e372d5c9d88ad5080314055af8b015da))
*   trash view wrong sort order ([4fceafaa](https://github.com/linuxdeepin/dde-file-manager/commit/4fceafaa1ceb543b81ee21f18c32ea56b83e2d71))
*   wrong item disappeared when remote device removed ([d14c17fd](https://github.com/linuxdeepin/dde-file-manager/commit/d14c17fd5802491d5a40fb2ec207b2de2ecf8bc1))



<a name="4.7.1.9"></a>
## 4.7.1.9 (2018-12-14)


#### Features

*   open with for multiple file selected ([250cc486](https://github.com/linuxdeepin/dde-file-manager/commit/250cc486bec256fa9c8831c1cb63c22b237d4f71))

#### Bug Fixes

*   file suffix keep letter-case ([f10e75c7](https://github.com/linuxdeepin/dde-file-manager/commit/f10e75c77a70989a4f3ebebf755edf67770066ad))
*   Dialog size not correct when using icon from theme ([520e4dee](https://github.com/linuxdeepin/dde-file-manager/commit/520e4deea668b2cac8b8cb206aed32f73b25b0a4))
*   display 'Disk info' in sidebar root item ([4f64df4f](https://github.com/linuxdeepin/dde-file-manager/commit/4f64df4f89a39d7b0ccd03e955918f64f18d52eb))
* **dock-plugin:**  the icon will not change after the system theme changes ([bd48579d](https://github.com/linuxdeepin/dde-file-manager/commit/bd48579d6edef6065c1f7ca8e25c49cbae296d19))



<a name="4.7.1.8"></a>
## 4.7.1.8 (2018-12-10)


#### Bug Fixes

*   remove focus border highlight ([42e9f8fd](https://github.com/linuxdeepin/dde-file-manager/commit/42e9f8fdd9e2f2926fda1af3b48cccbe10444cbe))



<a name="4.7.1.7"></a>
## 4.7.1.7 (2018-12-07)


#### Bug Fixes

*   QStorageInfo cannot get device on invalid symlink ([85d63953](https://github.com/linuxdeepin/dde-file-manager/commit/85d6395341e5b238b7b4216d21e692e63d91c722))
*   mount plugin for dock ver 1.1 ([5272d6a5](https://github.com/linuxdeepin/dde-file-manager/commit/5272d6a55117f065e2b82b1dc76802c09b5da686))
* **dde-desktop:**  relayout on the font size changed ([d1cc1313](https://github.com/linuxdeepin/dde-file-manager/commit/d1cc1313fee743cec7bf29e689ed1b1170c49d88))

#### Features

*   mac style headerview snapping ([9a661b07](https://github.com/linuxdeepin/dde-file-manager/commit/9a661b074752f57f14e9a43b6af2c51d25334ac8))
*   dav scheme basic support ([7870d32a](https://github.com/linuxdeepin/dde-file-manager/commit/7870d32af7525cd351042392e85e02c677b13afe))
*   ask for adding executable permission for flatpak file ([0ab1fb20](https://github.com/linuxdeepin/dde-file-manager/commit/0ab1fb20fbb5aaf8466872382d278f4664e0b4a1))
*   disk property for /home if available ([e0ff94a9](https://github.com/linuxdeepin/dde-file-manager/commit/e0ff94a941c592a0674a0d4a7b9adcdc824046cd))



<a name="4.7.1.6"></a>
## 4.7.1.6 (2018-12-03)


#### Features

*   ask for adding executable permission for AppImage file ([d7990128](https://github.com/linuxdeepin/dde-file-manager/commit/d7990128e6d38cbb352f229855ac0b437d22be11))
*   keep the suffix when the file name is truncated ([77e76229](https://github.com/linuxdeepin/dde-file-manager/commit/77e762291feac47a2ec748af52553fddf9860c9b))

#### Bug Fixes

*   call QAbstractItemModel::endRemoveRows() ([cf1b43dd](https://github.com/linuxdeepin/dde-file-manager/commit/cf1b43dd720b03a199d6187815cdc1ac2c1a344f))



<a name="4.7.1.5"></a>
## 4.7.1.5 (2018-11-30)


#### Bug Fixes

*   crash when remove file out of current view ([a2aa4ccd](https://github.com/linuxdeepin/dde-file-manager/commit/a2aa4ccde7da7a585831be07e944f4651b3e294d))
*   no longer remove device which should be ignored ([7675f3bc](https://github.com/linuxdeepin/dde-file-manager/commit/7675f3bc39b4850e32e35f209899869a745b868b))
*   no longer refresh device list when unmounting removable dev ([69db1f9c](https://github.com/linuxdeepin/dde-file-manager/commit/69db1f9cea01e2be91a1976268ab8fc7dc79c09e))
*   crash when insert a blank dvd disc ([0fe51d0b](https://github.com/linuxdeepin/dde-file-manager/commit/0fe51d0bc1e62ec99cb846c5145d2f282ec9d7e2))
* **text_preview:**  encoding support russian in cyrllic. ([b30e02e6](https://github.com/linuxdeepin/dde-file-manager/commit/b30e02e6541df97e9fd5eb9bb9adf3eee0e0d282))



<a name="4.7.1.4"></a>
## 4.7.1.4 (2018-11-27)


#### Features

*   filesystem type on propertydialog #41 ([197e57d4](https://github.com/linuxdeepin/dde-file-manager/commit/197e57d476ff6254169de046cf2c962c3539d920))

#### Bug Fixes

*   eject removable device from dock ([75e142e9](https://github.com/linuxdeepin/dde-file-manager/commit/75e142e9ea0482faa889dd0e680a8889b10eb5c4))
*   vendor support for context menu ([df26771a](https://github.com/linuxdeepin/dde-file-manager/commit/df26771ac03eb5d9cfc234bda37d43dae824be4d))
*   adjust item height when changing font size ([91e009e6](https://github.com/linuxdeepin/dde-file-manager/commit/91e009e6896a1a26499c9858841dff403b819053))
*   file template is not true-empty ([5b26d9ef](https://github.com/linuxdeepin/dde-file-manager/commit/5b26d9efedf50e576ebdc468e96b150ebbe3ae46))
*   no longer set itself as default filemanager ([63b044ff](https://github.com/linuxdeepin/dde-file-manager/commit/63b044ff390701a5a00ca69f1773799d4c6f4cfd))



<a name="4.7.1.3"></a>
## 4.7.1.3 (2018-11-23)


#### Features

*   selected text color ([cd284298](https://github.com/linuxdeepin/dde-file-manager/commit/cd284298af078179ea4f84e434744436d687520e))
*   support preview the gif image ([3751bf14](https://github.com/linuxdeepin/dde-file-manager/commit/3751bf14ec0390edf13ecde242948c422da44682))
*   GenericName and Vendor in .desktop file ([f92fe0e5](https://github.com/linuxdeepin/dde-file-manager/commit/f92fe0e5c3810132b7e8f955a615d4e01161b5a6))
*   Support for deepin vendor desktop file ([d2f93e5d](https://github.com/linuxdeepin/dde-file-manager/commit/d2f93e5dd12641ef67eba117ebc31a76a4efce1e))
*   display folder size as '-' ([a670ecec](https://github.com/linuxdeepin/dde-file-manager/commit/a670ececc95a1b433ea343914ae4c52cbde7e738))

#### Bug Fixes

*   for removable device, do eject instead of unmount ([1f8fe6ee](https://github.com/linuxdeepin/dde-file-manager/commit/1f8fe6ee3276a603139788bbdaafb66e51bbb6ea))
*   GenericName and vendor support ([4b58acba](https://github.com/linuxdeepin/dde-file-manager/commit/4b58acba3b8795bc9360c5145090bf359be3ebc9))
*   cannot mount afc. ([e6d7a570](https://github.com/linuxdeepin/dde-file-manager/commit/e6d7a570253695da1a7840a3321bd97553bdea7d))
*   can not set as default app in some case ([95a6d377](https://github.com/linuxdeepin/dde-file-manager/commit/95a6d377ef2572a4a4f4a1666b4d8c7e8eb0e1c0))
*   older qt version support ([a7e2eba2](https://github.com/linuxdeepin/dde-file-manager/commit/a7e2eba29cb99f2fa70be34802e3f1b7b379d491))
*   recent view flashing. ([33a2dd64](https://github.com/linuxdeepin/dde-file-manager/commit/33a2dd648f28701f738ed0bd85d43ac043f67760))
*   don't warning if ntfs mounted via kernel drv ([4b39c53e](https://github.com/linuxdeepin/dde-file-manager/commit/4b39c53e4055c8c9d03f05a2444a9f7235c99e6f))
* **dock-plugin:**  pixmap align ([777e8302](https://github.com/linuxdeepin/dde-file-manager/commit/777e8302e5525bf5d164d36adc1ac1a7105e7087))
* **fileutils:**  remove unused code. ([86742aac](https://github.com/linuxdeepin/dde-file-manager/commit/86742aac40e59ea8fa8a32e3480ab6ebffed830a))
* **recent:**
  *  search open location error. ([6a80e562](https://github.com/linuxdeepin/dde-file-manager/commit/6a80e5622c4ea7486a3e6d37d195c8c96806a8c6))
  *  use modified datetime. ([45a795b7](https://github.com/linuxdeepin/dde-file-manager/commit/45a795b7dc5070b92cb6da6cb5dcf1a5e6810ff1))
  *  use the correct last time. ([e3ec6545](https://github.com/linuxdeepin/dde-file-manager/commit/e3ec654579c960255701ea60a954cc8fdce09fb5))



<a name="4.7.1.1"></a>
## 4.7.1.1 (2018-11-12)


#### Bug Fixes

*   undefined symbol on archlinux ([34748039](https://github.com/linuxdeepin/dde-file-manager/commit/3474803988d5a143ff5910f936da2ec4e014c0a9))
*   xbel does not exist for listening failure. ([73763a4e](https://github.com/linuxdeepin/dde-file-manager/commit/73763a4eeb4d210f1f27027f370e479e446fd9da))
* **recent:**
  *  don't popup dialog. ([79686f09](https://github.com/linuxdeepin/dde-file-manager/commit/79686f09d304c15f410f5f006e141b49673b1a39))
  *  conflict with gtk recent manager. ([a68d84f7](https://github.com/linuxdeepin/dde-file-manager/commit/a68d84f765add370b6ffb82cd79fadc21160e3b1))



<a name="4.7.1"></a>
## 4.7.1 (2018-11-09)


#### Bug Fixes

*   Wrong selected item number ([11faeef0](https://github.com/linuxdeepin/dde-file-manager/commit/11faeef0c26e31e7781bad72d91247a2f771a520))
*   mount plugin ignore loop device ([e6992de3](https://github.com/linuxdeepin/dde-file-manager/commit/e6992de318f0a5aa050030e9af978b0b2cabaf8e))
*   lag when copy large file on desktop ([e637e230](https://github.com/linuxdeepin/dde-file-manager/commit/e637e2305ec44986acaaa16ccaa3922c8e19bd38))
*   multiple file DnD issue in some case ([bd4ef257](https://github.com/linuxdeepin/dde-file-manager/commit/bd4ef25754ac85cf5b0a612d4422cf3b79252dc4))
*   open with dialog missing recommend apps ([a6a132c4](https://github.com/linuxdeepin/dde-file-manager/commit/a6a132c42c2c5107d5b470af782099a0e0fbe75e))
*   delete files trash. ([3e10b5c9](https://github.com/linuxdeepin/dde-file-manager/commit/3e10b5c99679a26a110553a87c9e6e4bc68ab547))
*   missing header file ([df6cf62c](https://github.com/linuxdeepin/dde-file-manager/commit/df6cf62cf1e29db37d8a58b9b8768a227b33376b))
*   workaround for recent file support ([154d2528](https://github.com/linuxdeepin/dde-file-manager/commit/154d25283845986d0d307b4dc2d68dc380929d74))
*   disable set wallpaper for gif file ([b0f2569a](https://github.com/linuxdeepin/dde-file-manager/commit/b0f2569a78c6a8bb65fb114d257782bcc019687d))
*   wrong open with section when symlink no longer valid ([2409f208](https://github.com/linuxdeepin/dde-file-manager/commit/2409f208948d98dabd2effc994ee6b65e05eeacd))
*   root partition wrong file count in property dialog ([201b722c](https://github.com/linuxdeepin/dde-file-manager/commit/201b722cbf4e76e04901fd00bc22448de0e24da2))
*   subtitle empty floder. ([2f605bb0](https://github.com/linuxdeepin/dde-file-manager/commit/2f605bb0f2e9b5f68ead19a3c0b188f67c6d87e8))
*   goto home dir when hide recent entry ([3ad1e9cd](https://github.com/linuxdeepin/dde-file-manager/commit/3ad1e9cda6350a353d7c086bb239c2b7a0aeb8fc))
*   do not resize column if rowCount is 0 ([6de65107](https://github.com/linuxdeepin/dde-file-manager/commit/6de651076dd94ebc9ceed883f8b678e93f589641))
*   mount plugin i18n ([3b1623d3](https://github.com/linuxdeepin/dde-file-manager/commit/3b1623d3ebf6718916c0906fa60451ef5ab8d6ce))
*   tooltip hide right after mouse leave ([9500afc2](https://github.com/linuxdeepin/dde-file-manager/commit/9500afc23a491081c4d1a48ef53bf248b4b45591))
* **mountPlugin:**  unmount all will now unmount vfs device ([97541409](https://github.com/linuxdeepin/dde-file-manager/commit/9754140981ebecf8b83623ee76556d7b023d9d69))
* **pro:**  remove DnD limit for some desktop file ([d70f59c7](https://github.com/linuxdeepin/dde-file-manager/commit/d70f59c79615f89d35e6904f9bcdb6c5d93eb387))
* **recent:**
  *  support decompress file. ([36466703](https://github.com/linuxdeepin/dde-file-manager/commit/3646670377b808f6079fd5ec7cdb262083286ab7))
  *  support tag color display. ([cc4a8d11](https://github.com/linuxdeepin/dde-file-manager/commit/cc4a8d11a0fbf0b0cba0bd35e6af89a82718b41f))
  *  support listening delete files. ([5f3979a4](https://github.com/linuxdeepin/dde-file-manager/commit/5f3979a4d6a0335e059770700b740029468b107e))



<a name="4.6.10"></a>
## 4.6.10 (2018-11-01)


#### Features

*   support recent file settings are displayed. ([a5ea5541](https://github.com/linuxdeepin/dde-file-manager/commit/a5ea5541388eaf4b382c5aaf21669be537977825))
* **desktop:**  support keyboard chinese positioning. ([85e366f5](https://github.com/linuxdeepin/dde-file-manager/commit/85e366f5c3e5844df559b3ef355ea660247e5a37))
* **xbel:**  support for removing files from recent. ([1d728071](https://github.com/linuxdeepin/dde-file-manager/commit/1d7280710838d5d52edaab8a3fc0af730f8dfc3c))

#### Bug Fixes

*   remove warning of style sheet ([7e1b5782](https://github.com/linuxdeepin/dde-file-manager/commit/7e1b578251314f6ae3a65739b11fed59e4640ac6))
*   recent uses normal right menu. ([55265e1f](https://github.com/linuxdeepin/dde-file-manager/commit/55265e1f9e28d9492a166cc3ed89000df59e5886))
*   check only when dir got accessed ([a8603248](https://github.com/linuxdeepin/dde-file-manager/commit/a860324810af132f5f8462bb90a7d54fb091add5))
*   override the recent right menu. ([e00987e0](https://github.com/linuxdeepin/dde-file-manager/commit/e00987e03e207c59bb0a0cbe96f857bf07bb0f9d))
*   check smb dir permission with anonymous access ([a057e515](https://github.com/linuxdeepin/dde-file-manager/commit/a057e515b95b78dc3f356c551b1e0431de4b79ee))
*   allowed DnD when previous DnD copy not finished ([57c42479](https://github.com/linuxdeepin/dde-file-manager/commit/57c4247939774a99a2c19d6c90b6ea09904d58e5))
*   can not sort on recent view ([5bec3eba](https://github.com/linuxdeepin/dde-file-manager/commit/5bec3eba3655e34bc480750916d04a745c03c9f4))
*   file filtering cannot be selected. ([b372e1d7](https://github.com/linuxdeepin/dde-file-manager/commit/b372e1d7b9358086159f34cd1764cf00d4d8f72c))
*   check if file are in the same folder ([bea0d224](https://github.com/linuxdeepin/dde-file-manager/commit/bea0d224db441d50d535f371f0f4ea480a44a891))
*   invalid setting in usershare:/// ([6bc5423e](https://github.com/linuxdeepin/dde-file-manager/commit/6bc5423eb19c2386da66d41fc314ce3cf03e2233))
*   update device list for open file dialog ([d80d18ed](https://github.com/linuxdeepin/dde-file-manager/commit/d80d18ed21e57e549ecc1da3fcc82c00bb9f7c4d))
* **DFMSettings:**  no signal when a key first write to config file ([0bf9c69f](https://github.com/linuxdeepin/dde-file-manager/commit/0bf9c69f189977123cacf27dad445213b9e08dc8))
* **GitHub:**  issue should report to developer-center ([7ec1672d](https://github.com/linuxdeepin/dde-file-manager/commit/7ec1672d8ebcd7162657a8dddc31f42443759a80))
* **background:**  not update background pixmap when resolution changed ([fffcd811](https://github.com/linuxdeepin/dde-file-manager/commit/fffcd81188ddde127d44ff2e689046e4b4bb6afb))
* **icon_mode:**  rename editor display incomplete. ([b772bf19](https://github.com/linuxdeepin/dde-file-manager/commit/b772bf1955cc067dd581ae51fcd06c249260d19d))
* **recent:**
  *  new item does not show time. ([ea996306](https://github.com/linuxdeepin/dde-file-manager/commit/ea996306dbd05821b541d6981b3fb6ae092f931b))
  *  sort menu text does not display. ([3db6f293](https://github.com/linuxdeepin/dde-file-manager/commit/3db6f293b59ac91139b7203daf887096c04d185d))
* **sidebar:**  recent item does not display the property menu. ([7bba0def](https://github.com/linuxdeepin/dde-file-manager/commit/7bba0def1991f41d8267d45eadfb2d3a015be43a))
* **tabbar:**  no recent name is displayed. ([fa329e8a](https://github.com/linuxdeepin/dde-file-manager/commit/fa329e8ac3d392e892bf90c71652a50899116bee))



<a name="4.6.9"></a>
## 4.6.9 (2018-10-25)


#### Features

*   setting for toggle recent file entry in sidebar ([bc832825](https://github.com/linuxdeepin/dde-file-manager/commit/bc832825d8ae9862414a49d4f3ebea8dcc9489e7))
*   'open with' context menu for folder ([be91c1f1](https://github.com/linuxdeepin/dde-file-manager/commit/be91c1f1a7ae4ba783aa1ab5384b2ad67b8a7300))
*   do not limit the minimum height of the line edit on rename(icon mode) ([89237a2c](https://github.com/linuxdeepin/dde-file-manager/commit/89237a2c78af3477150b62b74e5cfeec28b26ca0))
*   auto resize column width when the column header view section handle double clicked ([45339f7d](https://github.com/linuxdeepin/dde-file-manager/commit/45339f7d26b0094fd1afee088698d8fbca839eea))
*   enable open-with option for debug build ([6dadfa75](https://github.com/linuxdeepin/dde-file-manager/commit/6dadfa75c599cba067a144ab719dcf16de598c83))
*   support use windows style display partition name ([543321c0](https://github.com/linuxdeepin/dde-file-manager/commit/543321c023d1eeff3ef866b2979c50810ffcecf5))
*   add two unidentified mimetype. ([cf603dc9](https://github.com/linuxdeepin/dde-file-manager/commit/cf603dc9f7c2d72b47648b4b38e0ffb875ed2b8e))
*   root mode supports HiDPI. ([130278cd](https://github.com/linuxdeepin/dde-file-manager/commit/130278cdf1c1f1bd80876fcc0c4534c7ae39b856))
* **ctxMenu:**
  *  Support template folder to create new file ([c36406c7](https://github.com/linuxdeepin/dde-file-manager/commit/c36406c76ed3358f74cdd8f8fe9449480c874408))
  *  Support template file to create new file ([285016c2](https://github.com/linuxdeepin/dde-file-manager/commit/285016c2814a8bcc89eb27dd692c0c869429ed0f))
* **mountplugin:**  add network device back ([0eda4d9c](https://github.com/linuxdeepin/dde-file-manager/commit/0eda4d9c48e38a85aa64f794efed3c9c4c281380))
* **propertyDlg:**  show fs idtype when debug enabled ([564b48a4](https://github.com/linuxdeepin/dde-file-manager/commit/564b48a4c37ed9008e63e48937ff6a7a73ee9427))
* **recent:**  monitor file support. ([374a2ed9](https://github.com/linuxdeepin/dde-file-manager/commit/374a2ed9f7d205b94d7dfe3612f4c81f83f53c69))
* **usb_formatter:**  support HiDPI. ([472cf954](https://github.com/linuxdeepin/dde-file-manager/commit/472cf954d3ebac03aa06d2b1eb316b29263aa1b9))

#### Bug Fixes

*   use qobject_cast instead of dynamic_cast ([be2f8ed4](https://github.com/linuxdeepin/dde-file-manager/commit/be2f8ed416719b924589a775a3e0e8b7d56c7028))
*   preview not working on dde-desktop ([88ce6879](https://github.com/linuxdeepin/dde-file-manager/commit/88ce6879abc8ec83a53ca632b2b2a01b4f0a99dd))
*   wrong context menu when switching views ([7193e4db](https://github.com/linuxdeepin/dde-file-manager/commit/7193e4dba20788152135e426b3f53becf6f27f26))
*   rename on property dialog for .desktop file ([f8037edf](https://github.com/linuxdeepin/dde-file-manager/commit/f8037edf79cc5da8b8ac9e0a115f9a0878695bd4))
*   i18n strings and typo ([a091ea8f](https://github.com/linuxdeepin/dde-file-manager/commit/a091ea8fa68a16475013c91ca7c242efc7bd0d7d))
*   avoid divide zero. ([ed0ed24e](https://github.com/linuxdeepin/dde-file-manager/commit/ed0ed24e9cee50439d9e5d3dcd024cb414b8e044))
*   Cannot select item in some case (#55 on gh) ([1543830e](https://github.com/linuxdeepin/dde-file-manager/commit/1543830e221eda26c0a8c1a744c5e3199983ee02))
*   always update vfs device info when changed ([68b4db18](https://github.com/linuxdeepin/dde-file-manager/commit/68b4db188175788688d1a9850c2fb3d76f42bbec))
*   file model event order not correct ([09dce730](https://github.com/linuxdeepin/dde-file-manager/commit/09dce730f6cca9770874440d6e98946dd0f2ff11))
*   keep the directory modified datetime when copy/move it ([89dcc877](https://github.com/linuxdeepin/dde-file-manager/commit/89dcc877101d1e25a5044f0bdc96bb24ff753a40))
*   crash when preview file ([a974e0b9](https://github.com/linuxdeepin/dde-file-manager/commit/a974e0b98751b77c1d1702e6078da6a3c8c30738))
*   replace gvfs-open with gio ([887f1a35](https://github.com/linuxdeepin/dde-file-manager/commit/887f1a35ef0a40aad90a8aeda0d2b3e4c1abd78b))
* **ComputerDialog:**  add computer name. ([f2714f9e](https://github.com/linuxdeepin/dde-file-manager/commit/f2714f9e92e26ee0d29ae329aa8231b7fdd8ec06))
* **DFileSystemModel:**  check idle when do refresh ([172de247](https://github.com/linuxdeepin/dde-file-manager/commit/172de247307806f93bc59d1b1f3c874d1f847e47))
* **desktop:**  avoid infinity EP query loop ([eddc6f2e](https://github.com/linuxdeepin/dde-file-manager/commit/eddc6f2e2981bd3bcdbc186f809ac2c2cdd9d181))
* **mountPlugin:**
  *  crash when vfsDevice create failed ([4e84a38b](https://github.com/linuxdeepin/dde-file-manager/commit/4e84a38bfb2534f02ea7e23f9775fcdd4b514f52))
  *  crash when icon ptr is null ([b43a4e28](https://github.com/linuxdeepin/dde-file-manager/commit/b43a4e2800ee883b4751e0e7ccf1f98f4be2cb8a))
* **statusBar:**  file size and number of files are incorrectly calculated ([3a63071a](https://github.com/linuxdeepin/dde-file-manager/commit/3a63071a796f1c9dc09e4f5cf1c2a88a40ba7cf3))
* **trash:**  not exist warning when copy file out ([cf5c3560](https://github.com/linuxdeepin/dde-file-manager/commit/cf5c3560a89494a36c8f6dbf339c2b1dba57f1fb))



<a name="4.6.8.4"></a>
## 4.6.8.4 (2018-09-13)


#### Bug Fixes

*   size too big of bmp image thumbnail ([b4b52f43](https://github.com/linuxdeepin/dde-file-manager/commit/b4b52f436936c61ba0893d7873f45c40ee541f3e))
*   can not monitor disk device added(UDisks2 <= 2.1.7) ([f8077abb](https://github.com/linuxdeepin/dde-file-manager/commit/f8077abbac77c138cce06ed261dcb5a494d964db))
* **DTaskDlg:**  disable pause btn on sw ([ffb6f3f8](https://github.com/linuxdeepin/dde-file-manager/commit/ffb6f3f850d20135693b31c7bb3cc70891dd2518))
* **desktop:**  background not align center ([2bf46b98](https://github.com/linuxdeepin/dde-file-manager/commit/2bf46b981ef499b53a5a34d2b212b6703debeb4a))
* **fileView:**  crash on slow device ([7d75ae65](https://github.com/linuxdeepin/dde-file-manager/commit/7d75ae65c105dc6e80eb1742fc229f4ed58f544b))
* **mime:**  use mimeType from fileinfo if is avfs file ([83dffe39](https://github.com/linuxdeepin/dde-file-manager/commit/83dffe39da2e1b45fa05cc879b3658142265886e))
* **wallpaper:**  wrong delete walllpaper widget ([386c7427](https://github.com/linuxdeepin/dde-file-manager/commit/386c742700271d7038237cf970f257c49cd6ca01))

#### Features

* **dde-desktop:**  support preload mode of dde-desktop ([d4061799](https://github.com/linuxdeepin/dde-file-manager/commit/d40617990242e7ad406aeb21f155e0a5199e8fec))



<a name="4.6.8.3"></a>
## 4.6.8.3 (2018-09-06)


#### Bug Fixes

*   can not build on loongson/sw ([e6938029](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/e6938029f646c50beda9e2caee82ead546e1f490))



<a name="4.6.8.2"></a>
## 4.6.8.2 (2018-09-06)


#### Features

*   use QGlobalStatic for some singleton ([c2cde2b4](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/c2cde2b4b8eef9254482388d9cf8a7009cea0f78))
* **desktop:**
  *  draw background ([b2e66431](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/b2e664310f29f3de055798a7840e8644a9142e76))
  *  use pure raster surfaces ([1532a522](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/1532a522899804838fd9146668a51e46d59d674f))



<a name="4.6.8.1"></a>
## 4.6.8.1 (2018-09-04)


#### Bug Fixes

* **fileinfo:**  check if fileinfo create failed ([2bb3b651](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/2bb3b651dd7054b680ad4c619e793109fcdbfc9f))
* **mountPlugin:**  internal impl of formatDiskSize ([936c90d9](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/936c90d916595b922f7f10aa657c67eb175eb46e))
* **preview:**  can't preview svg from search result ([6d9cc3c0](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/6d9cc3c0c51692cd5c69a473eb67fc5f791596a4))

#### Features

*   update depends ([3dc24fb8](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/3dc24fb81d94b6667ed37a0c1e469fb84c3fe5dc))



<a name="4.6.8"></a>
## 4.6.8 (2018-08-31)


#### Bug Fixes

* **computerView:**  disable new tab option if couldn't add new tab ([e4c61e2c](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/e4c61e2c1d7fcb96484f4dbc82c506cf4c7d18c3))
* **icon:**  remove mipsel special icon ([420bd78e](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/420bd78e52e5ae76a3fac07f0eed9224a9a07e5f))
* **mime:**  show msoffice mime file as Document ([8391d2dd](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/8391d2dd96f7b44cc3038a45ec409cfec24d10fc))
* **sidebar:**
  *  disable new tab option if couldn't add new tab ([686ba26d](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/686ba26def90e35ac1b4c928ed49ee33a3111618))
  *  fix dark theme phone icon missing ([69669fa9](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/69669fa9a724e9d318be2c40ef61bd1dd44101fe))

#### Features

* **computerView:**  Friendly file size (#35) ([3445c117](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/3445c11777eb450a1fdd89e9805ed7527149e5d7))



<a name="4.6.7"></a>
## 4.6.7 (2018-08-28)


#### Features

*   support only ReadWrite mode for DGIOFileDevice ([a36d5161](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/a36d51613cae7b3befe50a02bb8201c659b27c72))

#### Bug Fixes

*   the platform plugin version is wrong of judgment ([affb8732](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/affb8732e8c9337a4b45d94e713a2ec22349f68d))
* **config:**  default config file for pro support not correct ([b40f2ce2](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/b40f2ce21eb9e55c89ba283cfda75fdacb0a1b9d))
* **fileOp:**  use g_file_replace() ([5a8847a0](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/5a8847a0c7fe1f964a9c2dfa4739de017afdb52a))
* **permission:**  add executable flag for folder ([dc5b1d72](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/dc5b1d729fb63cb297175fb25e2d9d793c553a1a))
* **propertyDlg:**  no longer show openWith and auth section for trash file ([b830fe2f](https://github.com/linuxdeepin/dde-file-manager/tree/master/commit/b830fe2ff3cf4435970cb3a06f846997afb9bd30))



<a name="4.6.6"></a>
## 4.6.6 (2018-08-23)


#### Features

*   allowed disable quick search ([1cf9c53f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1cf9c53fad25ccdf52bc70d8da274a0003b22819))
* **mount:**  now support ask-question signal ([cd54c28e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/cd54c28e4f7449235493d46acda16048af5e7447))

#### Bug Fixes

*   DnD on sidebar ([b28b4ee7](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b28b4ee78dfc9c4235ea6cdf84ab5efd341580f5))
*   can not write file to sftp mount path ([ba9d0a92](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ba9d0a92767b450b8809540056c87acab3b74700))
* **DTaskDialog:**  the process bar label is not at center ([10601287](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/10601287bf268989e55bee0f8511093469d46fa4))
* **Image Preview:**  the preview image on the high-DPI screen is stretched ([a3b73c0d](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a3b73c0d5e824d3e11f74466b01b8c9c250c832a))
* **authDlg:**  checkbox set text ([054806d7](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/054806d70c6c9edacec36224656f25d46e6d07b3))
* **bookmark:**  Auto mount for ftp and sftp scheme ([65c51d0f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/65c51d0fdd33fb963db68211d2332ee3ac3e2d52))
* **bookmarm:**  always display remove bookmark option in context menu ([9192aecc](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/9192aeccc99910e65def8d10bb22d006f331884c))
* **fileDlg:**  press enter on save dialog ([e1587089](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e158708954b41d63a44ec75a2c12c5f2e836b1ec))
* **i10n:**  comment for partition label i10n ([22d90335](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/22d903350007cdafd4b612ecfbd4f36006facc99))
* **mount:**  ask question dialog now got a max width ([5709edf8](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/5709edf8e92e91f86ff569c96675e3e418d028f0))
* **mountPlugin:**
  *  Do not show partition if hintIgnore is true ([da70b6ed](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/da70b6ed0bb9031b4d1f0cfa29386a0b76b1747a))
  *  default config in resource file ([684eea1c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/684eea1c02ebd655db06e852761123e51d05bf30))
* **previewDlg:**  reset counter when start ([1c681c81](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1c681c81190ec32662dc8b3b9832b78eb58f747a))
* **propertyDlg:**
  *  Single file symlink-chain ([82418bf4](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/82418bf464c695ea5a75d8b2afe55d3d7f65b721))
  *  File display name and SymLink size ([27f1d13b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/27f1d13be20278439a2ebc319fd9dfa80d4c4120))
* **share:**  Use checkbox's setText() ([9d7e6ad5](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/9d7e6ad5bc982eff24a96196e56d805ad3b31424))
* **shareDlg:**  use checkbox setText() ([f3455233](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f3455233a57de578cb0eed02faa7ca3b4fe39787))
* **sidebar:**
  *  DFMSplitter use QGuiApplication::setOverrideCursor ([bdb4efe6](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/bdb4efe603b10ca9dd48f36c389b7b2996c9114d))
  *  splitter cursor fix ([668d2346](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/668d234625e0a46780d08fc8f20dd572a6c097b1))
  *  remove limit for i10n partition label ([4d356caf](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4d356caf3158325da71ffd6b26e6358603f310b6))
* **tagedit:**  disable mouse move event. ([103f59e8](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/103f59e86291932ea4c27b3ae73af7b822730005))
* **trash:**  item count not correct ([3009fc6b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/3009fc6b5cb472800af3530fca0405cbc2cfafbe))



<a name="4.6.5"></a>
## 4.6.5 (2018-08-12)


#### Bug Fixes

*   crash at ErrorHandle::timerEvent, because the pasteFilesV2 function is exit ([4780ecd6](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4780ecd615203b4e6512ed2fa5da6147328e535a))
*   the search result is incomplete on quick search ([c147323b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c147323bb4292d5d52d67f8818bd639f6ddd7008))
* **bookmark:**  Bookmark rename fix ([9416dd71](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/9416dd71eea834d3a5c9fb59755e9f2ea0648fbe))
* **desktop:**  set desktop item active ([37c127c3](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/37c127c33e338a45732f8ad847e4b0a7ed9514cb))
* **mime:**  support another appimage mime type ([6f72016f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/6f72016fc1f437ceff83617509af295aa68a3cf7))
* **preview:**  preview dlg always at center ([78a5a6a2](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/78a5a6a26abfaa660a8a9e247ffeb83441e6acb0))
* **share:**  Now can add bookmark ([e1ae9a8b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e1ae9a8b832029e10cb2cbfb992c616cc5314453))
* **tab:**  do not exec lambda if tab no longer valid ([a499cdf7](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a499cdf78341420ddc5f63afb73f840709993b03))
* **trash:**  Press delete key will ask for comfirm ([77aa2352](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/77aa23525a8feb1c677ce9fe45c922773f8cbf63))
* **view:**  fast switching view may lead crash ([64069332](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/640693327a0585da4f7691e9c48b1e00571014e2))

#### Features

*   add the "TargetIsSelfError" error type(DFileCopyMoveJob) ([dadaeb85](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/dadaeb850b7aa35ae548f31c2d9cef5939e76900))
* **computerView:**  hide internal disk header if no item avaliable ([2419cce3](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2419cce3b50333582b24764363ffe7eef107c6cd))



<a name="4.6.4"></a>
## 4.6.4 (2018-08-08)


#### Features

* **disk:**  disk i10n by label name ([f0a91db2](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f0a91db2df1a989b564b49cc740cb1b5217eaebc))

#### Bug Fixes

*   can not build on gcc 5.4.0 ([848db899](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/848db899607c6fe2ece9c0c95c36a71ee290ea8a))
* **completer:**  hide completer after press enter ([31417988](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/31417988b2ba82f7b9d39d8856c75e4719965250))
* **daemon:**  dquicksearch memory leak fix ([cf0598d4](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/cf0598d4c1bb3f333371380965cc3e7b3e95de4f))
* **mount:**
  *  Use udisk interface to calc partition size ([4fb42b9b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4fb42b9bfff2f387758ef0960bf6a55f1a89a7f9))
  *  don't mount device if it should be ignore ([e027a912](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e027a9124c31e220e399238c50bf30264c472317))
* **quicksearch:**  memory leak caused by missing regfree call ([42be8f8e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/42be8f8e02737e6fa7a1d20569b08447cbabd441))
* **trash:**
  *  the icon from theme ([c00068a4](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c00068a4ceb1c7d2f490aa0ca35db662b89e2abb))
  *  dialog file count ([64e55057](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/64e55057996c2a4361164b452d1bd52625b1423f))



<a name="4.6.3"></a>
## 4.6.3 (2018-08-06)


#### Bug Fixes

*   can not copy file by drag/drop(the 'Ctrl' key is pressed) ([b970fbae](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b970fbae551536edd124c122bb1e4619fccc3c0c))
*   ignore the cut operation of the same folder ([7ea543bd](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/7ea543bd74c8b0b72bd9eb80b93bbac764f6d6cb))
*   the type of error is wrong when copy a file ([45f4f122](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/45f4f122b7d8922d64e4dc1ddf0207ca09712808))
*   the remaining time of the file copy is error ([84118a28](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/84118a284f10c65fcec5d5d5d6ded266884cbf64))
*   invalid sort by file type ([8893a90e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/8893a90e17c685472d9c949dad56e0a01af47613))
*   use QByteArray::difference_type to replace int. if not will cause compiler error in clang. ([4d822e11](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4d822e11d964598f26237a6d224d74129c13efee))
*   if search failure, should not close quick-search. ([4afbab31](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4afbab31ba5206d9295060705cb409b9f72a5615))
*   creating a new archive, it shown in a wrong position. ([30713dd9](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/30713dd927e14634914130123c56e07342d6ee76))
*   crash when call DAbstractFileInfo::fileIcon on copy/move files ([8e973d90](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/8e973d90cd682ce7ebb686ca68bd199dbc34de50))
*   not update error message of the DTaskDialog ([31535edc](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/31535edce6c62c459769ae4b889bc7127e1f8896))
*   copy files from nautilus to dde-file-manager cause dde-file-manager segment fault. ([e4b3144a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e4b3144aaab3c18baa19bd606867259e33ddd3eb))
*   compact (\') in sql statement. ([fe3ef80b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/fe3ef80bc7eaebc6d546ef8c0b1b17a828613b9e))
*   crash when copy a large number of image file ([9573fe2b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/9573fe2b923711ea1efbe19ca4123c8c88f3b6d5))
* **bookmark:**  save mount point state ([d6705ac6](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/d6705ac6d6cc06717f63971c054efa6faf713f80))
* **desktop:**  add aosGlobal config file to watcher ([02d4273d](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/02d4273d9c732ab4348b6cd1d5ae8d04edb90760))
* **device:**  return the correct mount point ([8c9612d1](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/8c9612d1ea9e24d50e16e0c9781cc22b98d49440))
* **i10n:**
  *  translation fix ([48b3bec9](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/48b3bec97899262e713bf5e2bdbb539e77850b7b))
  *  Translation fix ([974a6aea](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/974a6aeafe8ca74a53b9d93394fb50b83624ba8f))
* **license:**  Option to disable ffmpeg, #31 on gh ([b52b8dc9](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b52b8dc97d9e64053f305233b2df462ad79f90b3))
* **mount:**  only show items mounted under '/media/' ([319aebeb](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/319aebeb2b053730a8e1f0e9d4a3ccd35fce6626))
* **sidebar:**  Cache data->urls() to avoid data expired ([775aca85](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/775aca8528b2f124387ae0b53f7328aae0eea085))
* **trashPlugin:**  show warning dialog ([ce636146](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ce636146877b7a2eb9f14d98b7b66933dd8d247e))

#### Features

*   support 'Alt' Key and mouse wheel button to scroll view(Horizontal) ([4bba89b3](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4bba89b3e1c68c6f51ebcfaa6d084849d7387e3e))
*   set the icon color scheme from the window text color when get a file icon from theme ([d2327505](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/d2327505536d488da38585e081be48dae1d4f2d0))
*   support wildcard in quick-search. ([da524977](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/da524977e3e6321d9e558c9c63671f215030c2d2))
* **bookmark:**
  *  auto mount partition for non-local bookmark ([d229d0b0](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/d229d0b0c8d7b88e1709ee736d22681a86741ee4))
  *  save mount locate url ([df6405b1](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/df6405b1f30fe895e908a373e7ac7d05db480ecb))
* **crumb:**  new crumb icons ([884ab321](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/884ab321a4954c3832386460dafbf0dd907b43db))
* **mount:**  use dde-file-manager with mount scheme if needed ([4bd80a5f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4bd80a5fdf056ac8eff8072f7b7e1ca37764d7b4))



<a name="4.6.2"></a>
## 4.6.2 (2018-07-27)


#### Bug Fixes

*   build failed on Qt 5.6.1 ([2f087eec](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2f087eecbb9e2c65f6a8f5108602050d6c22820a))
* **propertyDlg:**
  *  fix crash on readLine() call ([50791e7e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/50791e7e56d00530512f198d8478e443a19e2cbe))
  *  memory from /proc/meminfo ([7d962cef](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/7d962cef16f38a703b80523525c88bb9919677ac))

#### Features

*   support get a file thumbnail by a process ([65fd32b2](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/65fd32b2de821db2fcbb80ae5877c4524e7c1322))



<a name="4.6.0"></a>
## 4.6.0 (2018-07-26)


#### Bug Fixes

*   deepin-anything-server can not load deepin-anthing-monitor.so successfully. ([a794c3f3](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a794c3f393ee5717eae6853e61f08b00ff5ed8b3))
*   the searched directory property dialog size info is error ([04865f6b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/04865f6bf241b781504078cd784b87b13974fd72))
*   crash in DStorageInfo::setPath ([8d18efea](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/8d18efea96e45b1963dfb0355921dac9822ccad1))
*   can not auto start the com.deepin.filemanager.filedialog DBus service ([04e865a1](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/04e865a10170af7c7cb81911767192b2ef29c081))
*   can not rename file when click a selected file ([8789327d](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/8789327d967bfbc4c0aeeb0ea3e77fa1a31f4098))
*   the trash is emptry state is error ([e806bbef](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e806bbef920365e4750b38f5e311285f5b4c8d78))
*   can not get the system disk size ([280e3d43](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/280e3d4322a56d8541e1642d4dac12fbb5d76747))
*   can not update .lft(quick-search) timely. ([061a17a4](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/061a17a4f1dc01eb8af8469049516bb0d9de2845))
*   crash on preview the video files ([2db2fdd4](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2db2fdd4f17788d6f1217cc47089c97ab1bdd667))
*   crash in RequestEP::processEPChanged ([27c650a5](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/27c650a54994a3ae02c1de65e887f73b27789979))
*   crash in DFileSystemModel::index ([3dd0a386](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/3dd0a38659321dde5fa888636563f69318aa5aea))
*   skip the not-exists files when deleting files ([acaddc1b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/acaddc1bc745c65b272ca9f4fa9df227ead9a801))
*   can not copy file to mtp device ([c3939d3a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c3939d3aec68b3155fb4725ec32ea2369b1ceff6))
*   No progress of move the samba files(from one samba to another samba) ([f6500352](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f65003521b17b27059897e889cd376c1df54df9b))
*   load the translates failed on the settings dialog ([b260e486](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b260e486f632837c1961ca6abaa92590197c7302))
*   dde-desktop can not be show, if it was shown first time. And you click it, then it shown by itselft. ([5001cafb](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/5001cafbc0493114a0eef33dcd8ad654dec9dda9))
*   force deletion of files in the trash ([de479229](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/de479229931db7023c933af4ee9f63df1af1428f))
*   set the start position of the progress bar to 0 o'clock(DTaskDialog) ([0b903951](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/0b903951a85f86e08bc3ec902b1805c8aad180aa))
*   can not create the mkv video thumbnail ([821061f2](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/821061f20b7436614ae3b6ebb1fbcd7ff19f99d6))
*   the ghost signal of DAbstractFileWatcher is invalid ([8074ff62](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/8074ff62604ff4ef4a3bd64847564fa70b4b85a5))
*   crash in DFileSystemModel::remove ([83a62b1e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/83a62b1ee4176a5cbfd1b05e70a47782ab67e91e))
*   task dialog is not popup when emptying the recycle bin ([79bb736b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/79bb736b67b37513338dfec54097897dbbe69a42))
*   crash on destory DFMSortInodeDirIterator ([307d0018](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/307d0018f11ebf0ef051e72e457796477915dadd))
*   fix do not trigger create file thumbnail when scroll file view ([7de960b6](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/7de960b6c371572fdc27032c75d0fec1c5e3d0bf))
*   some network device may not shown in network share ([6115244d](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/6115244d50d881f8f6e36efe382dfb737c0595c6))
*   crash on destory DAbstractFileInfoPrivate ([fa73efb2](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/fa73efb2b33f14641af85a9f24e68b2fbc3bb1f7))
*   not jump to home path when unmount samba device ([b70e1cb1](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b70e1cb15b0accea4cdd65e5f10aa578f243bee1))
*   inactive label url does not jump after the current url is deleted ([cf054a02](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/cf054a02af71b98bb3820078b78c585f08e0e29a))
*   two windows appear when you open the file manager and use root mode ([e58746a7](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e58746a71c914071caaae455537109c4e7509d3e))
*   disable search in "network:///" ([117203df](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/117203df7d0c43ddc0d708952da9d9a76f8d9042))
*   config file load issue ([5bb16fc8](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/5bb16fc811c21c444e213f28f394eef9ad48fd10))
*   inefficent use of cpu and segment faults in quick-search. ([4601771c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4601771cad137d29447f3f21c17fa41297a1a8d5))
*   the DFileCopyMoveJob class progress value more than 1.0 ([aa816037](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/aa816037db7b65c18840d3db022622721ffeb036))
*   did not jump out of the while loop ([dc969474](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/dc969474a43634e6596e7bb84f163ade0b0c5a21))
*   when search something by quick-search, quick-search will segment fault. ([6a3942d6](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/6a3942d6f7ded078095c0152c40847a9814deda3))
*   can not watch signals after closing or opening text/video/doc preview. ([9b1f5bb0](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/9b1f5bb09523c750932639410d325bb7492915af))
*   click a file which is editting, it's icon will disappear. ([4e10dda1](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4e10dda1dfb8985a524fa7eb11b6c676d8df4330))
*   can not drop files to a selected directory ([9d7dd038](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/9d7dd0384f1fa6d5c648fcdd6e87a962f6f28c27))
*   no notify when bookmark item is removed ([a5d494f7](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a5d494f7d04d317e2298323cda4f2639b05cca9b))
*   crash on reanme file in list view mode by "F2" key ([374bd1e9](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/374bd1e92acb7b3f2519a11e276cfbdcd7b8170e))
*   wrong parsing of line breaks when generating text thumbnails ([ccae4466](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ccae4466aeb7a6bc7f8959f856982fe65a2fa60d))
*   Use udisks2 to send patition rename signal ([49255cff](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/49255cffddd0d47a93090ac281aaef2f3d032f52))
*   remove unecessary function call ([cc4b7263](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/cc4b7263ba8dd4b6a89abcc9deb9719d1cf4b89e))
*   sidebar resize issue ([b5215847](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b5215847943e4944b1d7b5c22fca89eaf2968c8d))
*   change pinyin '叶' to the used most often one ([1ac7cfb3](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1ac7cfb353335181a17d18a221c01f9ce5cd9c34))
*   fix create thumbnails failed when images file changed suffix name ([61c14d93](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/61c14d93ef20db4f066522786e1d5e014888e7dc))
*   can not preview it, if the images file changed suffix name ([62fe8d92](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/62fe8d92767aba3440e1447494c3dd81f1a131b2))
*   can not drag files to trash on dde-dock ([6889d4c2](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/6889d4c2efb1c3d1b039e355f65780d9868fdae2))
*   can not communicate with DQuickSearchDaemon and fix DQuickSearchFilter singleton error. ([e7c3a611](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e7c3a611c14b59f2b604c79846882c68cc51ce5b))
*   clean dirty name filters when file mode change ([e7019b13](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e7019b13c26457bb9bdabfa80c3258a6004a6bcd))
*   make deepin-anything-server-plugins light-weight, compile the dependence into binary. ([299e3ce8](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/299e3ce8d57d6322569e6628dd68c2b1e80334fa))
*   can not set property "hideOnAccept" of dbus file chooser dialog ([63fabde8](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/63fabde81aeb75d9f4deb4d2d4260cd4d6dc4f98))
*   drop action is wrong on drag move ([b6df179f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b6df179fd46f791a2f60039f650f690f8e554a6d))
*   disable drag/drop on file chooser dialog ([b8897001](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b8897001369533942ddbc7ffa87a5ffebf3f6cd6))
*   file name length exceeds limit ([3bee6f75](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/3bee6f75c81c863a3133ef0f81006805192f966e))
*   disable auto remove item when drop of MoveAction finished ([b84a80eb](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b84a80ebfac8ed30c92ea53543a930ec79dc204a))
*   file list mode header view sort indicator state is wrong ([a7206eac](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a7206eace9b717f4dcf40fd0ab57cd301e87652f))
*   optimize pixmap get of file icon ([c19a39dd](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c19a39ddc3534e7c269644407c6fc0cab31566c5))
*   hide the unmount button if the "canUnmount" property is false of the device ([2ebb7d44](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2ebb7d44193c201a87433dec18cc66c596b4d780))
*   the "GA_DisableNonRemovableDeviceUnmount" setting is invalid ([617ea39d](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/617ea39d4dcd04344da9dca06bd88e4181255a6d))
*   do nothing if the new file name is empty ([f3fc70ac](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f3fc70ac610e7dd13c3f3a1103b533e7d02d6d2f))
*   the saved window size is wrong on window close if the window state is maximized ([944b5d00](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/944b5d0020885d88d0fb95f5952240f2596ccb5a))
*   a wrong file info if the file url is redirected when set root url on DFileView ([3faa490e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/3faa490e131669dc5bb64d8741d90b2fb7be8c1f))
*   not refresh when preview files settings changed ([8dda41fe](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/8dda41feb049a4c4d6139ba5412e6dc592aaf366))
*   set the default view mode invalid ([0e15fe20](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/0e15fe20720ea4ac4cd44fb2800aecb75b753533))
*   null pointer exception ([9843ac16](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/9843ac1641e7a277191d6fcb28550a68faa4ed04))
*   failed on build if the cxx flags contains "-Werror=format-security" ([70b936f6](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/70b936f660706e0bdc6eb99f3163bad237526364))
*   the file chooser dialog theme not is "light" ([da1d9c10](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/da1d9c10fc461b92f8f4b89a789c8afb610325ce))
*   crash in DFileDialog:: selectedUrls ([e35190b1](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e35190b19fc7e0ab0161677761a68d95ee45b404))
*   the default file view mode is wrong(IconView is 1, not 0) ([17b017cf](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/17b017cf4fc5ca61cf4830b37d9939eb54b24385))
*   crash in dde-dock disk plugin ([a53d44dc](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a53d44dc3f88ba7a5fb1c1f22367356fc464f58d))
*   always open in a new window when opening a url by command line ([d901d050](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/d901d050ca9f19657def4de228f490363aa223ea))
*   cpu occupancy rate 100% after setting values in the settings dialog ([cdf80742](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/cdf807421746a150451ae2a39f13efad3ebcfc74))
*   Do not popup error dialog when auto mount ([61843845](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/618438456ac40d4d7ae36c7d5983bbe2450f4ce9))
*   type error with use clang compilation ([6d030855](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/6d03085555ae6396da024b32bf6c09d87622a7b8))
*   now you can choose to hide the suffix of file, when you are renaming a file. ([00bd8397](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/00bd8397ae40698a73429aa9e1c235c77aa03784))
*   the text background is malposition on icon file view ([6a7b3ab8](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/6a7b3ab8c3381e0138be9ae6774ad86e7978db6c))
*   Change tag color backend api changed ([cf044f94](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/cf044f947bec5d2edf6fda3c52b1dbebbc697340))
*   Sidebar bookmark reorder saving issue. ([b993ff2e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b993ff2e0dcb7d42f0d797d8fdca21ef56eb3f82))
*   Bookmark can't rename issue ([1c611da6](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1c611da62913253c4eb32ed99268a045babf0c35))
*   force push conflict resolving issue ([4ee8a20a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4ee8a20a2439b48818f35a62a0e85917230891ce))
*   showPropertyDialog cause file manager crash ([01d73d92](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/01d73d9239225f7627379db422249dafce23ebb3))
*   Size constraint when remove item from sidebar ([f7efa533](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f7efa5335aed92d1d2e7a687617a7f41260bae7d))
* **CVI:**  don't show context menu when read-only ([210df2fa](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/210df2fa63b5cae935a01600756ea74d982a1f2e))
* **addressbar:**
  *  Revert completer to case sensitive ([c6de8732](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c6de87324c8ec1e7d0295c91327e778abc0cedfb))
  *  weird lost focus issue ([733ae20e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/733ae20e0e23ebe62d53c3ee90fba337a7948756))
  *  reset completer model with history flag ([cedce824](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/cedce824beeb23e745f8224f4d0de4b0c4c11624))
  *  Tab to complete selected item ([2392a58f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2392a58f9ac84a12e8eeabbac2776e5993c80853))
  *  Press tab on cant't complete url ([7d13eb69](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/7d13eb696faa8201f39d589302a162700c28d81a))
  *  context menu casue addressbar hide. ([252476fc](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/252476fc1fff70ab8da64af61868d28fdd8152db))
  *  completion logic changed. ([7e94979e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/7e94979e10a874e047b98a74f7d655008af6ce42))
  *  auto completion deleting key issue. ([d00c6596](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/d00c6596220ee84a5ade35d41e8a99523b932d30))
* **authDlg:**  no longer use fixed dialog size ([1c3d329e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1c3d329e4413d779781cd96088dfe5afc9af6cb4))
* **batchRename:**  suffix not correct ([969e0e59](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/969e0e5931251abb6edd85c489dea8e919e9d12b))
* **bookmark:**
  *  now can't add dup tag bookmark ([465c65c4](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/465c65c436d2a01cd4473915d5b09ae99ab79a8e))
  *  create file info if url is bookmark root url ([64779cd1](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/64779cd1c259627940de4db6bb2804db76e455fc))
  *  now can entry the dir which was added as bookmark in leftsidebar. ([d0be85d7](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/d0be85d718de02e307fa159711368a6ddff52dda))
* **char-detector:**  replace uchardet to KCodecs for prober the charset of files. ([e8bd4a23](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e8bd4a23647eee871ad1caf8c10c465be2dbd6e7))
* **cleanup:**  tidy code ([0557586f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/0557586f47056b83beed0c6fe376a072a5e0d174))
* **completer:**
  *  Set base string before set complete prefix ([c3994582](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c3994582814b78df3fcef255ffb6eee54a965081))
  *  detect prefix fix ([c348cafd](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c348cafd82241e88b10711db1560fb149d9d8186))
  *  reset complete model when switch complete type ([7d6d8004](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/7d6d80048d1ae2e3b9c698f5c292edea498d22d2))
  *  Clear completeModel when hide addressbar ([c072ff6d](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c072ff6dbe83ed0c02269952d168453216bd2a10))
  *  Press enter to start changedirectory directly ([42124198](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/42124198e83284f8e68112ae534f171142866131))
* **contextmenu:**  remove duplicate item ([71c1631c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/71c1631c77da48e481abd256a36372bca0b43a4a))
* **crumb:**
  *  arrow not hide when toggle addressbar ([e8a027b3](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e8a027b3704e022e6af95880db7b93ac51d3686e))
  *  Go to searchTargetUrl when exit search. ([2723d663](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2723d6634d5ba6afa25d79e7c958a2495bacd197))
  *  Unexpected back action while searching. ([7d8c5ba2](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/7d8c5ba2203cdbd9e288c340a2829578268365fc))
  *  Click clear btn to exit address editing ([408cf81c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/408cf81c0ea169555d433c33011faea7bbf92462))
  *  backspace cdup wont update crumbbar. ([3e2daf7f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/3e2daf7f6eaa0ba3458c02292390aadba7b3f7cd))
  *  smb scheme url display issue. ([12a6b1e8](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/12a6b1e8b19bf5aaf7ac79e1e186ab41e9efa9c6))
  *  Crumb item display name and order. ([f181e1c9](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f181e1c99d665e7eb6ab08ef4fbbee061fcb83ae))
* **crumbbar:**  right click no context menu ([9ea8eae1](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/9ea8eae17f8d8e5d238f1b0da98c44f8ee0a64d7))
* **dde-anythingmonitor:**  add "plugin" for qmake CONFIG ([58e814d2](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/58e814d2100a3bb71f2f389f6c022c5ec0d66228))
* **dde-desktop:**
  *  can not remove the file by "delete" key ([e3281ce6](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e3281ce694ea9d074e9d1c57b575fd7cf0192b3c))
  *  crash when start the application ([878cd0d0](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/878cd0d0e30b9a92160b4464017c863aea0f38a7))
* **dde-file-manager-daemon:**  the cache location is "/.cache" ([04e4888e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/04e4888ecb982f535e9a2b173bbdc6dde4ace9ff))
* **desktop:**  check if is a symbol link ([fb0b2e35](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/fb0b2e358c0e5fab2b5c21cc67ec9f11d488bd2f))
* **deviceFormatter:**  Center align titlebar text ([0a09c078](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/0a09c078aad36867659aa9336362243f86859859))
* **fileView:**
  *  header not shown when dragging scrollbar ([d69525a1](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/d69525a163fe5f7dd8b918213b234d5f318d1c9c))
  *  Default size, save sort order by click ([77beba0c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/77beba0cfec27ea5cad3755a15556eef82ae75b9))
* **filedialog:**  update indicator icon when show ([d0fed062](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/d0fed06248a72d48357e1ae85b7d694b3a5680cb))
* **hint:**  Hint in shortcut list dialog. ([ad71d4f4](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ad71d4f4939ccf79a3a17ab54da32282a8ab9042))
* **i18n:**  text update ([0a7dfbc3](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/0a7dfbc396dfaeb792cc8e2a4ac2f2027b63f00b))
* **mountDlg:**  password text too small ([adb9302a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/adb9302aa6c2129def9a905d155f1a39e7ac5c42))
* **mountPlugin:**
  *  media-optical icon ([dd574640](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/dd574640be05033ec004d921a316083bf0378a0e))
  *  update device list after mount ([be5e6329](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/be5e6329773bd0217c42c5f040a85d5882c0a8fb))
  *  Always reload config when needed ([38c01033](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/38c01033530aa98496d8f77c27dd5ce09276931c))
* **network:**  can't open ftp and sftp scheme ([2941f4a5](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2941f4a5fdda41bb6d1d4051ba4bb19adc7d650c))
* **patitionRename:**  disable rename if patition not unmounted ([454cae3c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/454cae3c8fc5502210a08087de5b62517680352b))
* **property:**  Correct property menu for trash url ([e2ca9333](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e2ca93337339ca5546ec03881d9dd21a761872fb))
* **propertyDlg:**
  *  check owner than writable ([67ed3633](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/67ed36337091f6c05da18915384844a82f961bf7))
  *  Open with section url not correct ([34383446](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/34383446a2407adc6a5553906ffaf4159a22fba5))
* **properyDlg:**  file name label with elided text ([e1b89fec](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e1b89fec48f83a4a81171043764c437b78e8f495))
* **search:**
  *  disable network scheme search outside network:// ([95c3af1a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/95c3af1ab7913178701feac41a59ed616addfd08))
  *  Search in search url. ([7e02eabf](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/7e02eabf2745bcc7f2e8c1cc0ddfabe370c352ef))
  *  Now search support network scheme ([30a16d38](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/30a16d384cf53349fe0158e875758a14a7d6ed61))
* **sidebar:**
  *  don't show ctx menu at fileDialog ([4bcc4d79](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4bcc4d7945a601d6b5ff26d9bfd474f9c4fc267d))
  *  remember size on hide ([5b59b732](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/5b59b732c312d1ec9a334facbb55054873285179))
  *  check content widget visiable ([28242abd](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/28242abd332bc9a3455c0f0a250aa6b8f7cd986c))
  *  correct properties menu for devices ([97c007d8](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/97c007d8d5c168e164e7308f9bbcfb61f3c9126d))
  *  Save tag order when create. ([0fb6fd2f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/0fb6fd2ff13b6a993d3f8c7d262419e77bf0aa9b))
  *  spliter line visiable fix. ([a430dfa4](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a430dfa46a905a19a4ec6cbec4f49a3822d31f76))
  *  smb scheme missing context menu. ([3daf5401](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/3daf54018172dbd306cecfa414dd5b7390d3bf8a))
  *  bookmark url cd to target location ([2947f3ca](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2947f3cae0b5ce4f6f66ab8e2c89ac532fd81a03))
  *  select all when start rename. ([956c601f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/956c601f08ebc6fc47b8d7d4f3897fab19933273))
  *  Rename edit styling. ([1abfc446](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1abfc4464f73b362508b6df78d2bf485e9acff8f))
  *  cant cd to device with network scheme ([b662778d](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b662778d1d9ad26fbac29a87af773a5cfa1fbe20))
  *  Press esc to exit rename state. ([4db573bc](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4db573bcf0a3b037bd4c24821ce4d470061f0866))
  *  bookmark item context menu disable. ([73183c9a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/73183c9ab6bf6c0f77117f176b7a57bd7ee86b73))
  *  Device context menu missing item. ([ac7268d1](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ac7268d1e6a5c3e6ba8d701484b6b6c0225dda81))
  *  open in new window from context menu lead crash. ([d34e31f1](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/d34e31f14be2c3e9bff71221186e125601375f11))
  *  background transparent. ([a6eb8cb6](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a6eb8cb6374ae584eb0ba778bfd63a2d6ff5aad0))
* **tag:**  after deleting a tag, the UI of file-manager do not refresh. ([39e5c745](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/39e5c7456155a3ace65cfe373fb064c5f464e65e))
* **toolbar:**  check parent visible before calling show() ([f5f6f44e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f5f6f44e5428b93d9ca6006c24a58ed424a0103f))
* **udisklistener:**
  *  Should start watch changes ([43a646c8](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/43a646c8f45e0059660ffa5986f9bde14044cc32))
  *  crash when remove device. ([46681810](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/466818108d159a4f1c6450a291e7aa05e428e657))
* **wallpaper:**  optimization of thumbnail speed ([c5e80d86](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c5e80d860e50c513b9259964586b2537e1c459bd))

#### Performance

* **cache:**  fix pixmap cache not working ([5285ed3a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/5285ed3a7c68fcbe673f38aa43d123250d7dbc0e))
* **dde-desktop:**  remove the timer of repainting. ([b25e70dc](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b25e70dc8594a67e45bdc6c69c4cd824ed63ce53))
* **tag:**  remove the warning, becase multi sqlite use the same connection name. ([26d38cb6](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/26d38cb6f12a096dae09ccc745bd93c26932d896))

#### Features

*   keep it permissions when copy a fodler ([b0caa7dd](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b0caa7ddb69af265f1b57eea0a68d29cbc66a89c))
*   support gio for gvfs mount files on DFileCopyMoveJob ([daf7464f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/daf7464f68704c3d5fd51761ca4d378169f3a585))
*   if local files changed, quick-search lft can watched and then update this cache. ([1bfac759](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1bfac75922de820cf9754e09d6cba3aa6f0896b8))
*   support fuzzy search. ([4b6e533a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4b6e533a5f88d676fe107d2dfad9ad90ee9d9396))
*   keep its modification time when copying a file ([aae1a516](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/aae1a51635d9a9a1c3fbd1b029a2f249d155b590))
*   sort the files by inode when coopy/move ([df8d6dba](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/df8d6dba1636e64e52e494379e5eccd5a9ea048b))
*   check the file integrity when copy ([04e1844a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/04e1844ad265b434367901c779b5600c6b36bb6a))
*   add class DStorageInfo ([fece5bdb](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/fece5bdba143fb76f035e695e932f3420dba2fad))
*   add new IO operation interfaces of file copy/move ([0371a1fa](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/0371a1fa9805f4962e0dc4e43f77c15aa4d5ef56))
*   add UI for the switch of quick-search. ([648510de](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/648510de51c9eea9edaf2dc1ab683afed71706b3))
*   disable DDE file chooser dialog for "handbrake" ([24a2c611](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/24a2c6117b379b525d79101e094f25afd5a2585e))
*   register quick-search to dde-file-manager-daemon. ([275b7f9a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/275b7f9ad3f111d4e61b3316e2382cc7aa17b0f3))
*   keep the left sidebar width when the window width change ([f99afbea](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f99afbeaffe6d0e7fecdd1c0a3ca22ad3e25c4c6))
*   support fast search.           (1), handle that partion was unmounted suddenly.           (2), increase api for searching in specify path.           (3), new way to load .lft which exists but be filtered.           (4), new way to get the mount point of the specify path. ([f6ed7079](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f6ed7079631e4920ee08ef6f2630eb85a3244946))
*   disable DDE file chooser dialog for "libreoffice" ([4b98de18](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4b98de182f8ca9fb420dfdeae84fa0b746d84525))
*   support touch event for DFileView ([dd3f9ed2](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/dd3f9ed288df7a5ea8302af1397d92d69913342e))
*   support custom the non allowable characters of file name ([e21238d4](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e21238d44eab955ba558179828bb4a0856ed3ed8))
*   add udisks2 support ([c8a1c113](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c8a1c1137de27c7d6202f4685ddebee424839034))
*   add file io library ([5c69af6f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/5c69af6f4170763924790ab7eebda4f36addbc18))
*   increasing monitoring for dde-anythingmonitor, you can configure it configuration files. ([a9b43a39](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a9b43a395658e8d2b07e13e55df239658a689f2c))
*   set the file manager window default size to 800x600 ([564de733](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/564de7335543e62bf1381eb2beb1d992f8cb3d68))
*   support "standard://" url ([614eb98b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/614eb98b4721d158a5982ef5475aabdf53a042ea))
*   add DFMSettings class ([62aaf3d5](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/62aaf3d597d602f84efd5140f0b90f5b4275174b))
*   add "addChildrenList" for JobController ([1cc5f90e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1cc5f90ebf5077d2faaa2e4c89b6a024cc88ea25))
*   add error dialog on failed for mount device ([3349b0b7](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/3349b0b785ec3698c1d164529a3968a52ab09076))
*   change the api of TagManager for changing the color of tag. ([2b6f0a1c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2b6f0a1c3cd44d0eaf7e50b2523d325b4d15c8c4))
* **ProgressBar:**  round radius and better UX ([d8cb7863](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/d8cb786380166f253f5b7f00c18629c128b679b0))
* **addressbar:**
  *  placeholder text center alignment ([24637cae](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/24637cae33c7d20eca9969308ba7cb5b1e1b1d14))
  *  glowing animation ([5f49b8a9](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/5f49b8a982f36bafb798b2148356be87cb447198))
* **authDlg:**
  *  disable combobox when user can't set permission ([2c4673f3](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2c4673f346f472a5774c53ad827751329e86ac66))
  *  set permission from auth dialog ([eee31551](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/eee31551356875d23bec82c5cdfac1de774d9c7b))
* **authMgr:**
  *  interface for permission management ([64290a15](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/64290a15869559cf5b2eea99e671c1cec2219431))
  *  Add back the auth manager widget ([16bd61b7](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/16bd61b775c9268634d524e31ae1d764c0f804ec))
* **bookmark:**  Can only have one bookmark with same url ([c15ed1a8](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c15ed1a8183fbade737ad944f66b7de0cd88d3a5))
* **cli:**  Use -r arg to open as root ([076160db](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/076160dbec8c367b688f640e2d9d61b8b89195c7))
* **completer:**  Case-insensitive completion. ([71640631](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/71640631ccb508553815df7c62d01bae26988675))
* **computerView:**  rename patition at computer view ([65fdb91b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/65fdb91ba5c1da64ccb56d88c6e287c881f7cd7e))
* **doc:**
  *  better index page ([bb8935f8](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/bb8935f86513111f1f9bb1601603f30b48ccccce))
  *  Documentation improvement. ([586f2123](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/586f2123153793cb8bd2d484318f38d66f1999a7))
  *  Macro for \param and \return. ([b356f093](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b356f093df5c82d5f38b034857b1c1ca8e88b63b))
  *  Documention infrastructure for DFM. ([05ae37dc](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/05ae37dcce0cd6b9e0053bbeb7c0d6b150f95279))
* **fileView:**
  *  uniform view for all type of file ([ecbcea9a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ecbcea9a03b58edebd373e9562edcd1185526af9))
  *  remember column width ([2506d45f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2506d45f4503a91aa98e7c675199be06e2c36061))
  *  column size adjustable ([e0fb498d](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e0fb498d203dfa35471578f2c1ee905922dbb5b3))
* **filejob:**  check the target storage free space ([83f838e8](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/83f838e8e1a6d6cd79e5e2889b8347e444d0bb33))
* **fileview-sortorder:**  now showing files supports to configure through a json file. ([342f4213](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/342f4213d08feab70501c99066242d9658775507))
* **github:**  Adding issue template ([e699f70f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e699f70f26725da4481e6cd2bee6fc883ef2ab62))
* **gvfsMountCli:**  warn user if we got any error ([d0aef908](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/d0aef908246983c5726be0e49096ed0531f1477d))
* **gvfsMountMgr:**  Show a dialog when we got error ([9ebb06bf](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/9ebb06bf2935a61b62ac2e463df1e6c61ad6611d))
* **menuExt:**  Set the working dir and allowed Args key ([4001d50a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4001d50a35716ca6f464088119b8dfda26c6a05b))
* **mount:**
  *  check if is encrypted ([37803366](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/37803366abd872e23d91cd19beb8214a8fbdaca6))
  *  new scheme for mounting support ([6f2fc705](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/6f2fc70575227c64361dee2b6d671b5cb2f610eb))
* **mountPlugin:**  better scroll behavior ([1a093d77](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1a093d7700799c5e907517abac10eb5d797967a8))
* **network:**  slient option to prevent request network ([8edd92f1](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/8edd92f109250375c5bb4e770e3527b6bddaef61))
* **partition:**  ability to rename partition ([8a2fb7ff](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/8a2fb7ffa4e3427f4737010e6a5b273d6cbc99aa))
* **quick-search:**
  *  support filter files when is searching files. ([564a492f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/564a492fe66d809a490e8f2c91b3c3428f2a4835))
  *  create quick-search plugin(which is used to update .lft). ([44bde2b2](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/44bde2b2ec0dc2f0e64e7b487c0945385969d92f))
* **setting:**  allow hide system disk ([26c23a19](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/26c23a190fc17d79d563284a6616cded0828dd1c))
* **shortcut:**  Ctrl + number to switch view mode ([6e3bd763](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/6e3bd763d57f597bc41865267a0e1973bf5f1945))
* **sidebar:**
  *  remember sidebar width ([be425a8c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/be425a8c25f0e7e0c0ea81155c414ecab3b826d5))
  *  bookmark support drag-to-delete ([1ee8e4f6](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1ee8e4f640824b7cf1f9d3fcfb7bb4587af6e863))
  *  tag now support remember drag order ([ded51b54](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ded51b54f70e06a545d622510c6b89bf4f738058))
  *  new sort order interface ([e4327492](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e432749261f91acfc756ed3a593f65bfb442c815))
  *  Sidebar save bookmark to new place. ([a202b3c8](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a202b3c8bd7cd1c97bdd4894f0b75c38f9242273))
* **smb:**  Display a notice msg when samba not installed ([cb03f103](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/cb03f103cd1c2a24bc72ddb6c7ff456329deb9ed))
* **tag:**  now anythingmonitor support white-list and black-list. ([5fd0ac87](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/5fd0ac87b5f976babdbeedcdb3e3882751cbd9b4))
* **theme:**  use icon from theme ([09dd4814](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/09dd48146b973ccb1400f21a15cbc4aacbd22fa1))
* **trash:**  Folder is empty hint ([3dc08271](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/3dc082712dd592c27805a6dde0c12ff209e6eb5f))



<a name="4.5.4"></a>
## 4.5.4 (2018-05-18)




<a name="4.5.3"></a>
## 4.5.3 (2018-05-18)


#### Bug Fixes

*   crash in remove files on tag file view ([51f247ea](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/51f247ea4a33fc3069e4815a89bdf0a5c0d0769e))



<a name="4.5.2"></a>
## 4.5.2 (2018-05-17)


#### Bug Fixes

*   crash in DStyledItemDelegate::commitDataAndCloseActiveEditor ([d7373f08](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/d7373f08f4f1087d96688a7fa24d77db7845b862))
*   remove the mount-point of files when storing file into sqlite. ([3eec6a61](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/3eec6a61e8f009c06120a82f0d894e2f67e87995))
* **menu:**  be out of a Sperator befor tag-info(menu action). ([e290cc55](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e290cc55750a1d119ee7ad8dc05d1faeb722c836))
* **wallpaper:**  thumbnail support HiDPI. ([ab202327](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ab202327168d6b9485718b232487958c8d24dc57))



<a name="4.5.1"></a>
## 4.5.1 (2018-05-16)


#### Bug Fixes

*   setting "Always to open in new window" does not work in computer view ([aced802f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/aced802f8050e7cc91458c1a30e1eb2ce0799bb9))
*   crash when access bookmark:/// ([731cb483](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/731cb4834d29cb0b42756c8c79a362eed922e504))
*   ignore G_IO_ERROR_CANCELLED error on copy/move file by gio ([1455d618](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1455d6187ec670105c2a946e4164a3faecff22b9))
*   the last line of the file name in the icon mode is incomplete ([f532cd78](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f532cd784249ed0b6365c04959a26283e8999851))
*   cannot revocaion same operator on file chooser dialog ([c6c89c47](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c6c89c478da6bda7c9c914ffbf558a82b0e70d2a))
*   remove extra columns of the search view ([532b9aeb](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/532b9aeb33e417fafec7aaaf48689a6ba9220419))
*   distinguish cutting-operation of files in different partion through dde-file-manager. ([6f4e8fde](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/6f4e8fde6d80c2783aca79a11263e1825882e27c))
*   preview plain text files on search/tag file view ([5e90d796](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/5e90d796c235b03f1092e5040e0143a170ab17b6))
*   the file item will be removed when move the file in current directory ([b292dec3](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b292dec348a84879377679e2f159971710820a6c))
*   show multiple text edit widget on multiple window when rename the tag file on left sidebar ([82da0142](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/82da01428ac591808cddd6709f8765d52774fb73))
*   change std::thread to QThread in dde-anythingmonitor      and after renaming files will emit untagFiles and filesWereTagged. ([bcdfc47a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/bcdfc47add78b10b8c3f59a42aed118623d848cc))
*   cannot revocation the batch rename operator of file ([f35b5103](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f35b51030bb7995889eea7d7f6567b802f4d00db))
*   not sync tag info when reversal file operator by "Ctrl+Z" ([ff92b949](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ff92b9493ff7911c5410923981334e023f987eda))
*   change the tag name to determine if the old and new names are equal ([57fa131f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/57fa131f1d75f2c14995fef5dd6e8c35441d3019))
*   search file view not update when delete the searched file ([5a0bc120](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/5a0bc12042b82e399be59f6e42ba676f8d96eb13))
*   add inspection for checking whether the thread is running if call DThreadUtil::runInThread. ([b36e513b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b36e513b9063f7ef94155ac9525d8e1410429ac3))
*   in dark theme, the deep-blue was shown light-blue. ([c3756c8f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c3756c8f04d20571b9f2d559b03d57df7cb4e529))
*   the tag file view not update when the local file deleted. ([0b8494e0](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/0b8494e009c44ac39030f02ee1835e703873e5cf))
*   Change tag color backend api changed ([991fedd8](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/991fedd8abdac7f864809c76efed2d7c4ed54046))
*   after deleting a empty tag can not get signal from daemon. ([bd69e2e4](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/bd69e2e4441b94897661a440c36408d8385be5bc))
*   the file non-exists if it url format is "tag:///{Tag Name}" format ([15a8adea](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/15a8adeab76d0712ffe763a87bfd2ecf9a84e980))
*   the "OpenInTerminal" menu action is invalid in tag file view ([8e86a571](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/8e86a571b0822f338acbf113281fda3c03a0b3da))
*   cannot preview file on tag file view ([ef48435f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ef48435fc4d4b9d62ba69ffbd47eef9587696540))
* **dde-anythingmonitor:**  cpu occupancy rate is high ([2bc85248](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2bc8524823fe5f335d993b83fb2aa3081bc32a4b))
* **dde-desktop:**  crash when drag/drop files ([b640c0ea](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b640c0ea1a82ed8d829568d6f0fffd7973cd8370))
* **video preview plugin:**  pause button not visible ([cbc04090](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/cbc04090b72a30ce1ae7571b58965c6d2a7b5167))

#### Features

*   support operator+= for usual container in Qt. ([366b4490](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/366b449019197bc79d9d432f17989c232e45bd13))
*   use asynchronous operator for tagging files ([0b314716](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/0b314716947f70bd71350a037d70bdf95221404d))
*   change the api of TagManager for changing the color of tag. ([f50c71a1](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f50c71a1a438f1ce13281de3a00f7bb3a26f9f3a))
*   save the file tag name list in trashinfo ([3130bcb2](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/3130bcb2f9b7c633b906f4a3cfe838c335479bdf))
*   support search on tag file view ([f5c00011](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f5c00011bb3b819f3796ff292cabb87936105126))
*   add a new api for DUrl and adjust DUrl::fromUserTaggedFile make it easy. ([4d195392](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4d195392499cb29b4cf50770b144d0eb7deae639))



<a name="4.5.0"></a>
## 4.5.0 (2018-05-04)


#### Bug Fixes

*   after cutting files and moving file to trash remove old file. ([9e721960](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/9e7219607d788705e0cc162db87e0d91a6e66452))
*   crash in FileJob::doMove ([53c6c559](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/53c6c5596089bb7639b29c5e84f8220bf7cc8267))
*   update left sidebar on tag list changed ([cebad2a0](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/cebad2a0985657ea99b5efbf65495b7e7da2c895))
*   sync the property of tag after deleting files. ([76265637](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/7626563744955aeff8110651468c2034defe6c58))
*   update tag file view when the local file modified of the tag file ([96cce4d7](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/96cce4d76a982225cbb07a82e5679e26289f5b0b))
*   update tag view when the file tag/untag ([1aa7a334](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1aa7a3346efdfb9beeb8344c2b2897c86987b000))
*   update tag database when file renamed/deleted ([6ff53beb](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/6ff53bebdb26c119a32f0de50807fdfb03f1b0ae))
*   sync the property of files through signals of dbus. ([d210aea8](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/d210aea886a591a62fb77de09acd1b6cdaeb1ec4))
*   crash in tag file view ([be16e6ee](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/be16e6ee32e039b91eac6761a7dbd89ab367a2a5))
*   can not delete tag ([637440f5](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/637440f51b7d3a9e217ec94f6647fe142af905db))
*   the default color tag translates ([040852fb](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/040852fbd4b3d8228976d720ea406e23d0a96416))
*   when setting file tag information, other files will be mistakenly deleted ([b174d64b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/b174d64b7f7afe827ad4c85bf312e8f9df0e4698))
*   the tag edit widget position ([ee99f1b8](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ee99f1b86220dc0a0f51fcf90b593e77116c4560))
*   fix translates in DFileDialog::onAcceptButtonClicked ([436b2071](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/436b20710e7a55a30ac880f44b47fa56942a538d))
*   failed on copy file if the target is existed(ths "m_isSkip"/"m_isCoExisted"/"m_isReplaced" is dirty) ([0f294494](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/0f2944946aa305dfb163658376cec8274c6401a6))
*   cannot save the file view state(view mode & icon size) ([4a080f92](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4a080f92acd907f75a08b9487e59a5cd01a5c3fe))
*   crash in RequestEP::processEPChanged ([f50af033](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f50af0331fdaf5ad4c7b7419b0541a05c12c8739))
*   bookmarkitem rename crash issue. ([7470bc79](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/7470bc799a98225286dc95e8ca4806fd2b36b447))
*   can not drag move file to trash ([f88c0a03](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f88c0a03507cfd5a6c9600f56ae2484aa57cdf34))
*   Enter smb scheme and cd to real path. ([321fcf52](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/321fcf52b2f2e8221768bc4d81a2b457aac9d6c4))
*   can not get the file extension propertys on dde-desktop ([2665bf3c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/2665bf3c1e85047d9490db82b507a2573406beef))
*   carsh in close preview directory at destory size worker thread ([cfabe5eb](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/cfabe5eb0c46b24019e0d8668f946fde9c30fd14))
*   ComputerViewItem not correct issue ([54e02a42](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/54e02a42863fae00b2cfce3a299d9203e2758007))
*   can not open help in dde-desktop ([fff57238](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/fff572389b251b143fc61b1a6e49ebbab4490de3))
*   support hi-DPI for ExpandedItem ([c242abb0](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c242abb0d030c36f5899e22cd427eeac0bfe7885))
*   crash on preview the music files ([6728e7b3](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/6728e7b33251e6f22c742031c3b1e4f5998389ae))
*   file icon view text background path on select single file and the file name is need expansion ([0be51a23](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/0be51a23fdfa02a8b358289f2ee3b9eef1c71534))
*   crash in RequestEP::processEPChanged ([29177816](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/29177816fa65243743df8c09580c80b533af1162))
*   reset configs not apply on file view ([11cbf960](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/11cbf96038d5ac26a70b8653c67c4e92d44516e7))
*   Can't type trailing space when editing file name. ([322ebe1d](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/322ebe1da324d7e6bd2845b6e10ddd7c8bd9e496))
*   keep the trash file view column count to 2 ([45b4184e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/45b4184e70df43ae4a954251019ca103e51ac919))
*   Ctrl + Shift + Tab can not toggle previous tab ([638b8a5c](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/638b8a5c321014de0abaabe0ee29e56d23b40fc0))
*   can not select files by mouse button click when the Shfit key is pressed ([a9cb0406](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a9cb040691ca4d7d67f430b1f03f6d1d4b7c0052))
*   trash file view font is wrong ([a3aa78ae](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/a3aa78ae85601e0b9c08ac7bc3ac6ecbbc419c98))
*   can not share file on search file view ([3d1f5cbf](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/3d1f5cbf050d38ea1d9f3e19c9045650d524a7d5))
*   the files not refresh on add tag info ([3fc15155](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/3fc1515575f4e8f37de0cf92abb7367e348e4f45))
*   a logic error when changing the color of tag and delete redundant code. ([f58c5943](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f58c59433c0158245d45581759efb4876fa174ab))
*   destory file preview resources on dialog close ([fa7b2733](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/fa7b27338bd7b02101134ccb5be3d0a94e658e94))
*   can not get the files through tag and tag files through DTagEdit. ([3462058b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/3462058b0a69293d8b01ad28847eb908ce6b000b))
*   new way to get home path of user(s). ([8c6df80a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/8c6df80a118801c9afd9bb2ab2913941ee0f190a))
*   crash in RequestEP::processEPChanged ([062a3cb2](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/062a3cb24f7824c2ac1cc255156d65c067d891af))
*   crash when clean trash by dock trash plugin ([1b30c18b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/1b30c18b40555862a255947aeeda0922b12a3d3c))
*   support multi read and single write. ([e3c4f49a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/e3c4f49a75ac29f83afbecf452587f8f5ffcf07e))
*   open search file view on mount device by left sidebar ([087fe6fe](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/087fe6fe485627ee26f57dd30e92887c4e4d3587))
* **dde-desktop:**  update file view when the file tag info changed ([bc6a729d](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/bc6a729d57817247809209e93670d4be17cb53b6))
* **desktop:**  delay show wallpaper choose frame ([ad289da9](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/ad289da9ce80982a7abb2014aea32712d9df8d86))
* **tag:**
  *  after delete a tag through left-sidebar will cause segment fault. ([260d4e2f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/260d4e2f76990b40d44d3d71ba324a46a84640a5))
  *  change the returned type of a API and delete redundant code. ([f9bc5246](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f9bc52464c953808f8131f968ea77973bc99b4de))
  *  fail to umount partion and segment fault. ([79ca3f5d](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/79ca3f5d41f3a0cdc8564e2ade9606532ebafe75))
* **wallpaper:**  delayed loading wallpaper ([dc62a6e5](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/dc62a6e5dfa17d9dc5526fe8baadfab97b102d1f))

#### Features

*   after delete tags will emit a signal which includes the files in current tags. ([dae4ea45](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/dae4ea456ec3e79a31435997cda1f45b413ce1c5))
*   update file view when file tag info changed ([74df5c5a](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/74df5c5ab3bfac660cd918490e704d5189ac05e4))
*   support operation the file on tag file view ([61d1852d](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/61d1852d7bda42865efa748ef4272397ea2469ca))
*   update translates ([418e258f](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/418e258feb0207f7116520a418e2eb05dcb0087c))
*   add tooltip for tag action widget on hover state ([9e897967](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/9e8979678a679ee11e241e332b6295f90d8967f2))
*   copy the file tag infos when copy the file ([6bdff4ee](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/6bdff4ee749753541f2989a9122db678da4492b2))
*   add signal "DFileService::fileCopied" ([c0bcf69b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/c0bcf69bba396a57999dbe6b446f980be36582eb))
*   add new API for tagging support. ([6813841e](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/6813841ea9fb9812674c510ff95cf98cac27f09e))
*   update translates ([7a8e8df9](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/7a8e8df99d798a6942c6ae4e28d080722c8d71ff))
*   popup dialog when copy file failed by gio ([f6178b25](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/f6178b25f167d5ee12a51467abb40395dc2abd2c))
*   support disable menu actions by config file ([9a77414b](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/9a77414b61f6f0db7c7143559752986f3eb17211))
*   support custom hidden files ([df2d9b05](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/df2d9b057bc99bf2e263d88b0868e580cc7f0e0b))
*   can set border color for DStyledItemDelegate::paintCircleList ([4e400319](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4e400319a0155324cd5e9d63ac75e3ed6d5980cd))
*   paint the file tag infos on file views ([9d077c1d](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/9d077c1db22154aaf909c309607217743f36d8fa))
*   support extension popertys for DFileInfo ([21dac0fd](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/21dac0fd22aa43bd29e1ff24ce7abd8a5a3218bb))
*   can undo restore trash files by Ctrl+Z ([cb48d412](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/cb48d412fa3980220b2e8a6cfe30fdd1d006747f))
* **dock disk plugin:**  update disk info when show ([4fa57584](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/4fa57584e8bf2aba989c55758e901f67711bc797))
* **tag:**
  *  support tag protocol(tag://) and give supports to monitor tag(s). ([6531d482](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/6531d482a034862c16a22216cd23e61db14ad914))
  *  support file watcher. ([fa661305](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/fa661305a2a4da03213ec4ca3b9ae3534671ff99))



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



