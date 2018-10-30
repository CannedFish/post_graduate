package com.simon.graduation;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.nio.channels.ClosedChannelException;

import javax.servlet.http.HttpServletResponse;

public class GetRoute {
	//private map connPool;
	private static GetRoute ins = new GetRoute();

	private Socket sock;
	private PrintWriter out;
	private InputStream inputStream;
	private BufferedReader in;
	private String serverIP = "192.168.1.7";
	private int serverPort = 8930;
	private ConnPool connPool = null;
	
	private GetRoute() {
		try {
			connPool = new ConnPool(serverIP, serverPort);
			new Thread(connPool).start();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public static GetRoute getInstance()  {
		return ins;
	}
	
	public String getOneRoute(String content) {
		return send(content);
	}
	
	public void getRoute(String content, HttpServletResponse response) {
		try {
			connPool.sendReq(content, response);
		} catch (ClosedChannelException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	private void sockInit() {
		InetAddress address;
		try {
			address = InetAddress.getByName(serverIP);
			sock = new Socket(address, serverPort);
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
			inputStream = sock.getInputStream();
			in = new BufferedReader(new InputStreamReader(inputStream));
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	private String send(String content) {
		sockInit();
		IOInit();
		//log.debug(content);
		out.println(content);		
		out.flush();
		
		//String tmp = null;
		StringBuffer sb = new StringBuffer();
        
		try {
			/*while( (tmp = in.readLine()) != null ){
				System.out.println(tmp);
				sb.append(tmp); 
				sb.append("\r\n");
			}*/
			sb.append(in.readLine());
			
			return sb.toString();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return null;
		}
		
	}
	
	public void Destroy() {
		try {
			in.close();
			inputStream.close();
			out.close();
			sock.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	public static void main(String[] args) throws IOException {
		GetRoute getRoute = GetRoute.getInstance();
		//System.out.println(getRoute.getOneRoute("00 Wuhan 12730409.02 3576475.82 12731280.65 3575972.92"));
		//getRoute.Destroy();
		System.out.println("haha");
		getRoute.getRoute("00 Wuhan 12730409.02 3576475.82 12731280.65 3575972.92", null);
	}
}
