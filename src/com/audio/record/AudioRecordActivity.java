package com.audio.record;

import android.R.integer;
import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;

import com.audio.record.phone.NetPhone;

public class AudioRecordActivity extends Activity {

	private Button connectB;
	private Button stopB;
	private EditText ipText;

	private NetPhone phone;
	static {  
	    System.loadLibrary("hdaec");  
	}  
	
	private native void main();
	public native static  short[]processTx(short[] in,int size);
	public native static  short[]processRx(short[] in,int size);
	/** Called when the activity is first created. */
	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		connectB = (Button) findViewById(R.id.connectB);
		stopB = (Button) findViewById(R.id.stop);
		ipText = (EditText) findViewById(R.id.ipText);

		main();
		connectB.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				String ip = ipText.getText().toString();
				phone = new NetPhone();
				try {
					phone.startPhone(ip);
					ipText.setEnabled(false);
					Toast.makeText(AudioRecordActivity.this, "start phone ok!", Toast.LENGTH_SHORT).show();
				} catch (Exception e) {
					Log.e(AudioRecordActivity.class.getName(), e.getMessage(), e);
				}
			}
		});

		stopB.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				if (phone != null)
					try {
						phone.stopPhone();
						phone = null;
						ipText.setEnabled(true);
						Toast.makeText(AudioRecordActivity.this, "stop phone ok!", Toast.LENGTH_SHORT).show();
					} catch (Exception e) {
						Log.e(AudioRecordActivity.class.getName(), e.getMessage(), e);
					}
			}
		});

	}

	@Override
	protected void onDestroy() {
		try {
			if(null != phone){
				phone.stopPhone();
			}
			
		} catch (Exception e) {
			Log.e("phone", e.getMessage(), e);
		}
		super.onDestroy();
		android.os.Process.killProcess(android.os.Process.myPid());
	}

}