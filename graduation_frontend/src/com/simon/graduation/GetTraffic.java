package com.simon.graduation;

import java.io.IOException;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;

import org.postgresql.ds.PGPoolingDataSource;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.ArrayNode;
import com.fasterxml.jackson.databind.node.ObjectNode;

public class GetTraffic {
	public static PGPoolingDataSource pgpool = null;
	Connection conn = null;
    PreparedStatement preparedStatement = null;
    ResultSet resultSet = null;
    private static GetTraffic ins = null;
    
    public static GetTraffic getInstance() {
    	if(ins == null) {
    		ins = new GetTraffic();
    	}
    	return ins;
    }

	private GetTraffic() {
		init();
	}
	
	public String setOneSpeed(String roadid, String speed) {
		String sql = "UPDATE Wuhan_Road SET avgspeed = " + speed
				+ " WHERE id = '" + roadid + "'";
		try {
			conn = pgpool.getConnection();
			preparedStatement = conn.prepareStatement(sql);
			preparedStatement.execute();
			
			return "OK";
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return "Failed";
		}
	}
	
	public String getTraffic(String mapName) {
		String sql = "SELECT id, avgspeed FROM " + mapName + "_Road";
		try {
			conn = pgpool.getConnection();
			preparedStatement = conn.prepareStatement(sql);
			resultSet = preparedStatement.executeQuery();
			
			ObjectMapper mapperResp = new ObjectMapper();
	        ObjectNode rootNodeResp = mapperResp.createObjectNode();
	        ArrayNode arrayNode = rootNodeResp.putArray("contentList");
	        
			while(resultSet.next()) {
				ObjectNode contentNode = arrayNode.addObject();
				contentNode.put("id", resultSet.getString(1));
				contentNode.put("avgspeed", resultSet.getDouble(2));
			}
			rootNodeResp.put("contentListType", "graduation.avgspeed");
			
			return mapperResp.writeValueAsString(rootNodeResp);
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return null;
		} catch (JsonProcessingException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			return null;
		}
	}

	private void init() {
		// Put your code here
		if (pgpool == null) {
			//String DataSourceName = "First Source";
			String ServerName = "192.168.1.7";
			String User = "test";
			String Password = "123456";
			//int MaxConnections = 5;
			String dbInstance = "mapdata";

			pgpool = new PGPoolingDataSource();
			//pgpool.setDataSourceName(DataSourceName);
			pgpool.setServerName(ServerName);
			pgpool.setDatabaseName(dbInstance);
			pgpool.setUser(User);
			pgpool.setPassword(Password);
			//pgpool.setMaxConnections(MaxConnections);
			pgpool.setPortNumber(5432);
		}
	}

	public void destroy() {
		pgpool.close();
	}
	
	public static void main(String[] args) throws IOException {
		System.out.println(GetTraffic.getInstance().getTraffic("Wuhan"));
		GetTraffic.getInstance().destroy();
	}
}
