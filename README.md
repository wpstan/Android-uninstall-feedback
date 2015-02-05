uninstallFeedback
=================

卸载APK，跳转到反馈页面。

原理：在c层fork一个进程，监听data/data/包名/lib目录。

注：在华为机型上，如果在java端中直接fork这个c进程，且java端退出调用了system.exit方法，则会出现黑屏崩溃问题。

所以，把这个native调用放在remote service中。在需要监听的项目中，启动一个远程服务，这个远程服务只做一个任务：调用c层代码来fork监听进程。

在C层写了一个Kill方法，在JAVA层调用，可以用来结束C层的监听进程，实现原理：将fork出来的监听进程的pid存在data/data/包名/files/中的uninstall文件中，

当需要结束C层监听进程的时候，读取该文件，获取到pid值，再调用linux中的kill函数结束进程。
