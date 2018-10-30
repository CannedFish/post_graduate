function Simulation(_marker, _roadList, _roadData) {
	//attributes
	var marker = _marker;
	var roadList = _roadList;//[roadId1, roadId2, ...]
	var roadData = _roadData;//[roadId1:{coors:[point1, ...]}, ...]
	var stepInterval = 10;
	var moveSpeed = 1000;
	var simuList = undefined;
	var curRoadNum = undefined;
	var curPointInRoad = undefined;
	var timer = undefined;
	//temp variables
	var pointList = undefined;
	var pointStr = undefined;
	var points = undefined;
	var roadPoints = undefined;
	//private methods
	function initSimuList() {
		simuList = [];
		curRoadNum = 0;
		curPointInRoad = 0;
		transRoads2SimuList(curRoadNum, roadList);
	}
	
	function transRoads2SimuList(_start, _rIdList) {
		var startPoint = {};
		startPoint.x = ((_start == 0) ? marker.lonlat.lon : simuList[_start][0].x);
		startPoint.y = ((_start == 0) ? marker.lonlat.lat : simuList[_start][0].y);
		var dir = true;
		
		for(var i = _start; i < _rIdList.length; ++i) {
			roadPoints = [];
			pointStr = roadData[_rIdList[i]].coors;
			pointStr = pointStr.slice(11, -1);
			pointList = pointStr.split(", ");
			for(var j = 0; j < pointList.length; ++j) {
				points = pointList[j].split(" ");
				roadPoints[j] = {};
				roadPoints[j].x = points[0];
				roadPoints[j].y = points[1];
			}
			
			(roadPoints[0].x == startPoint.x && roadPoints[0].y == startPoint.y) ? (dir = true) : (dir = false);
			
			transRoad2SimuList(roadPoints, i, dir);
			
			if(dir) {
				startPoint.x = roadPoints[roadPoints.length - 1].x;
				startPoint.y = roadPoints[roadPoints.length - 1].y;
			} else {
				startPoint.x = roadPoints[0].x;
				startPoint.y = roadPoints[0].y;
			}
		}
	}
	
	function startPos(direction, len) {
		return (direction ? (0) : (len - 1));
	}
	
	function nextPos(cur, direction) {
		return (direction ? (cur + 1) : (cur - 1));
	}
	
	function isDone(cur, direction, len) {
		return direction ? (cur == len - 1) : (cur == 0);
	}
	
	function transRoad2SimuList(_roadPoints, rNum, direction) {
	//_roadPoints: [{x1,y1},{x2,y2},...]
		simuList[rNum] = [];
		var xDis, yDis, xPol, yPol, xStep, yStep;
		var tmp = {}, tmp2 = {};
		for(var i = startPos(direction, _roadPoints.length);
				!isDone(i, direction, _roadPoints.length); 
				i = nextPos(i, direction)) {
			tmp.x = parseFloat(_roadPoints[i].x);
			tmp.y = parseFloat(_roadPoints[i].y);
			tmp2.x = parseFloat(_roadPoints[nextPos(i, direction)].x);
			tmp2.y = parseFloat(_roadPoints[nextPos(i, direction)].y);
			
			(tmp.x > tmp2.x) ? (xPol = -1) : (xPol = 1);
			/*
			if(tmp.y > tmp2.y) {
				console.log(">");
				console.log(tmp.y + " " + tmp2.y);
			} else {
				console.log("<=");
				console.log(tmp.y + " " + tmp2.y);
			}
			*/
			(tmp.y > tmp2.y) ? (yPol = -1) : (yPol = 1);
			
			xDis = Math.abs(tmp.x - tmp2.x);
			yDis = Math.abs(tmp.y - tmp2.y);
			
			if(xDis >= yDis) {
				xStep = stepInterval;
				yStep = Math.round(xStep / xDis * yDis * 100) / 100;
			} else {
				yStep = stepInterval;
				xStep = Math.round(yStep / yDis * xDis * 100) / 100;
			}
			
			while((tmp2.x - tmp.x) * xPol > 0 
					&& (tmp2.y - tmp.y) * yPol > 0) {
				var tmp1 = {};
				tmp1.x = tmp.x;
				tmp1.y = tmp.y;
				simuList[rNum].push(tmp1);
				tmp.x += (xPol * xStep);
				tmp.y += (yPol * yStep);
			}
		}
		simuList[rNum].push(tmp2);
	}
	
	function doSimulation() {
		var offX, offY;
		timer = setInterval(function() {
			if(curRoadNum >= simuList.length) {
				completeSimu();
				return ;
			}
			
			marker.moveTo(
				marker.map.getPixelFromLonLat(
					new OpenLayers.LonLat(
						simuList[curRoadNum][curPointInRoad].x,
						simuList[curRoadNum][curPointInRoad].y
					)
				)
			);
			
			curPointInRoad++;
			if(curPointInRoad >= simuList[curRoadNum].length) {
				curPointInRoad = 0;
				curRoadNum++;
			}
		}, moveSpeed);
	}
	
	function completeSimu() {
		clearInterval(timer);
		curRoadNum = 0;
		curPointInRoad = 0;
		/*
		marker.moveTo(
			marker.map.getPixelFromLonLat(
				new OpenLayers.LonLat(
					simuList[0][0].x,
					simuList[0][0].y
				)
			)
		);
		*/
		alert("Simulation Complated.");
		msgDispatch("Notification:SimuCompleted");
	}
	
	function startImp() {
		initSimuList();
		doSimulation();
	}
	
	function stopImp() {
		clearInterval(timer);
		timer = null;
	}
	
	function pauseImp() {
		clearInterval(timer);
		timer = null;
	}
	
	function resumeImp() {
		doSimulation();
	}
	
	function replanImp(_newRoadList) {
		transRoads2SimuList(curRoadNum + 1, _newRoadList);
		roadList = _newRoadList;
	}
	
	return {
	//interfaces
		setIcon: function(_marker) {
			marker = _marker;
		},
		
		setRoadList: function(_roadList) {
			roadList = _roadList;
		},
		
		setRoadData: function(_roadData) {
			roadData = _roadData;
		},
		
		getReplanEndPoints: function() {
			var l1 = simuList[curRoadNum].length;
			var l2 = simuList[roadList.length - 1].length;
			return {
				start: simuList[curRoadNum][l1 - 1],
				end: simuList[roadList.length - 1][l2 - 1]
			};
		},
		
		getCurRoadNum: function() {
			return curRoadNum;
		},
		
		start: function() {
			startImp();
		},
		
		stop: function() {
			stopImp();
		},
		
		pause: function() {
			pauseImp();
		},
		
		resume: function() {
			resumeImp();
		},
		
		replan: function(_newRoadList) {
			//pauseImp();
			replanImp(_newRoadList);
			//resumeImp();
		}
	};
}