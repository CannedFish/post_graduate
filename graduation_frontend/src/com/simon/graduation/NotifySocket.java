package com.simon.graduation;

import org.eclipse.jetty.websocket.api.Session;
import org.eclipse.jetty.websocket.api.WebSocketListener;

public class NotifySocket implements WebSocketListener {

	private Session outbound;
	
	@Override
	public void onWebSocketBinary(byte[] payload, int offset, int len) {
		// TODO Auto-generated method stub
		System.out.println("Binary from romote peer");
	}

	@Override
	public void onWebSocketClose(int statusCode, String reason) {
		// TODO Auto-generated method stub
		System.out.println("WebSocket is closed, because " + reason);
		NotifyClientList.getInstance().unRegistSession(this.outbound);
		this.outbound = null;
	}

	@Override
	public void onWebSocketConnect(Session session) {
		// TODO Auto-generated method stub
		System.out.println("WebSocket is connected");
		this.outbound = session;
		//System.out.println("a");
		//if(NotifyClientList.getInstance() != null)
			NotifyClientList.getInstance().registSession(this.outbound);
		//System.out.println("b");
	}

	@Override
	public void onWebSocketError(Throwable arg0) {
		// TODO Auto-generated method stub
		arg0.printStackTrace();
	}

	@Override
	public void onWebSocketText(String arg0) {
		// TODO Auto-generated method stub
		System.out.println("Text from romote peer: " + arg0);
	}
	
}
