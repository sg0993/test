// AEC Simple File I/O Test


#include <android/log.h>
#include <stdio.h>
#include <jni.h>
#include <common/xdm_packages/ti/xdais/std.h>
#include <common/xdm_packages/ti/xdais/xdas.h>

#include <common/include/adt_typedef_user.h>
#include <AECG4/include/iaecg4.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

#define FRAME_SIZE 80
#define SAMPLING_RATE 8000

FILE *RxInFile,*RxOutFile,*TxInFile,*TxOutFile;
IAECG4_Handle hGAEC;

IAECG4_Params DefaultAECParams = 
{
	// Base Parameters
	sizeof(IAECG4_Params),
	0,				//LockCallback_t
	FRAME_SIZE,		//Frame Size Samples 
	0,		        // AntiHow
	SAMPLING_RATE,	// ADT_Int32 SamplingRate;
	SAMPLING_RATE/2,	//maxAudioFreq
	0*FRAME_SIZE,		// Bulk Delay Samples 1*FRAME_SIZE,//1*FRAME_SIZE for simulated echo, 2*FRAME_SIZE for typical realtime environment
					// Seems to be 3*FRAME_SIZE for C6747 EVM demo when using EVM RxOut as RxIn.
					//             4*FRAME_SIZE for C6747 EVM demo when using EVM RxIn as RxIn.
	0,	//TailSearchSamples
	0,	//InitialBulkDelay
	64,			// ADT_Int16 ActiveTailLengthMSec
	64,	//ADT_Int16 TotalTailLengthMSec
	6,	//ADT_Int16 txNLPAggressiveness
	40, //ADT_Int16 MaxTxLossSTdB;
	10, //ADT_Int16 MaxTxLossDTdB;
	0,	// 12, //ADT_Int16 MaxRxLossdB;
	0,	//InitialRxOutAttendB
	-85,	// ADT_Int16 TargetResidualLeveldBm;
	-90,	//-60,	// ADT_Int16 MaxRxNoiseLeveldBm;
	-18,		// ADT_Int16 worstExpectedERLdB
	3,		//RxSaturateLeveldBm
	1,		// ADT_Int16 NoiseReduction1Setting
	0,		// ADT_Int16 NoiseReduction2Setting
	1,		//CNGEnable
	0,		//fixedGaindB10
// TxAGC Parameters
	0,		// ADT_Int8 AGCEnable;
	10,		// ADT_Int8 AGCMaxGaindB; 
	-6,		//ADT_Int8 AGCMaxLossdB; 
	-10,	// ADT_Int8 AGCTargetLeveldBm;
	-36,	//ADT_Int8 AGCLowSigThreshdBm;
// RxAGC Parameters
	0,		// ADT_Int8 AGCEnable;
	10,		// ADT_Int8 AGCMaxGaindB; 
	10,		//ADT_Int8 AGCMaxLossdB; 
	-10,	// ADT_Int8 AGCTargetLeveldBm;
	-40,	//ADT_Int8 AGCLowSigThreshdBm;
	0,		//RxBypass
	0000,	//ADT_Int16 maxTrainingTimeMSec,
	0,		//ADT_Int16 TrainingRxNoiseLeveldBm
	0,	//ADT_Int8 *pTxEqualizerdB10; 
	0,	//MIPSMemReductionSetting
	0,	//MIPSReductionSetting2
	0   // reserved
};
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jshortArray JNICALL Java_com_audio_record_AudioRecordActivity_processTx
  (JNIEnv * pEnv, jclass jClass, jshortArray dataIn,jint size){
	int len = size;
	__android_log_print(ANDROID_LOG_INFO, "ADT HDAEC TEST",
				" processTx size = %d 11111111111\n", size);
	short *RxOut = new short[len];
//	IAECG4_Params MyParams;
	IAECG4_Status Status;
	ADT_Int16 BestShortTermERLEdB=0;
	int TotalSampleCount =  0;
	jshort*dataArray;
	jboolean copy = JNI_FALSE;
	__android_log_print(ANDROID_LOG_INFO,"ADT HDAEC TEST", "processTx 33333\n");

	dataArray = pEnv->GetShortArrayElements(dataIn,&copy);
	__android_log_print(ANDROID_LOG_INFO, "ADT HDAEC TEST"," processTx dataArray[0] = %ld 44444\n", dataArray[0]);

	int time = 0;
	if (len <= FRAME_SIZE) {
		time = 1;
	} else if (len % FRAME_SIZE == 0) {
		time = len / FRAME_SIZE;
	} else {
		time = len / FRAME_SIZE + 1;
	}
	int position = 0;
	for (int i = 0; i < time; i++) {
		short data[FRAME_SIZE] = { 0 };
		short dataOut[FRAME_SIZE] = { 0 };
		int copySize = 0;
		if ((position + FRAME_SIZE) <= len) {
			memcpy(data, dataArray + position, FRAME_SIZE);
			position += FRAME_SIZE;
			copySize = FRAME_SIZE;
		} else {
			copySize = len - position;
			memcpy(data, dataArray + position, copySize);
			position += copySize;
		}
		AECG4_ADT_applyTx(hGAEC, data, dataOut);
		memcpy(RxOut + position - copySize, dataOut, copySize);
		/*AECG4_ADT_applyRx(hGAEC, data, dataOut);
		 memcpy(RxOut+position-copySize,data,copySize);*/
	}


	AECG4_ADT_control(hGAEC, IAECG4_GETSTATUS, &Status);
	if (Status.shortTermERLEdB10 / 10 > BestShortTermERLEdB)
		BestShortTermERLEdB = Status.shortTermERLEdB10 / 10;

	pEnv->ReleaseShortArrayElements(dataIn, dataArray, 0);
	jshortArray dataOut = pEnv->NewShortArray(len);
	pEnv->SetShortArrayRegion(dataOut, 0, len,RxOut);
	__android_log_print(ANDROID_LOG_INFO, "ADT HDAEC TEST"," RxOut= %d 555555555\n", RxOut[0]);
	delete RxOut;
	return dataOut;
}

JNIEXPORT jshortArray JNICALL Java_com_audio_record_AudioRecordActivity_processRx
  (JNIEnv * pEnv, jclass jClass, jshortArray dataIn,jint size){
	int len = size;
	__android_log_print(ANDROID_LOG_INFO, "ADT HDAEC TEST"," processRx size = %d 11111111111\n", size);
	short *RxOut = new short[len];
	IAECG4_Status Status;
	ADT_Int16 BestShortTermERLEdB=0;
	int TotalSampleCount =  0;
	jshort*dataArray;
	jboolean copy = JNI_FALSE;
	__android_log_print(ANDROID_LOG_INFO,"ADT HDAEC TEST", "processRx 33333\n");

	dataArray = pEnv->GetShortArrayElements(dataIn,&copy);
	__android_log_print(ANDROID_LOG_INFO, "ADT HDAEC TEST"," processRx dataArray[0] = %ld 44444\n", dataArray[0]);

	int time = 0;
	if(len <= FRAME_SIZE){
		time = 1;
	}
	else if(len % FRAME_SIZE == 0){
		time = len/FRAME_SIZE;
	}
	else{
		time = len/FRAME_SIZE + 1;
	}
	int position = 0;
	for(int i = 0 ;i < time; i++){
		short data[FRAME_SIZE] = {0};
		short dataOut[FRAME_SIZE] = {0};
		int copySize = 0;
		if((position + FRAME_SIZE) <= len ){
			memcpy(data,dataArray + position,FRAME_SIZE);
			position += FRAME_SIZE;
			copySize = FRAME_SIZE;
		}
		else{
			copySize = len - position;
			memcpy(data,dataArray + position,copySize);
			position += copySize;
		}
		AECG4_ADT_applyRx(hGAEC, data, dataOut);
		memcpy(RxOut+position-copySize,dataOut,copySize);
		/*AECG4_ADT_applyRx(hGAEC, data, dataOut);
		memcpy(RxOut+position-copySize,data,copySize);*/
	}


	AECG4_ADT_control(hGAEC, IAECG4_GETSTATUS, &Status);
	if (Status.shortTermERLEdB10 / 10 > BestShortTermERLEdB)
		BestShortTermERLEdB = Status.shortTermERLEdB10 / 10;

	pEnv->ReleaseShortArrayElements(dataIn, dataArray, 0);
	jshortArray dataOut = pEnv->NewShortArray(len);
	pEnv->SetShortArrayRegion(dataOut, 0, len,RxOut);
	__android_log_print(ANDROID_LOG_INFO, "ADT HDAEC TEST"," RxOut= %d 555555555\n", RxOut[0]);
	delete RxOut;
	return dataOut;
}

//com.audio.record.AudioRecordActivity
void Java_com_audio_record_AudioRecordActivity_main(void)
{
	IAECG4_Params MyParams;
	memcpy(&MyParams,&DefaultAECParams,sizeof(MyParams));
		MyParams.size = sizeof(IAECG4_Params);
		if(0 == hGAEC){
			hGAEC = (IAECG4_Handle) AECG4_ADT_create(0, &MyParams);
		}
		if(hGAEC == 0)
			{
				__android_log_print(ANDROID_LOG_INFO,"ADT HDAEC TEST", "processTx AEC Allocation failed 2222\n");
				exit(0);
			}
	/*IAECG4_Handle hAEC;
	IAECG4_Params MyParams;
	IAECG4_Status Status;
	int TotalSampleCount =  0;

	static short int RxIn[FRAME_SIZE],TxIn[FRAME_SIZE],RxOut[FRAME_SIZE],TxOut[FRAME_SIZE];
	ADT_Int16 BestShortTermERLEdB=0;

	RxInFile = fopen("/sdcard/rxin.pcm", "rb");
	TxInFile = fopen("/sdcard/txin.pcm", "rb");
	RxOutFile = fopen("/sdcard/rxout.pcm", "wb");
	TxOutFile = fopen("/sdcard/txout.pcm", "wb");


	memcpy(&MyParams,&DefaultAECParams,sizeof(MyParams));
	MyParams.size = sizeof(IAECG4_Params);
	hAEC = (IAECG4_Handle) AECG4_ADT_create(0, &MyParams);
	if (hAEC == 0)
	{
		__android_log_print(ANDROID_LOG_INFO,"ADT HDAEC TEST", "AEC Allocation failed\n");
		exit(0);
	}
	while (!feof(RxInFile) && !feof(TxInFile))
	{
		if(fread(RxIn, sizeof(short int), FRAME_SIZE,RxInFile) != FRAME_SIZE)
		   break;
		if(fread(TxIn,sizeof(short int), FRAME_SIZE,TxInFile) != FRAME_SIZE)
		   break;
		if (feof(RxInFile) && feof(TxInFile))
			break;
#ifndef SPLIT_API
			AECG4_ADT_apply(hAEC,RxIn,RxOut,TxIn,TxOut);
#else
			AECG4_ADT_applyTx(hGAEC, TxIn, TxOut);
			AECG4_ADT_applyRx(hGAEC, RxIn, RxOut);
#endif
		fwrite(TxOut, sizeof(short int), FRAME_SIZE, TxOutFile);
		fwrite(RxOut, sizeof(short int), FRAME_SIZE, RxOutFile);

		AECG4_ADT_control(hAEC,IAECG4_GETSTATUS, &Status);
		if (Status.shortTermERLEdB10/10 > BestShortTermERLEdB)
			BestShortTermERLEdB = Status.shortTermERLEdB10/10;

		TotalSampleCount += FRAME_SIZE;
	}
	fclose(RxInFile);
	fclose(RxOutFile);
	fclose(TxInFile);
	fclose(TxOutFile);
	AECG4_ADT_control(hAEC,IAECG4_GETSTATUS,  &Status);

	AECG4_ADT_delete(hAEC);
	__android_log_print(ANDROID_LOG_INFO,"ADT HDAEC TEST", "  Total samples processed = %ld\n",TotalSampleCount);
	__android_log_print(ANDROID_LOG_INFO,"ADT HDAEC TEST", "  Best Short Term ERLE = %d\n", BestShortTermERLEdB);*/

}
#ifdef __cplusplus
}
#endif
