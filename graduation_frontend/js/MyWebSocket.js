function MyWebSocket(_url, _eventHandlers) {
	var url = _url;
	var evtHandlers = _eventHandlers;
	var webSocket = undefined;
	
	function onOpen(evt) {
		webSocket.send("Hello");
		console.log("Connected to web socket server successfully");
	}
	
	function onMessage(evt) {
		console.log("Receive a message from server: " + evt.data);
	}
	
	function onClose(evt) {
		console.log("Connection has been closed");
	}
	
	function onError(evt) {
		console.log("Error occured: " + evt.data);
	}
	
	return {
		open : function() {
			webSocket = new WebSocket(url);
			if(typeof webSocket === "undefined") {
				console.log("Create web socket failed..");
				return ;
			}
			
			webSocket.onopen = function (evt) {
				if(typeof evtHandlers.onOpen === "undefined") {
					onOpen(evt);
				} else {
					evtHandlers.onOpen(evt);
				}
			}
			
			webSocket.onclose = function (evt) {
				if(typeof evtHandlers.onClose === "undefined") {
					onClose(evt);
				} else {
					evtHandlers.onClose(evt);
				}
			}
			
			webSocket.onmessage = function (evt) {
				webSocket.send("Message recived!");
				if(typeof evtHandlers.onMessage === "undefined") {
					onMessage(evt);
				} else {
					evtHandlers.onMessage(evt);
				}
			}
			
			webSocket.onerror = function (evt) {
				if(typeof evtHandlers.onError === "undefined") {
					onError(evt);
				} else {
					evtHandlers.onError(evt);
				}
			}
		},
		
		close : function() {
			if(webSocket != null || typeof webSocket !== "undefined") {
				webSocket.close();
				webSocket = null;
			}
		},
		
		sendMsg : function(msg) {
			var ret = webSocket.send(msg);
			if(!ret) {
				console.log("Fail to send message to server.");
			}
		}
	};
}