package com.simon.graduation;

import org.eclipse.jetty.websocket.servlet.WebSocketServlet;
import org.eclipse.jetty.websocket.servlet.WebSocketServletFactory;

public class NotificationServlet extends WebSocketServlet {

	/**
	 * 
	 */
	private static final long serialVersionUID = 1L;

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		// TODO Auto-generated method stub

	}

	@Override
	public void configure(WebSocketServletFactory factory) {
		// TODO Auto-generated method stub
		//factory.getPolicy().setIdleTimeout(10000);
		factory.register(NotifySocket.class);
	}

}
