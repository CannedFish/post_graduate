package com.simon.graduation;

import java.io.IOException;
import java.io.PrintWriter;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.SocketChannel;
import java.nio.charset.CharacterCodingException;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map.Entry;
import java.util.Set;

import javax.servlet.http.HttpServletResponse;

public class ConnPool implements Runnable {

	private class ConnPoolClient {
		public boolean inUse;
		public String content;
		public HttpServletResponse response;
	}

	final private int POOLSIZE = 20;
	final private Selector selector;
	final private SocketAddress server;
	HashMap<SocketChannel, ConnPoolClient> clientList = new HashMap<SocketChannel, ConnPoolClient>();
	
	private Connector connectHandler = new Connector();
	private WriteHandler writeHandler = new WriteHandler();
	private ReadHandler readHandler = new ReadHandler();
	
	ConnPool(String dstAddr_, int port_) throws IOException {
		selector = Selector.open();
		server = new InetSocketAddress(dstAddr_, port_);
		
		initClinetList();
		new Thread(new HeartBeat()).start();
	}
	
	private void initClinetList() throws IOException {
		for(int i = 0; i < POOLSIZE; ++i) {
			ConnPoolClient cpClient = new ConnPoolClient();
			SocketChannel client = SocketChannel.open();
			client.configureBlocking(false);
			client.connect(server);
			client.register(selector, SelectionKey.OP_CONNECT, connectHandler);
			cpClient.inUse = true;
			clientList.put(client, cpClient);
		}
	}
	
	public void sendReq(String content_, HttpServletResponse response_) throws InterruptedException, ClosedChannelException {
		SocketChannel cnClient;
		for( ; true; Thread.sleep(1000)) {
			if((cnClient = getAnIdleClient()) != null) break;
		}
		
		clientList.get(cnClient).content = content_;
		clientList.get(cnClient).inUse = true;
		clientList.get(cnClient).response = response_;
		cnClient.register(selector, SelectionKey.OP_WRITE, writeHandler);
		System.out.println("Send a requset");
	}
	
	private SocketChannel getAnIdleClient() {
		Iterator<Entry<SocketChannel, ConnPoolClient>> it_map = clientList.entrySet().iterator();
		while(it_map.hasNext()) {
			Entry<SocketChannel, ConnPoolClient> tmp = it_map.next();
			if(!tmp.getValue().inUse)
				return tmp.getKey();
		}
		return null;
	}
	
	private void dispatch(SelectionKey key) {
		SocketChannel client = (SocketChannel)key.channel();
		if(key.isConnectable()) {
			Connector con = (Connector)(key.attachment());
			con.set(clientList.get(client), client);
			con.run();
		}
		if(key.isReadable()) {
			ReadHandler con = (ReadHandler)(key.attachment());
			con.set(clientList.get(client), client);
			con.run();
		}
		if(key.isWritable()) {
			WriteHandler con = (WriteHandler)(key.attachment());
			con.set(clientList.get(client), client);
			con.run();
		}
	}

	@Override
	public void run() {
		// TODO Auto-generated method stub
		try {
			while(!Thread.interrupted()) {
				int ret = selector.select(1000);
				if(ret > 0) {
					Set<SelectionKey> keys = selector.selectedKeys();
					Iterator<SelectionKey> itr = keys.iterator();
					//int x = 0;
					while(itr.hasNext()) {
						dispatch(itr.next());
						//x++;
					}
					//System.out.println(x);
					keys.clear();
				}
			}
		} catch(IOException e) {
			e.printStackTrace();
		}
	}
	
	class HeartBeat implements Runnable {

		@Override
		public void run() {
			// TODO Auto-generated method stub
			ByteBuffer buf = ByteBuffer.wrap(new String("HeartBeat\n").getBytes());
			while(!Thread.interrupted()) {
				try {
					Thread.sleep(5000);
					Iterator<Entry<SocketChannel, ConnPoolClient>> it_map = clientList.entrySet().iterator();
					while(it_map.hasNext()) {
						SocketChannel client = it_map.next().getKey();
						if(client.isConnected()) {
							try {
								client.write(buf);
								buf.position(0);
							} catch (IOException e) {
								// TODO Auto-generated catch block
								e.printStackTrace();
							}
						}
					}
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}
		
	}
	
	class Connector implements Runnable {
		
		ConnPoolClient data;
		SocketChannel client;
		
		public Connector() {
			data = null;
			client = null;
		}
		
		public Connector(ConnPoolClient data_, SocketChannel client_) {
			data = data_;
			client = client_;
		}
		
		public void set(ConnPoolClient data_, SocketChannel client_) {
			data = data_;
			client = client_;
		}

		@Override
		public void run() {
			// TODO Auto-generated method stub
			if(client.isConnectionPending()) {
				try {
					client.finishConnect();
					data.inUse = false;
					//System.out.println("Connection OK!!");
					//data.content = "Hello";
					//client.register(selector, SelectionKey.OP_WRITE, new WriteHandler());
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		}
		
	}
	
	class WriteHandler implements Runnable {
		
		ConnPoolClient data;
		SocketChannel client;
		
		public WriteHandler() {
			data = null;
			client = null;
		}
		
		public WriteHandler(ConnPoolClient data_, SocketChannel client_) {
			data = data_;
			client = client_;
		}
		
		public void set(ConnPoolClient data_, SocketChannel client_) {
			data = data_;
			client = client_;
		}

		@Override
		public void run() {
			// TODO Auto-generated method stub
			try {
				int ret = client.write(ByteBuffer.wrap(data.content.getBytes()));
				if(ret > 0) {
					client.register(selector, SelectionKey.OP_READ, readHandler);
				} else {
					System.out.println("Send error!!");
				}
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		
	}
	
	class ReadHandler implements Runnable {
		
		ConnPoolClient data;
		SocketChannel client;
		ByteBuffer buf = ByteBuffer.allocate(1024);
		
		public ReadHandler() {
			data = null;
			client = null;
		}
		
		public ReadHandler(ConnPoolClient data_, SocketChannel client_) {
			data = data_;
			client = client_;
		}
		
		public void set(ConnPoolClient data_, SocketChannel client_) {
			data = data_;
			client = client_;
		}
		
		private String byteBuffer2String(ByteBuffer b) {
			try {
				CharBuffer charBuffer;
				Charset charset = Charset.forName("UTF-8");
				CharsetDecoder decoder = charset.newDecoder();
				charBuffer = decoder.decode(b.asReadOnlyBuffer());
				return charBuffer.toString();
			} catch (CharacterCodingException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				return null;
			}
		}
		
		@Override
		public void run() {
			// TODO Auto-generated method stub
			try {
				buf.clear();
				int ret = client.read(buf);
				//System.out.println(ret + "Bytes: " + byteBuffer2String((ByteBuffer)buf.flip()));
				if(ret > 0) {
					System.out.println("Response2: " + data.response.toString());
					PrintWriter out = data.response.getWriter();
					out.write(byteBuffer2String((ByteBuffer)buf.flip()));
					out.flush();
					out.close();
				} else {
					System.out.println("Read error!!");
				}
				clientList.get(client).inUse = false;
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}
}
