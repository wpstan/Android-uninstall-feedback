uninstallFeedback
=================

卸载APK，跳转到反馈页面。

原理：在c层fork一个进程，监听data/data/包名/lib目录。
