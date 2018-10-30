package com.simon.graduation;

import java.io.IOException;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;

import org.eclipse.jetty.websocket.api.Session;

public class NotifyClientList {
	/**/
	private static NotifyClientList ins = new NotifyClientList();
	private HashMap<Integer, Session> sessionList = new HashMap<Integer, Session>();
	
	private NotifyClientList() {}
	
	public static NotifyClientList getInstance() {
		return ins;
	}
	
	public void registSession(Session session) {
		//System.out.println("a");
		sessionList.put(session.hashCode(), session);
		//System.out.println("b");
	}
	
	public void unRegistSession(Session session) {
		sessionList.remove(session.hashCode());
	}
	
	public void sendMessage(String msg) {
		Iterator<Entry<Integer, Session>> mapItor = sessionList.entrySet().iterator();
		while(mapItor.hasNext()) {
			Session tmp = mapItor.next().getValue();
			
			if(!tmp.isOpen()) {
				System.out.println("Session " + tmp.getRemoteAddress().toString() + " is not open");
				continue;
			}
			
			try {
				tmp.getRemote().sendString(msg);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}			
		}
	}

}
