/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <android/log.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <fcntl.h>

/* 宏定义begin */
//清0宏
#define MEM_ZERO(pDest, destSize) memset(pDest, 0, destSize)

//LOG宏定义
#define LOG_INFO(tag, msg) __android_log_write(ANDROID_LOG_INFO, tag, msg)
#define LOG_DEBUG(tag, msg) __android_log_write(ANDROID_LOG_DEBUG, tag, msg)
#define LOG_WARN(tag, msg) __android_log_write(ANDROID_LOG_WARN, tag, msg)
#define LOG_ERROR(tag, msg) __android_log_write(ANDROID_LOG_ERROR, tag, msg)

/* 内全局变量begin */
static char c_TAG[] = "onEvent";
static jboolean b_IS_COPY = JNI_TRUE;
static const char APP_DIR[] =
		"/data/data/cn.edu.bjtu.tsplaycool.uninstallfeedback";
static const char APP_FILES_DIR[] =
		"/data/data/cn.edu.bjtu.tsplaycool.uninstallfeedback/uninstall";
static const char APP_LOCK_FILE[] =
		"/data/data/cn.edu.bjtu.tsplaycool.uninstallfeedback/uninstall/lockFile";
static const char APP_OBSERVED_FILE[] =
		"/data/data/cn.edu.bjtu.tsplaycool.uninstallfeedback/uninstall/observedFile";
static const char APP_UNINSTALL_PID_FILE[] =
		"/data/data/cn.edu.bjtu.tsplaycool.uninstallfeedback/uninstall/uninstall"; // 存储c层监听进程的pid
void Java_cn_edu_bjtu_tsplaycool_uninstallfeedback_utils_UninstallUtil_init(
		JNIEnv* env, jobject thiz, jstring intentAction, jstring url,
		jint version) {

	jstring tag = (*env)->NewStringUTF(env, c_TAG);

	//初始化log
	LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &b_IS_COPY),
			(*env)->GetStringUTFChars(env, (*env)->NewStringUTF(env, "init OK"),
					&b_IS_COPY));

	//fork子进程，以执行轮询任务
	pid_t pid = fork();
	if (pid < 0) {
		//出错log
		LOG_ERROR((*env)->GetStringUTFChars(env, tag, &b_IS_COPY),
				(*env)->GetStringUTFChars(env,
						(*env)->NewStringUTF(env, "fork failed !!!"),
						&b_IS_COPY));
	} else if (pid == 0) {

		// 若监听文件所在文件夹不存在，创建
		FILE *p_filesDir = fopen(APP_FILES_DIR, "r");
		if (p_filesDir == NULL) {
			int filesDirRet = mkdir(APP_FILES_DIR, S_IRWXU | S_IRWXG | S_IXOTH);
			if (filesDirRet == -1) {
				LOG_ERROR((*env)->GetStringUTFChars(env, tag, &b_IS_COPY),
						(*env)->GetStringUTFChars(env,
								(*env)->NewStringUTF(env, "mkdir failed !!!"),
								&b_IS_COPY));

				exit(1);
			}
		}
		// 若被监听文件不存在，创建文件
		FILE *p_observedFile = fopen(APP_OBSERVED_FILE, "r");
		if (p_observedFile == NULL) {
			p_observedFile = fopen(APP_OBSERVED_FILE, "w");
		}
		fclose(p_observedFile);
		// 创建锁文件，通过检测加锁状态来保证只有一个卸载监听进程
		int lockFileDescriptor = open(APP_LOCK_FILE, O_RDONLY);
		if (lockFileDescriptor == -1) {
			lockFileDescriptor = open(APP_LOCK_FILE, O_CREAT);
		}
		int lockRet = flock(lockFileDescriptor, LOCK_EX | LOCK_NB);
		if (lockRet == -1) {
			LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &b_IS_COPY),
					(*env)->GetStringUTFChars(env,
							(*env)->NewStringUTF(env,
									"observed by another process"),
							&b_IS_COPY));

			exit(0);
		}
		LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &b_IS_COPY),
				(*env)->GetStringUTFChars(env,
						(*env)->NewStringUTF(env, "observed by child process"),
						&b_IS_COPY));
		// 把c层监听进程的pid写入APP_UNINSTALL_PID_FILE
		FILE *uninstallFile = fopen(APP_UNINSTALL_PID_FILE, "w");
		fprintf(uninstallFile, "%d", getpid());
		fclose(uninstallFile);

		//子进程注册目录监听器
		int fileDescriptor = inotify_init();
		if (fileDescriptor < 0) {
			LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &b_IS_COPY),
					(*env)->GetStringUTFChars(env,
							(*env)->NewStringUTF(env,
									"inotify_init failed !!!"), &b_IS_COPY));

			exit(1);
		}

		int watchDescriptor;

		watchDescriptor = inotify_add_watch(fileDescriptor, APP_OBSERVED_FILE,
				IN_DELETE);
		if (watchDescriptor < 0) {
			LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &b_IS_COPY),
					(*env)->GetStringUTFChars(env,
							(*env)->NewStringUTF(env,
									"inotify_add_watch failed !!!"),
							&b_IS_COPY));

			exit(1);
		}

		//分配缓存，以便读取event，缓存大小=一个struct inotify_event的大小，这样一次处理一个event
		void *p_buf = malloc(sizeof(struct inotify_event));
		if (p_buf == NULL) {
			LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &b_IS_COPY),
					(*env)->GetStringUTFChars(env,
							(*env)->NewStringUTF(env, "malloc failed !!!"),
							&b_IS_COPY));

			exit(1);
		}
		//开始监听
		LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &b_IS_COPY),
				(*env)->GetStringUTFChars(env,
						(*env)->NewStringUTF(env, "start observer"),
						&b_IS_COPY));
		//read会阻塞进程，
		size_t readBytes = read(fileDescriptor, p_buf,
				sizeof(struct inotify_event));

		sleep(1); // 睡眠一秒钟，让系统删除干净包名。(防止监听到observedFile被删除后，而包目录还未删除，代码就已经执行到此处。导致不弹出浏览器)

		//走到这里说明收到目录被删除的事件，注销监听器
		free(p_buf);
		inotify_rm_watch(fileDescriptor, IN_DELETE);
		// 判断是否真正卸载了
		FILE *p_appDir = fopen(APP_DIR, "r");
		// 确认已卸载
		if (p_appDir == NULL) {
			//目录不存在log
			LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &b_IS_COPY),
					(*env)->GetStringUTFChars(env,
							(*env)->NewStringUTF(env, "uninstalled"),
							&b_IS_COPY));

			if (version >= 17) {
				//4.2以上的系统由于用户权限管理更严格，需要加上 --user 0
				if (strcmp((*env)->GetStringUTFChars(env, intentAction, NULL),
						"android.intent.action.VIEW") == 0) {
					// 系统没有自带浏览器，启动默认浏览器
					LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &b_IS_COPY),
							(*env)->GetStringUTFChars(env,
									(*env)->NewStringUTF(env,
											"Android big than 17，default"),
									&b_IS_COPY));
					execlp("am", "am", "start", "--user", "0", "-a",
							"android.intent.action.VIEW", "-d",
							(*env)->GetStringUTFChars(env, url, NULL),
							(char *) NULL);
				} else {
					//启动自带浏览器
					LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &b_IS_COPY),
							(*env)->GetStringUTFChars(env,
									(*env)->NewStringUTF(env,
											"Android big than 17，self "),
									&b_IS_COPY));
					execlp("am", "am", "start", "--user", "0", "-a",
							"android.intent.action.VIEW", "-d",
							(*env)->GetStringUTFChars(env, url, NULL), "-n",
							"com.android.browser/com.android.browser.BrowserActivity",
							(char *) NULL);
				}
			} else {
				if (strcmp((*env)->GetStringUTFChars(env, intentAction, NULL),
						"android.intent.action.VIEW") == 0) {
					//系统没有自带浏览器，启动默认浏览器
					LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &b_IS_COPY),
							(*env)->GetStringUTFChars(env,
									(*env)->NewStringUTF(env,
											"Android small than 17，default "),
									&b_IS_COPY));
					execlp("am", "am", "start", "-a",
							"android.intent.action.VIEW", "-d",
							(*env)->GetStringUTFChars(env, url, NULL),
							(char *) NULL);
				} else {
					//启动自带浏览器
					LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &b_IS_COPY),
							(*env)->GetStringUTFChars(env,
									(*env)->NewStringUTF(env,
											"Android small than 17，self"),
									&b_IS_COPY));
					execlp("am", "am", "start", "-a",
							"android.intent.action.VIEW", "-d",
							(*env)->GetStringUTFChars(env, url, NULL), "-n",
							"com.android.browser/com.android.browser.BrowserActivity",
							(char *) NULL);
				}
			}

		} else {
			//如果observedFile被删除了，但是实际未卸载，则不弹出浏览器。也防止了一些误弹浏览器操作。
		}

	} else {
		//父进程直接退出，使子进程被init进程领养，以避免子进程僵死
	}

}

// 杀死c层监听进程
void Java_cn_edu_bjtu_tsplaycool_uninstallfeedback_utils_UninstallUtil_kill(
		JNIEnv* env, jobject thiz) {
	jstring tag = (*env)->NewStringUTF(env, c_TAG);
	LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &b_IS_COPY),
			(*env)->GetStringUTFChars(env,
					(*env)->NewStringUTF(env, "调用kill方法"), &b_IS_COPY));
	// 读取pid
	FILE *fp = fopen(APP_UNINSTALL_PID_FILE, "r");
	if (fp != NULL) {
		int pid;
		fscanf(fp, "%d", &pid);
		// 杀死指定pid进程
		if (pid > 0) {
			LOG_DEBUG((*env)->GetStringUTFChars(env, tag, &b_IS_COPY),
					(*env)->GetStringUTFChars(env,
							(*env)->NewStringUTF(env, "执行kill方法"), &b_IS_COPY));
			kill(pid, SIGKILL);
		}
	}
	fclose(fp);
}

