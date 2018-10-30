function msgDispatch(msg) {
	var notify = msg.split(':');
	if(notify[0] == "Notification") {
		if(notify[1] == "TrafficJam") {
			console.log("Traffic Jam: " + handleTrafficJamMsg());
		} else if(notify[1] == "SimuCompleted") {
			btnHideSimu();
		}
	}
}

function handleTrafficJamMsg() {
	/*
	var ret = confirm("Traffic Jam happens in some road.\nDo you need to replan the routing?");
	
	if(ret) {
		//alert("The routing plan will be recompulated.");
	} else {
		//alert("Keep current routing plan.");
	}*/
	//check whether it is needed to replan or not
	if(routeIds == null || simuObj == null) return false;
	for(var i = 0; i < routeIds.length; ++i) {
		var t = trafficLayer.getFeaturesByAttribute("id", routeIds[i]);
		if(t[0].attributes.average_speed < 10) {
			replanRemainedRoute();
			return true;
		}
	}
	return false;
}