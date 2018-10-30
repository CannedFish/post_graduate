package com.simon.graduation;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.TimeUnit;

import javax.servlet.AsyncContext;
import javax.servlet.ServletContextEvent;
import javax.servlet.ServletContextListener;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

public class ServletWork implements ServletContextListener {
	
	private static final BlockingQueue<AsyncContext> queue = new LinkedBlockingQueue<AsyncContext>();
	private volatile Thread thread;
	
	public static void add(AsyncContext ac) {
		queue.add(ac);
	}

	@Override
	public void contextDestroyed(ServletContextEvent arg0) {
		// TODO Auto-generated method stub
		thread.interrupt();
	}

	@Override
	public void contextInitialized(ServletContextEvent arg0) {
		// TODO Auto-generated method stub
		thread = new Thread(new Runnable() {

			private AsyncContext context = null;
			private GetRoute getRoute = GetRoute.getInstance();
			private GetTraffic getTraffic = GetTraffic.getInstance();
			
			@Override
			public void run() {
				// TODO Auto-generated method stub
				while(true) {
					try {
						while((context = queue.poll(2, TimeUnit.SECONDS)) != null) {
							HttpServletRequest request = (HttpServletRequest)context.getRequest();
							HttpServletResponse response = (HttpServletResponse)context.getResponse();

							try {
								response.setContentType("application/json;charset=utf-8");
								PrintWriter out = response.getWriter();
								//String requsetURL = request.getRequestURI();
								String requestPath = request.getPathInfo();
								//System.out.println(requestPath);
								
								if(requestPath.startsWith("/Graduation/data/getroute")) {
									InputStream in = request.getInputStream();
									BufferedReader reader = new BufferedReader(new InputStreamReader(in));
									String postReqData = reader.readLine();
									System.out.println(postReqData);
						    	  
									ObjectMapper mapperReq = new ObjectMapper();
									JsonNode rootNodeReq = mapperReq.readTree(postReqData);
									JsonNode subNode = rootNodeReq.get("content");
									
									String type = subNode.get("type").asText();
									String mapName = subNode.get("mapname").asText();
									String x1 = subNode.get("x1").asText();
									String y1 = subNode.get("y1").asText();
									String x2 = subNode.get("x2").asText();
									String y2 = subNode.get("y2").asText();
									//System.out.println(mapName + " " + x1 + " " + y1 + " " + x2 + " " + y2);
									/*
									ObjectMapper mapperResp = new ObjectMapper();
									ObjectNode rootNodeResp = mapperResp.createObjectNode();
						          
									rootNodeResp.put("contentListType", "graduation.route");
									out.write(mapperResp.writeValueAsString(rootNodeResp));
									*/
									//out.write(getRoute.getOneRoute("00 " + mapName + " " + x1 + " " + y1 + " " + x2 + " " + y2));
									//out.flush();
									getRoute.getRoute(type + " " + mapName + " " + x1 + " " + y1 + " " + x2 + " " + y2, response);
									try {
										Thread.sleep(200);
									} catch (InterruptedException e) {
										// TODO Auto-generated catch block
										e.printStackTrace();
									}
									System.out.println("Response1: " + response.toString());
								} else if(requestPath.startsWith("/Graduation/data/gettraffic")) {
									InputStream in = request.getInputStream();
									BufferedReader reader = new BufferedReader(new InputStreamReader(in));
									String postReqData = reader.readLine();
									System.out.println(postReqData);
									
									ObjectMapper mapper = new ObjectMapper();
									JsonNode rootNode = mapper.readTree(postReqData);
									JsonNode contentNode = rootNode.get("content");
									String mapName = contentNode.get("mapname").asText();
									
									out.write(getTraffic.getTraffic(mapName));
									out.flush();
									out.close();
								} else if(requestPath.startsWith("/Graduation/data/change")) {
									InputStream in = request.getInputStream();
									BufferedReader reader = new BufferedReader(new InputStreamReader(in));
									String postReqData = reader.readLine();
									System.out.println(postReqData);
						    	  
									ObjectMapper mapperReq = new ObjectMapper();
									JsonNode rootNodeReq = mapperReq.readTree(postReqData);
									String roadid = rootNodeReq.get("id").asText();
									String speed = rootNodeReq.get("speed").asText();
									
									out.write(getTraffic.setOneSpeed(roadid, speed));
									out.flush();
									out.close();
								}
							} catch(IOException e) {
								e.printStackTrace();
							} finally {
								context.complete();
							}
						}
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
					//this.wait(2000);
					//Thread.sleep(200);
				}
				
			}
			
		});
		
		thread.start();
		
	}

}
