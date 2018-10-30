
//configuration
var URL = "/FCDDemo/data/average_speed";//url to get arerage_speed
var IP = "localhost";
var PORT = "8080";
var VERSION = "1.0.0";
var FEATURE_TYPE = "planet_osm_roads";
var FEATURE_PREFIX = "myTest";//workspace
var FEATURE_NS = "http://localhost:8080/geoserver";
var SRS_NAME = "EPSG:900913";
var GEOMETRY_NAME = "way";

var _map; 
var _wmsLayer;
var juncLayer;
var roadLayer;
var trafficLayer;
var simulateLayer;

var _getAvgSpeedTimer;
var _curZoom;
var _baseURL = "/Graduation"
	
var _filter = ["motorway", "motorway_link", "trunk", "trunk_link", "primary", "primary_link", "secondary", "secondary_link", "tertiary", "residential", "unclassified", "service", ""];

var _filterLayers = [
	[_filter[0], _filter[1]]
	, [_filter[2], _filter[3]]
	, [_filter[4], _filter[5]]
	, [_filter[6], _filter[7]]
	, [_filter[8], _filter[9], _filter[10], _filter[11], _filter[12]]
];

var juncFeatures = undefined;
var juncData = undefined;
var roadFeatures = undefined;
var roadData = undefined;

var prePoint = null;
var coverView = undefined;
var routeIds = undefined;
var simuStart = null;
var simuIcon = undefined;
var simuObj = undefined;
var marker = undefined;
var mannerList = undefined;
var rpSwitcher = true;

var wsObj = undefined;
var wsURL = "ws://192.168.1.191:7000/Graduation/notify/aha";

//var _FCDDataServer = "http://192.168.1.107:7000/FCDDataServer";
// _FCDAvgSpeedReq = undefined;

function test(){
	console.log("123");
	
	reloadAvgSpeedValue();
}

function init(){
	createMap();
	
	addWMSLayer();
	addJuncLayer();
	addRoadLayer();
	addSimulateLayer();
	
	addMapEvents();
	showMap();
	setupConnection();

	coverView = document.getElementById("cover");
	mannerList = document.getElementById("mannerList");
	$('#openSimu').hide();
	toggleRoutePlanning();
	
	startMainLoop();
}

function startMainLoop() {
	_getAvgSpeedTimer = setInterval(function() {
		//reloadAvgSpeedValue();
		postTraffic("Wuhan");
		wsObj.sendMsg("heartbeat");
	}, 60000);
}

function setupConnection() {
	wsObj = new MyWebSocket(wsURL, {
		onMessage: function(evt) {
			var d = new Date();
			console.log('Receive a message from server: "' + evt.data + '" timestemp: ' + d);
			//handle message
			msgDispatch(evt.data);
		}
	});
	wsObj.open();
}

function createMap(){
	OpenLayers.ProxyHost= "proxy/";
	OpenLayers.Feature.Vector.style['default']['strokeWidth'] = '5';

	_map = new OpenLayers.Map("basicMap", {
	   projection: 'EPSG:4326',	
	   displayProjection: new OpenLayers.Projection("EPSG:4326")
	});
}

function addWMSLayer(){
	_wmsLayer = new OpenLayers.Layer.OSM(
		"Base layer", [
			"http://otile1.mqcdn.com/tiles/1.0.0/osm/${z}/${x}/${y}.png"
			,"http://otile2.mqcdn.com/tiles/1.0.0/osm/${z}/${x}/${y}.png"
			,"http://otile3.mqcdn.com/tiles/1.0.0/osm/${z}/${x}/${y}.png"
			,"http://otile4.mqcdn.com/tiles/1.0.0/osm/${z}/${x}/${y}.png"
		], {
			layers: 'basic',
			isBaseLayer: true
		}
	);
	_map.addLayer(_wmsLayer);
}

function addJuncLayer() {
	var renderer = OpenLayers.Util.getParameters(window.location.href).renderer;
	renderer = (renderer) ? [renderer] : OpenLayers.Layer.Vector.prototype.renderers;
	
	var style_default = new OpenLayers.Style({
			strokeColor: "#CECECE",
			strokeOpacity: 1,
			strokeWidth: 1,
			fillColor: "#F5DEB3",
			fillOpacity: 1,
			pointRadius: 8,
		},
		{
			rules: [
				new OpenLayers.Rule({       //"undefined"
                    filter: new OpenLayers.Filter.Comparison({
                        type: OpenLayers.Filter.Comparison.LIKE,
                        property: "id",// the "foo" feature attribute
                        value: "undefined"
                    }),
                    // if a feature matches the above filter, use this symbolizer
                    symbolizer: {
                        strokeColor: "#FFFF00"
                    }
                }),
				new OpenLayers.Rule({
                    // apply this rule if no others apply
                    elseFilter: true,
                    symbolizer: {
                        label: "${id}",
                        fontSize: 10,
                        labelYOffset: -18,
                    }
                })
			]
		}
	);
	
	var style_select = new OpenLayers.Style({
			strokeWidth: 5,
	});
	
	var styleMap_wfs = new OpenLayers.StyleMap({
		"default": style_default,
		select: style_select
	});

	juncLayer = new OpenLayers.Layer.Vector("junction", {
		styleMap: styleMap_wfs,
		renderers: renderer,
		visibility: false
	});
	
	_map.addLayer(juncLayer);
	//_map.setLayerZIndex(juncLayer, 2000);
	
	getJunctions();
}

function addRoadLayer() {
	var renderer = OpenLayers.Util.getParameters(window.location.href).renderer;
	renderer = (renderer) ? [renderer] : OpenLayers.Layer.Vector.prototype.renderers;
	
	var style_default = new OpenLayers.Style({
			strokeColor: "#CECECE",
			strokeOpacity: 1,
			strokeWidth: 5,
		},
		{
			rules: [
                new OpenLayers.Rule({		//"undefined"
                    filter: new OpenLayers.Filter.Comparison({
                        type: OpenLayers.Filter.Comparison.LIKE,
                        property: "average_speed", // the "foo" feature attribute
                        value: "undefined"
                    }),
                    symbolizer: {
						strokeColor: "#CECECE"
                    }
                }), 
				new OpenLayers.Rule({		//"unknown"
                    filter: new OpenLayers.Filter.Comparison({
                        type: OpenLayers.Filter.Comparison.EQUAL_TO,
                        property: "average_speed", // the "foo" feature attribute
                        value: "-1"
                    }),
                    symbolizer: {
						strokeColor: "#CECECE"
                    }
                }), 
				new OpenLayers.Rule({		
                    filter: new OpenLayers.Filter.Comparison({
                        type: OpenLayers.Filter.Comparison.BETWEEN,
                        property: "average_speed",
                        lowerBoundary: 40,
                        upperBoundary: 120
                    }),
                    symbolizer: {
                        strokeColor: "#00FF00"
                    }
                }), 
				new OpenLayers.Rule({		
                    filter: new OpenLayers.Filter.Comparison({
                        type: OpenLayers.Filter.Comparison.BETWEEN,
                        property: "average_speed",
                        lowerBoundary: 25,
                        upperBoundary: 40
                    }),
                    symbolizer: {
                        //strokeColor: "#2BFD73"
						strokeColor: "#FFFF00"
                    }
                }), 
				new OpenLayers.Rule({		
                    filter: new OpenLayers.Filter.Comparison({
                        type: OpenLayers.Filter.Comparison.BETWEEN,
                        property: "average_speed",
                        lowerBoundary: 10,
                        upperBoundary: 25
                    }),
                    symbolizer: {
                        strokeColor: "#FFBF2E"
                    }
                }), 
				new OpenLayers.Rule({		
                    filter: new OpenLayers.Filter.Comparison({
                        type: OpenLayers.Filter.Comparison.BETWEEN,
                        property: "average_speed",
                        lowerBoundary: 0,
                        upperBoundary: 10
                    }),
                    symbolizer: {
                        strokeColor: "#FF0000"
                    }
                }), 
                new OpenLayers.Rule({   
                    elseFilter: true,
                    symbolizer: {
						strokeColor: "#CECECE",						
                    }
                })
            ]
		}
	);
	
	var style_select = new OpenLayers.Style({
			strokeWidth: 8,
	});
	
	var styleMap_wfs = new OpenLayers.StyleMap({
		"default": style_default,
		select: style_select
	});

	roadLayer = new OpenLayers.Layer.Vector("Road", {
		styleMap: styleMap_wfs,
		renderers: renderer,
		visibility: false
	});
	
	_map.addLayer(roadLayer);

	trafficLayer = new OpenLayers.Layer.Vector("TrafficInfo", {
		styleMap: styleMap_wfs,
		renderers: renderer,
		visibility: false
	});
	
	_map.addLayer(trafficLayer);
	_map.setLayerZIndex(trafficLayer, 1000);
	
	getRoadNet();
}

function showLayer(layer) {
	layer.display(true);
	layer.visibility = true;
	layer.redraw();
}

function hideLayer(layer) {
	layer.display(false);
	layer.visibility = false;
}

function bindLayerCtrl() {
	$('#tmc').click(function () {
		if(!trafficLayer.visibility) {
			showLayer(trafficLayer);
			$('#tmc-des').show("drop");
		} else {
			hideLayer(trafficLayer);
			$('#tmc-des').hide("drop");
		}
	});
	
	$('#rp').click(function() {
		toggleRoutePlanning();
	});
}

function addSimulateLayer() {
	simulateLayer = new OpenLayers.Layer.Markers("Simulation", {
		visibility: false
	});
	_map.addLayer(simulateLayer);
	_map.setLayerZIndex(simulateLayer, 9000);
	
	var size = new OpenLayers.Size(70, 70);
	simuIcon = new OpenLayers.Icon(_baseURL + '/img/simuIcon.png', size);
}

function addMapEvents(){
	bindLayerCtrl();

	roadLayer.events.register("featuresadded", roadLayer, function(e) {  //lmj: 当有新的路段添加到wfsLayer时，同步对应的平均速度
		//reloadAvgSpeedValue();
		console.log("featuresadded");
	});
	/*
	roadLayer.events.register("featureadded", roadLayer, function(e) { 	//lmj: 当有新的路段添加到wfsLayer时，给新路段添加“average_speed”属性
		var wfsObj = e.feature;
		if(typeof(wfsObj.attributes) == "undefined") wfsObj.attributes = {};
		wfsObj.attributes.average_speed = 65;
		//roadLayer.drawFeature(wfsObj);
	});
	*/
	_map.events.register("zoomend", _map, function(e) { 	//lmj: 当有新的路段添加到wfsLayer时，给新路段添加“average_speed”属性
		if( _curZoom != _map.zoom ){
			_curZoom = _map.zoom;
			console.log("zoomend: " + _curZoom);
			/**/if(_curZoom <= 9) {
				//console.log("<= 9");
				//_wfsLayer.filter = _filterLayers[0];
				//removeRoads();
				//addRoads(0);
			} else if(_curZoom <= 13) {
				//console.log("<= 14");
				//_wfsLayer.filter = _filterLayers[1];
				//removeRoads();
				//addRoads(1);
				redrawRoute(3);
			} else if(_curZoom <= 15){
				//_wfsLayer.filter = _filterLayers[2];
				//removeRoads();
				//addRoads(2);
				redrawRoute(5);
			} else if(_curZoom <= 16){
				//_wfsLayer.filter = _filterLayers[3];
				//removeRoads();
				//addRoads(3);
				redrawRoute(7);
			} else if(_curZoom <= 17) {
				redrawRoute(8);
				//_wfsLayer.filter = _filterLayers[4];
				//removeRoads();
				//addRoads(4);
			} else {
				redrawRoute(9);
			}
		}
		console.log("zoomend");
	});
}

function showMap(){
	//var position = new OpenLayers.LonLat(12620305.35,3549685.61).transform( new OpenLayers.Projection("EPSG:900913"), OpenLayers.Projection("EPSG:3857"));
	var position = new OpenLayers.LonLat(12721660.85,3580290.42).transform( new OpenLayers.Projection("EPSG:900913"), OpenLayers.Projection("EPSG:3857"));
	var zoom     = 16;
	
	selectControl = new OpenLayers.Control.SelectFeature(
		[juncLayer, roadLayer, trafficLayer],
		{	
			onSelect: onFeatureSelect, 
			onUnselect: onFeatureUnselect,
			onBeforeSelect: onBeforeFeatrueSelect
		}
	);
	
	_curZoom = zoom;
	
	_map.addControl(new OpenLayers.Control.Navigation({}));
	_map.addControl(new OpenLayers.Control.PanZoomBar({}));
	//_map.addControl(new OpenLayers.Control.LayerSwitcher({}));
	_map.addControl(new OpenLayers.Control.Scale("scale"));
	//_map.addControl(new OpenLayers.Control.Permalink());
	_map.addControl(new OpenLayers.Control.MousePosition({displayProjection: new OpenLayers.Projection("EPSG:900913")}));
	_map.addControl(selectControl);
	
	if( ! _map.getCenter() ){  
		  _map.setCenter(position, zoom ); 
	}
	
	selectControl.activate();
}

function findOneJunc(event) {
	if(event.keyCode == 13) {
		var juncID = document.getElementById("searchbox").value;
		if(juncID == "") return ;
		var targetJunc = juncLayer.getFeaturesByAttribute("id", juncID);
		setJuncCenter(targetJunc[0]);
	}
}

function setJuncCenter(junc) {
    var position = new OpenLayers.LonLat(junc.geometry.x, junc.geometry.y).transform(new OpenLayers.Projection("EPSG:900913"), OpenLayers.Projection("EPSG:3857"));
    _map.setCenter(position);
}

function getJunctions() {
	$.get(_baseURL + "/mapdata/junc.json"
	, function(data, status) {
		if(status == "success") {
			juncData = undefined;
			//data = JSON.parse(data);
			juncData = data.contentList;
			//removeRoads();
			addJuncs();
		}
	});
}

function addJuncs() {
	juncFeatures = [];
	for(var i = 0; i < juncData.length; ++i) {
		juncFeatures.push(
			new OpenLayers.Feature.Vector(
				new OpenLayers.Geometry.fromWKT(juncData[i].coor)
				, {id: juncData[i].juncid}
			)
		)
	}
	juncData = null;
	juncLayer.addFeatures(juncFeatures);
}

function buildRoadData(roadList) {
	for(var i = 0; i < roadList.length; ++i) {
		roadData[roadList[i].roadid] = {};
		roadData[roadList[i].roadid].coors = roadList[i].coors;
	}
	roadList = null;
}

function initTrafficLayer(roadList) {
	var trafficFeatures = [];
	for(var i = 0; i < roadList.length; ++i) {
		trafficFeatures.push(
			new OpenLayers.Feature.Vector(
				new OpenLayers.Geometry.fromWKT(roadList[i].coors)
				, {
					id: roadList[i].roadid,
					average_speed: 40
				}
			)
		);
	}
	trafficLayer.addFeatures(trafficFeatures);
	roadList = null;
}
	
function getRoadNet() {
	$.get(_baseURL + "/mapdata/road.json"
	, function(data, status) {
		if(status == "success") {
			roadData = [];
			//data = JSON.parse(data);
			//roadData = data.contentList;
			buildRoadData(data.contentList);
			initTrafficLayer(data.contentList);
			postTraffic("Wuhan");
			removeRoads();
			//addRoads(2);
			//getARoute();
			
			data = null;
		}
	});
}

function addRoads(level) {
	roadFeatures = [];
	var f;
	for(var i = 0; i < roadData.length; ++i) {
		f = true;
		/*
		for(var j = 0; j <= level; ++j) {
			for(var x = 0; x < _filterLayers[j].length; ++x) {
				if(roadData[i].highway == _filterLayers[j][x]) {
					f = true;
					break;
				}
			}
			if(f) break;
		}
		*/
		if(f) {
			roadFeatures.push(
				new OpenLayers.Feature.Vector(
					new OpenLayers.Geometry.fromWKT(roadData[i].coors)
					, {
						osm_id: roadData[i].roadid, 
						average_speed: 20
					}
				)
			);
		}
	}
	roadLayer.addFeatures(roadFeatures);
}

function redrawRoute(lineWidth) {
	if(roadFeatures == null) return ;
	for(var i = 0; i < roadFeatures.length; ++i) {
		roadFeatures[i].layer.styleMap.styles.default.defaultStyle.strokeWidth = lineWidth;
		roadFeatures[i].layer.styleMap.styles.select.defaultStyle.strokeWidth = lineWidth + 2;
		roadLayer.drawFeature(roadFeatures[i]);
	}
}

function drawARoute(routeList, change, cur) {
	removeRoads(change, cur);
	if(typeof(change) == "undefined") {
		roadFeatures = [];
		routeIds = routeList;
	} else {
		while(routeIds.length != cur + 1)
			routeIds.pop();
		for(var i = 0; i < routeList.length; ++i)
			routeIds.push(routeList[i]);
	}
	
	for(var i = 0; i < routeList.length; ++i) {
		var t = trafficLayer.getFeaturesByAttribute("id", routeList[i]);
		roadFeatures.push(
			new OpenLayers.Feature.Vector(
				new OpenLayers.Geometry.fromWKT(roadData[routeList[i]].coors)
				, {
					osm_id: routeList[i],
					average_speed: t[0].attributes.average_speed
				}	
			)
		);
	}
	roadLayer.addFeatures(roadFeatures);
}

function getARoute() {
	$.get(_baseURL + "/mapdata/route.json"
	, function(data, status) {
		if(status == "success") {
			//data = JSON.parse(data);
			if(data != null && data.contentList.length != 0) {
				drawARoute(data.contentList);
			}
			data = null;
		}
	});
}

function postARoute(type, mapName, x1, y1, x2, y2, change, cur) {
	console.log("Routing manner: " + mannerList.options[type].text);
	$.ajax({
		url: _baseURL + "/data/getroute",
		type: "POST",
		async: true,
		data: JSON.stringify({
			"content": {
				"type": type,
				"mapname": mapName,
				"x1": String(x1),
				"y1": String(y1),
				"x2": String(x2),
				"y2": String(y2)
			},
			"contentType": "graduation.getroute"
		}),
		headers: {
			'Accept': 'application/json'
		},
		success: function(data) {
			//var json = JSON.parse(data);
			if(data != null && data.contentList.length != 0) {
				drawARoute(data.contentList, change, cur);
			} else {
				if(typeof(change) == "undefined") {
					alert("no route");
				}
			}
			data = null;
			if(typeof(change) == "undefined") {
				hiddenCover();
				hideLayer(juncLayer);
				showLayer(simulateLayer);
				$('#openSimu').show();
				$('#tmc-des').show("drop");
			} else {
				simuObj.replan(routeIds);
				simuObj.resume();
			}
		},
		error: function(data) {
			if(typeof(change) == "undefined") {
				alert("Failed!!");
				hiddenCover();
			}
		}
	});
}

function postTraffic(mapName) {
	$.ajax({
		url: _baseURL + "/data/gettraffic",
		type: "POST",
		async: true,
		data: JSON.stringify({
			"content": {
				"mapname": mapName
			},
			"contentType": "graduation.gettraffic"
		}),
		headers: {
			'Accept': 'application/json'
		},
		success: function(data) {
			if(data != null && data.contentList.length != 0) {
				updateTraffic(data.contentList);
			} else {
				alert("No traffic information!!");
			}
			data = null;
		},
		error: function(data) {
			alert("Get traffic information failed!!");
		}
	});
}

function updateTraffic(trafficList) {
	var features = trafficLayer.features;
	var infos = [];
	for(var i = 0; i < trafficList.length; ++i) {
		infos[trafficList[i].id] = trafficList[i].avgspeed;
	}
	for(var i = 0; i < trafficList.length; ++i) {
		features[i].attributes.average_speed = 
		infos[features[i].attributes.id];
		trafficLayer.drawFeature(features[i]);
	}
}
	
function removeRoads(change, cur) {
	if(roadFeatures == null) return ;
	if(typeof(change) != "undefined") {
		var tmp = [];
		while(roadFeatures.length != cur + 1) {
			tmp.push(roadFeatures.pop());
		}
		roadLayer.removeFeatures(tmp);
	} else {
		roadLayer.removeFeatures(roadFeatures);
		roadFeatures = null;
	}
}

function showCover() {
	coverView.className = "showCover";
}

function hiddenCover() {
	coverView.className = "hiddenCover";
}

function onBeforeFeatrueSelect(feature) {
	if(feature.layer.name == "Road") {
		feature.attributes.average_speed = 100;
	}
}

function replanRemainedRoute() {
	simuObj.pause();
	var points = simuObj.getReplanEndPoints();
	//console.log(points);
	postARoute(1
		, "Wuhan"
		, points.start.x
		, points.start.y
		, points.end.x
		, points.end.y
		, true
		, simuObj.getCurRoadNum());
}

function postChangeOneTMC(roadid, speed) {
	$.ajax({
		url: _baseURL + "/data/change",
		type: "POST",
		async: true,
		data: JSON.stringify({
			"id": roadid,
			"speed": speed
		}),
		headers: {
			'Accept': 'application/json'
		},
		success: function(data) {
			if(data != null) {
				console.log(data);
			}
		},
		error: function(data) {
			console.log(data);
		}
	});
}

function onFeatureSelect(feature) {
	if(feature.layer.name == "junction") {
		if(prePoint == null) {
			prePoint = {};
			prePoint.x = feature.geometry.x;
			prePoint.y = feature.geometry.y;
			simuStart = prePoint;
		} else {
			showCover();
			postARoute(mannerList.selectedIndex
				, "Wuhan"
				, prePoint.x
				, prePoint.y
				, feature.geometry.x
				, feature.geometry.y);
			selectControl.unselect(feature);
			prePoint = null;
		}
	} else if(feature.layer.name == "Road") {
		//replanRemainedRoute();
		feature.attributes.average_speed = 5;
		roadLayer.drawFeature(feature);
		postChangeOneTMC(feature.attributes.osm_id, 5);
	}
}

function onFeatureUnselect(feature) {
	if(feature.layer.name == "junction") {
		//prePoint = null;
	} else if(feature.layer.name == "Road") {
		//feature.attributes.average_speed = 20;
	}
}

function regHttpUrl(httpUrl){
	var reg1 = new RegExp(":","g"); 
	var reg2 = new RegExp("/","g"); 
	httpUrl = httpUrl.replace(reg1, "%3A");
	httpUrl = httpUrl.replace(reg2, "%2F");
	
	return httpUrl;
}

function reloadAvgSpeedValue(){
	var requestList = []; var requestJson = {};
	var features = roadLayer.features; 
	for( var i=0; i<features.length; i++ ){			
		requestList.push(features[i].attributes.osm_id);
	}
	
	requestJson.requestListLength = requestList.length;
	requestJson.requestList = requestList;
	requestJson.requestType = "FCD.average_speed";

/*	if( typeof(_FCDAvgSpeedReq)  == "undefined" ) {
		var avgSpeedReq = "proxy/"+_FCDDataServer+"/data/average_speed";
		_FCDAvgSpeedReq = regHttpUrl(avgSpeedReq); console.log(_FCDAvgSpeedReq);
	}*/
	
	$.ajax({
		url: URL,
		type: "POST",
		async: true,
		data: JSON.stringify(requestJson),
		headers: {
			'Accept': 'application/json'
		},
		success: function(data) {
			//var json = JSON.parse(data)
			if( typeof(data.contentListType) == "undefined" || data.contentListType != "FCD.average_speed" ){
				return ;
			}
			
			if( typeof(data.contentListLength) != "undefined" && data.contentListLength != 0 ){
				updateAvgSpeedValue(data.contentList);
			}
		},
		error: function(data) {
		}
	});
}

function updateAvgSpeedValue(averageSpeedList){
	if( typeof(averageSpeedList) == "undefined" ){return ;}
	
	var features = roadLayer.features;
	for( var i=0; i<averageSpeedList.length; i++ ){	
		var osm_id = averageSpeedList[i].osm_id;
		
		for( var j=0; j<features.length; j++ ){
			if( features[j].attributes.osm_id == osm_id && 
				features[j].attributes.average_speed != averageSpeedList[i].value){
				features[j].attributes.average_speed = averageSpeedList[i].value;
				roadLayer.drawFeature(features[j]);
			}
		}
	}
}

function getAllGeo(){
	var wkt = new OpenLayers.Format.WKT();

	var features = layer.features;
	for( var i=0; i<features.length; i++ ){			
		console.log(wkt.write( layer.features[0] ));
		console.log(wkt.write( layer.features[1] ));
	}
}

function mouseOver(obj) {
	obj.style.backgroundColor = "#A9A9A9";
}

function mouseOut(obj) {
	obj.style.backgroundColor = "#D3D3D3";
}

function btnOpenSimu() {
	marker = new OpenLayers.Marker(new OpenLayers.LonLat(simuStart.x, simuStart.y), simuIcon.clone());
	simulateLayer.addMarker(marker);
	simuObj = new Simulation(marker, routeIds, roadData);
	$("#openSimu").animate({opacity: 0}
		, function() {
			$("#openSimu").hide();
			$("#simuPanel").show();
			$("#simuPanel").animate({opacity: 0.8});
		});
}

function btnHideSimu() {
	simuObj.stop();
	simuObj = null;
	simulateLayer.removeMarker(marker);
	marker.destroy();
	$("#simuPanel").animate({opacity: 0}
		, function() {
			$("#simuPanel").hide();
			$("#openSimu").show();
			$("#openSimu").animate({opacity: 0.8});
		});
}

function btnStartSimu() {
	simuObj.start();
}

function btnPauseSimu() {
	simuObj.pause();
}

function btnResumeSimu() {
	simuObj.resume();
}

function btnReplan() {
	$('#tmc-des').hide("drop");
	$('#openSimu').hide();
	removeRoads();
	showLayer(juncLayer);
	hideLayer(simulateLayer);
}

function toggleRoutePlanning() {
	if(!rpSwitcher) {
	//open route planning
		$('#routingManner').show("drop");
		showLayer(juncLayer);
		showLayer(roadLayer);
		
		rpSwitcher = true;
	} else {
	//close
		$('#routingManner').hide("drop");
		$('#tmc-des').hide("drop");
		$('#openSimu').hide();
		hideLayer(juncLayer);
		hideLayer(roadLayer);
		hideLayer(simulateLayer);
		
		rpSwitcher = false;
	}
}
