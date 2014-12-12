package cn.edu.bjtu.tsplaycool.uninstallfeedback.utils;

import android.content.Context;
import android.os.Build;

/**
 * 监听卸载工具
 * @author Tans
 *
 */
public class UninstallUtil {

	public static void regUninstallService(Context context) {

		int sdkVersion = Build.VERSION.SDK_INT;
		// 监听lib目录，为了防止在任务管理器清空数据也响应
		init("/data/data/cn.edu.bjtu.tsplaycool.uninstallfeedback/lib", "http://www.baidu.com", sdkVersion);
	}

	private static native void init(String packageName, String url, int version);

	static {
		System.loadLibrary("uninstall");
	}
}
