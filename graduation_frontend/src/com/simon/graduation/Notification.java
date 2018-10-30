package com.simon.graduation;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;

public class Notification extends Thread {
	
	final private int TRAFFICJAM = 1;
	
	private String ipAddr;
	private int port;
	private Socket sock;
	private PrintWriter out;
	
	public Notification(String _ipAddr, int _port) {
		ipAddr = _ipAddr;
		port = _port;
		
		sockInit();
		IOInit();
		
		System.out.println("Notification subscrib init OK!");
	}
	
	public void run() {
		BlockRecive blockRecive = new BlockRecive(sock);
		blockRecive.start();
		
		registSelf();
		try {
			for( ; true; sleep(5000)) {
				keepAlive();
			}
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	private void sockInit() {
		InetAddress address;
		try {
			address = InetAddress.getByName(ipAddr);
			sock = new Socket(address, port);
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}			
	}
	
	private void IOInit() {
		try {
			out = new PrintWriter(sock.getOutputStream());
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	private void registSelf() {
		//System.out.println("REGIST" + TRAFFICJAM);
		out.println("REGIST" + TRAFFICJAM);
		out.flush();
	}
	
	private void keepAlive() {
		//System.out.println("Keep alive");
		out.println("KEEPALIVE");
		out.flush();
	}
	
	private class BlockRecive extends Thread {

		private InputStream inputStream;
		private BufferedReader in;
		
		public BlockRecive(Socket sock) {
			try {
				inputStream = sock.getInputStream();
				in = new BufferedReader(new InputStreamReader(inputStream));
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		
		public void run() {
			while(true) {
				try {
					handleReply(in.readLine());
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}
		
		private void handleReply(String reply) {
			System.out.println(reply);
			if(reply.startsWith("Regist")) return ;
			
			String[] rep = reply.split(",");
			//long tp = Long.valueOf(rep[1].substring(4));
			//SimpleDateFormat format = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
			//Date Tp = new Date(tp);
			//Date cur = new Date(System.currentTimeMillis());
			//System.out.println("duration: " + String.valueOf(System.currentTimeMillis() - tp));
			//System.out.println(format.format(Tp) + " -> " + format.format(cur));
			int evType;
			if(rep[0].startsWith("Notify")) {
				evType = Integer.valueOf(rep[0].substring(8));
				if((evType & TRAFFICJAM) != 0) {
					System.out.println("Notification: Traffic jam.");
					NotifyClientList.getInstance().sendMessage("Notification:TrafficJam");
				}
			}
		}
	}

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub
		Notification noti = new Notification("192.168.1.7", 8931);
		noti.run();
	}

}
