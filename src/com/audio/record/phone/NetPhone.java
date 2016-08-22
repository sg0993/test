package com.audio.record.phone;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.concurrent.ConcurrentLinkedQueue;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;
import android.os.Environment;
import android.util.Log;
import android.widget.ListView.FixedViewInfo;

import com.audio.record.AudioRecordActivity;

public class NetPhone implements Runnable {	

	private final static int Sample_Rate = 8000;//44100;//8000;
	private final static int Channel_In_Configuration = AudioFormat.CHANNEL_IN_MONO;
	private final static int Channel_Out_Configuration = AudioFormat.CHANNEL_OUT_MONO;
	private final static int AudioEncoding = AudioFormat.ENCODING_PCM_16BIT;

	private AudioRecord phoneMIC;
	private AudioTrack phoneSPK;

	private volatile boolean stoped = true;

	private CallLink curCallLink;

	private int recBufferSize;
	private int playBufferSize;

	private Thread inThread, outThread;

	public void startPhone(String inIP) throws Exception {
		initAudioHardware();

		initCallLink(inIP);

		stoped = false;

		outThread = new Thread(new Runnable() {

			@Override
			public void run() {
				try {
					startPhoneMIC();
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
		outThread.start();

		
		//播放不播了
		curCallLink.listen();
		inThread = new Thread(this);
		inThread.start();

	}

	private void initAudioHardware() throws Exception {
		/* recBufferSize = AudioRecord.getMinBufferSize(Sample_Rate,
		 Channel_In_Configuration, AudioEncoding);
		 playBufferSize = AudioTrack.getMinBufferSize(Sample_Rate,
		 Channel_Out_Configuration, AudioEncoding);*/
		recBufferSize = 1024;
		playBufferSize = 1024;
		phoneMIC = new AudioRecord(MediaRecorder.AudioSource.MIC, Sample_Rate,
				Channel_In_Configuration, AudioEncoding, recBufferSize);
		phoneSPK = new AudioTrack(AudioManager.STREAM_MUSIC, Sample_Rate,
				Channel_Out_Configuration, AudioEncoding, playBufferSize,
				AudioTrack.MODE_STREAM);
		phoneSPK.setStereoVolume(0.7f, 0.7f);

	}

	private void initCallLink(String inIP) {
		curCallLink = new CallLink(inIP);
	}

	@Override
	public void run() {
		/*try {
			curCallLink.listen();
		} catch (IOException e) {
			e.printStackTrace();
		}*/
	    
		startPhoneSPK();
	}

	
	private static final int FRAME_SIZE = 80;
	private static final int BODY_LENGTH = FRAME_SIZE * 2;
	private static final int START_COUNT = 6;
	private int mCount = 0;
	private short[] mStartData;
	
	private FileOutputStream getOutPutStream(String fileName) throws IOException {
	    File dirFile = new File(Environment.getExternalStorageDirectory().getAbsolutePath()+ "/AudioTest");
	    if(!dirFile.exists()){
	        dirFile.mkdirs();
	    }
	    File  file  = new File(dirFile, fileName);
        if(file.exists()){
            file.delete();
        }
        file.createNewFile();
        FileOutputStream fileOutputStream = new FileOutputStream(file);
        return fileOutputStream;
    }
	private void startPhoneMIC() throws Exception {
		curCallLink.open();
		phoneMIC.startRecording();
		mStartData = new short[FRAME_SIZE];
		for (int i = 0; i < FRAME_SIZE; i++) {
		    mStartData[i] = 0;
        }
		
		/*************************/
		FileOutputStream recordPreOutPutStream = getOutPutStream("recordPre.pcm");
		FileOutputStream recordLastOutPutStream = getOutPutStream("recordLast.pcm"); 
		FileOutputStream recordNetPreOutPutStream = getOutPutStream("netPre.pcm");
		
		/*************************/
	
		
		while ((!Thread.interrupted()) && !stoped) {
			if(stoped){
				break;
			}
			//1.录音
			short[] testVoice = new short[FRAME_SIZE];
			int num  = phoneMIC.read(testVoice, 0, FRAME_SIZE);
			
			
			//2.处理
			short[] processVoice = AudioRecordActivity.processTx(testVoice,num);
			ByteArrayOutputStream txProcess = new ByteArrayOutputStream();
			
			/****************************/
			ByteArrayOutputStream origin = new ByteArrayOutputStream();
			/****************************/
			Log.i("ADT HDAEC TEST", "startPhoneMIC() processVoice[0]="+processVoice[0]+",,,processVoice[1]:"+processVoice[1]+",,111");
			//3.pcm to alaw
			for (int i = 0; i < num; i++) {
                txProcess.write(s16_to_alaw(processVoice[i]));              
                /****************************/
                origin.write(s16_to_alaw(testVoice[i])); 
                /****************************/
                
                
            }
			byte [] data = txProcess.toByteArray();
			//4.发出去
            if(null != curCallLink.getOutputStream()){
                curCallLink.getOutputStream().write(data, 0, data.length);
            }  
            
            /********************************************/
            //原始数据写入文件
            recordPreOutPutStream.write(origin.toByteArray());  
            
            //优化数据写入文件
            recordLastOutPutStream.write(data); 
            /*********************************************/
  
            byte[] jsonByte = mRecordQueue.poll();
            if(null == jsonByte){
                AudioRecordActivity.processRx(mStartData, FRAME_SIZE);
                phoneSPK.write(mStartData, 0, FRAME_SIZE);  
               continue;
           }
            //7.processRx 处理
            short shortArray[] = new short[jsonByte.length];
            for(int f = 0; f < jsonByte.length; f++){
                int pcm = alaw_to_s16(jsonByte[f]);
                shortArray[f] = (short)pcm;
            }     
            
            /*********************/
            recordNetPreOutPutStream.write(jsonByte);
            /****************************/
            
            short[] processSpeakVoice = AudioRecordActivity.processRx(shortArray,jsonByte.length);
            Log.i(TAG, "jsonByte.length=" + jsonByte.length);
            Log.i("ADT HDAEC TEST", "startPhoneMIC() processSpeakVoice[0]="+processSpeakVoice[0]+",,,processSpeakVoice[1]:"+processSpeakVoice[1]+",,22222");
            //8.播放处理后的s16数据
            //phoneSPK.write(processSpeakVoice, 0, jsonByte.length); 
            
            //8.播放网络受到的数据
            phoneSPK.write(jsonByte, 0, jsonByte.length);   
            
            //8.播放处理前的s16数据
            //phoneSPK.write(shortArray, 0, jsonByte.length);   
		}
		closeOutPutStream(recordPreOutPutStream);
		closeOutPutStream(recordLastOutPutStream);
		closeOutPutStream(recordNetPreOutPutStream);
	}
	
	


    private void closeOutPutStream(FileOutputStream fileOutputStream) throws IOException {
        fileOutputStream.flush();
        fileOutputStream.close();
    }


    private static final String TIME_FORMAT = "yyyy-MM-dd HH:mm:ss:SSS";
	private static final int TIME_FORMAT_LENGTH = TIME_FORMAT.length();
	private static final String TAG = "ADT HDAEC TEST";
	public static String getCurrentTimeString() {
		SimpleDateFormat sDateFormat = new SimpleDateFormat(TIME_FORMAT);
		String time = sDateFormat.format(new Date());  
		return time;
	}

	byte s16_to_alaw(int pcm_val)
	{
	    int		mask;
	    int		seg;
	    byte	aval;
	    
	    if (pcm_val >= 0) {
	        mask = 0xD5;
	    } else {
	        mask = 0x55;
	        pcm_val = -pcm_val;
	        if (pcm_val > 0x7fff)
	            pcm_val = 0x7fff;
	    }
	    
	    if (pcm_val < 256)
	        aval = (byte) (pcm_val >> 4);
	    else {
	        /* Convert the scaled magnitude to segment number. */
	        seg = val_seg(pcm_val);
	        aval = (byte)((seg << 4) | ((pcm_val >> (seg + 3)) & 0x0f));
	    }
	    return (byte) (aval ^ mask);
	}
    int val_seg(int val)
    {
        int r = 0;
        val >>= 7;
        if ((val & 0xf0) > 0) {
            val >>= 4;
            r += 4;
        }
        if ((val & 0x0c) > 0) {
            val >>= 2;
            r += 2;
        }
        if ((val & 0x02) > 0)
            r += 1;
        return r;
    }
	private void startPhoneSPK() {
//		byte[] gsmdata = new byte[playBufferSize];
		int numBytesRead = 0;
		phoneSPK.play();
		mRecordQueue.clear();
		try {
			while ((!Thread.interrupted()) && !stoped) {
				if(stoped){
					break;
				}
				if(null == curCallLink.getInputStream()){
					continue;
				}
				byte[] gsmdata = new byte[FRAME_SIZE];
				numBytesRead = curCallLink.getInputStream().read(gsmdata);
				if (numBytesRead == -1) {
					Log.e(TAG, "startPhoneSPK exit!");
					break;
				}			
				if(numBytesRead > 0){
				    mRecordQueue.add(gsmdata);
				}
			}
		} catch (Exception e) {
			Log.e("phone", e.getMessage(), e);
		}
	}
	private ConcurrentLinkedQueue<byte[]> mRecordQueue = new ConcurrentLinkedQueue<byte[]>();
	private int alaw_to_s16(byte a_val) {
		int		t;
	    int		seg;
	    
	    a_val ^= 0x55;
	    t = a_val & 0x7f;
	    if (t < 16)
	        t = (t << 4) + 8;
	    else {
	        seg = (t >> 4) & 0x07;
	        t = ((t & 0x0f) << 4) + 0x108;
	        t <<= seg -1;
	    }
	    return (((a_val & 0x80) > 0 )? t : -t);
	}

	public void stopPhone() throws Exception {
		stoped = true;
		//while (inThread.isAlive() || outThread.isAlive()) {
		while (outThread.isAlive()) {
			Thread.sleep(100);
		}
		while (inThread.isAlive()) {
			Thread.sleep(100);
		}
		if(null != phoneMIC){
			phoneMIC.stop();
			phoneMIC.release();
			phoneMIC = null;
		}
		if(null != phoneSPK){
			phoneSPK.stop();
			phoneSPK.release();
			phoneSPK = null;
		}	
		//phoneSPK.stop();
		if(null != curCallLink){
			curCallLink.close();
			curCallLink = null;
		}
		
	}

}
