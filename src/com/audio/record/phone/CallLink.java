package com.audio.record.phone;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;

public class CallLink {

	final int TALK_PORT = 22222;

	String ipAddr = null;
	Socket outSock = null;
//	ServerSocket inServSock = null;
//	Socket inSock = null;

	CallLink(String inIP) {
		ipAddr = inIP;
	}

	void open() throws IOException, UnknownHostException {// ����·����
		if (ipAddr != null)
			outSock = new Socket(ipAddr, TALK_PORT);
	}

	void listen() throws IOException {// ����,�Ⱥ����
		/*inServSock = new ServerSocket(TALK_PORT);
		inSock = inServSock.accept();*/
	}

	public InputStream getInputStream() throws IOException {// ������Ƶ����������
		if (outSock != null)
			return outSock.getInputStream();
		else
			return null;
	}

	public OutputStream getOutputStream() throws IOException {// ������Ƶ���������
		if (outSock != null)
			return outSock.getOutputStream();
		else
			return null;
	}

	void close() throws IOException {// �ر���������
		/*if(null != inSock){
			inSock.close();
			inSock = null;
		}*/
		if(null != outSock){
			outSock.close();
			outSock = null;
		}		
	}

}
