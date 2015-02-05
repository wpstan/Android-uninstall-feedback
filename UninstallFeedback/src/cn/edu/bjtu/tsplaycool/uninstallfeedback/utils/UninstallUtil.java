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
	

	// Service中低调用JNI方法启动C层进程
	public static void regUninstallService(Context context) {

		int sdkVersion = Build.VERSION.SDK_INT;
		String url = "http://tan-shuai.cn";
		String intentAction = getBrowserIntentString(context);
		if (sdkVersion > 0 && !TextUtils.isEmpty(url)
				&& !TextUtils.isEmpty(intentAction)) {
			init("/data/data/cn.edu.bjtu.tsplaycool.uninstallfeedback/lib",
					intentAction, url, sdkVersion);
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

	private static native void init(String packageName, String intentAction,
			String url, int version);

	private static native void kill(); // 根据需要，可以从java层调用次方法杀死c层进程

	static {
		System.loadLibrary("uninstall");
	}
}
