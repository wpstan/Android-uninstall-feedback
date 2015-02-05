package cn.edu.bjtu.tsplaycool.uninstallfeedback.service;

import cn.edu.bjtu.tsplaycool.uninstallfeedback.utils.UninstallUtil;
import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

public class UninstallMonitorService extends Service {

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		UninstallUtil.regUninstallService(this);
		return super.onStartCommand(intent, flags, startId);
	}

	@Override
	public IBinder onBind(Intent intent) {
		return null;
	}

}
