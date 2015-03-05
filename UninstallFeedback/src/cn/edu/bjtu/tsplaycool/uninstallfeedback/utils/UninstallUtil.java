package cn.edu.bjtu.tsplaycool.uninstallfeedback.utils;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.text.TextUtils;

/**
 * 监听卸载工具
 * @author Tans
 *
 */
public class UninstallUtil {

	private static boolean loaded;

	// Service中低调用JNI方法启动C层进程
	public static void regUninstallService(Context context) {

		if (!loaded) {
			return;
		}
		int sdkVersion = Build.VERSION.SDK_INT;
		String url = "http://tan-shuai.cn";
		String intentAction = getBrowserIntentString(context);
		if (sdkVersion > 0 && !TextUtils.isEmpty(url)
				&& !TextUtils.isEmpty(intentAction)) {
			init(intentAction, url, sdkVersion);
		}
	}

	// 判断是否具有系统自带浏览器，打开卸载反馈url的时候，优先使用系统自带浏览器，否在使用系统默认浏览器
	private static String getBrowserIntentString(Context context) {
		Intent protoIntent = new Intent();
		String browser = "com.android.browser/com.android.browser.BrowserActivity";// 优先启动系统自带浏览器
		protoIntent.setComponent(ComponentName.unflattenFromString(browser));
		if (context.getPackageManager().resolveActivity(protoIntent, 0) == null) {
			browser = "android.intent.action.VIEW";
		}
		return browser;// 　启动系统默认浏览器
	}

	private static native void init(String intentAction, String url, int version);

	private static native void kill(); // 根据需要，可以从java层调用次方法杀死c层进程

	// 虽然static代码块在类加载的时候执行，但是有些手机会报错Unsatisfied错误，添加一个try catch捕获该错误
	static {
		try {
			System.loadLibrary("uninstall");
			loaded = true;
		} catch (UnsatisfiedLinkError e) {
			loaded = false;
		}
	}
}
