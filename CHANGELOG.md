<a name="4.5.5"></a>
## 4.5.5 (2018-06-08)


#### Bug Fixes

*   optimize pixmap get of file icon ([983afce0](https://github.com/linuxdeepin/dde-file-manager/tree/develop2.0/commit/983afce03fa0185a6999c19a7faae1e4e206614d))



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



