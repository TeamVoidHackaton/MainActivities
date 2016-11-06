/* var express, path, drone, server, app, faye, client, controller, morgan, methodOverride, Leap;

express = require("express");
morgan = require("morgan");
bodyParser = require("body-parser");
methodOverride = require('method-override');
path = require("path");
http = require('http');
faye = require('faye');
Leap = require('leapjs') // added gestures
	, controller = new Leap.Controller({enableGestures:true})
	, direction = require('curtsy');

drone = require("ar-drone").createClient(); // enables communication with drone in javascript
// require("dronestream").listen(5555); // 5555 for video rendering
app = express();


var env = process.env.NODE_ENV || 'development';
if('development' == env){
  app.set('port', process.env.PORT || 3000); // 8081, 5556, 11311, 5559, 3000 process.env.PORT adjusts PORT to accept environmental parameter (ie deploying to Heroku)
  app.use(express.static(__dirname + '/public')); // serves static files from disk
  app.use("/node_modules", express.static(path.join(__dirname, 'node_modules')))
}

server = require('http').createServer(app);

var bayeux = new faye.NodeAdapter({  // central messaging server for clients to communicate with one another; Can also add 'engine' property, which controls backend of the server (ie faye-redis) and 'ping' property, which is how often, in seconds, to send keep-alive ping messages over WebSocket and EventSource connections. Used if Faye server will be accessed through a proxy that kills idle connections.
  mount: '/faye', // path on the host at which the Faye service is available (ie http://localhost:3001/faye).
  timeout: 50  // maximum time to hold connection open before returning response. Given in seconds and must be smaller than timeout on frontend webserver.
});

bayeux.attach(server); // attached to server; will handle all requests to paths matching the mount path and delegate all other requests to handlers.

client = new faye.Client("http://localhost:" + (app.get("port")) + "/faye", {}); // sets up new client at environmental port that accesses the server at the /faye mount

client.subscribe("/drone/move", function (d) { // move includes any directional actions
  console.log(d);
  console.log("drone move?")
  return drone[d.action](d.speed);
});

client.subscribe("/drone/drone", function (d) { // drone commands include takeoff and landing
  console.log(d);
  console.log("drone stuff")
  return drone[d.action]();
});

server.listen(app.get('port'), function () {
  console.log("Express server listening on port " + app.get("port"));
})

// events = require('events').EventEmitter.prototype._maxListeners = 100;
// EventEmitter.setMaxListeners(100);

var Cylon = require('cylon');
// events = leapmotion.emitter.setMaxListeners()
// Newly inputted
var Leap = require('leapjs')
	, controller = new Leap.Controller({enableGestures:true})
	, direction = require('curtsy');

controller.connect();

Cylon.robot({
  connections:{
    leapmotion: {adaptor: 'leapmotion'},
    ardrone: {adaptor: 'ardrone', port: '192.168.1.1'}
  },

  devices: {
    leapmotion: {driver: 'leapmotion', connection: 'leapmotion'},
    drone: {driver: 'ardrone', connection: 'ardrone'}
  },
  
/*  work: function(my){
	   my.leapmotion.on('hand', function(hand){
	     my.drone.takeoff();
	     after((5).seconds(), function(){
	       my.drone.land();
	     })
	   })
	 }
	}).start(); 
  
  work: function(my){
		my.leapmotion.on('frame', function(frame){
			if(frame.hands.length > 0){
				my.drone.takeoff();
			} else {
				my.drone.land();
			}
			
			if (gesture.type === 'circle'){
				my.drone.clockwise(0.5);
				after((6).seconds(), function(){
					my.drone.land();
					});
			}
		});
  }.start(); */
  
  work: function(my){
		my.leapmotion.on('frame', function(frame){
			if(frame.hands.length > 0){
				my.drone.takeoff();
				console.log('LETS PLAY');
			} else {
				my.drone.land();
				console.log('GAMES OVER');
			}

			if(frame.valid && frame.gestures.length > 0){
				frame.gestures.forEach(function(g){
					if(g.type === 'circle'){
						my.drone.clockwise(0.8);
						console.log('YAWING');
						/* after((5).seconds(), function(){
					       // my.drone.counterClockwise(1.0);
					       //console.log('UNYAWING');
					       // continue; 
						}) */
					} 
					if (g.type === 'keyTab'){ //keyTab, swipe
						// //my.drone.counterClockwise(0.8); // 0.5
						//if (direction(g).type === "right") { // direction(gesture).
							//my.drone.right();
							my.drone.counterClockwise(0.8); // 0.5
							console.log('YAWING');
							/* after((5).seconds(), function(){
						       // my.drone.clockwise(0.8);
						       // console.log('UNYAWING');
						       // continue; 
							}) */
						//}
					} 
					// ALL OF THE COMMENTS ARE DUE TO THE SPACE LIMITATION
					/*if (g.type === 'swipe'){
						if (direction(g).type === "down") {
							my.drone.down();}
						if (direction(g).type === "forward") {
							my.drone.forward();}
						if (direction(g).type === "back") {
							my.drone.back();}
						if (direction(g).type === "left") {
							my.drone.left();}
						if (direction(g).type === "right") {
							my.drone.right();
						}
					} */
					/*if(g.type === 'swipe'){
						var currentPosition = g.position;
						var startPosition = g.startPosition;

						var xDirection = currentPosition[0] - startPosition[0];
						var yDirection = currentPosition[1] - startPosition[1];
						var zDirection = currentPosition[2] - startPosition[2];

						var xAxis = Math.abs(xDirection);
						var yAxis = Math.abs(yDirection);
						var zAxis = Math.abs(zDirection);

						var superiorPosition  = Math.max(xAxis, yAxis, zAxis);

						if(superiorPosition === xAxis){
							if(xDirection < 0){
								console.log('LEFT');
								my.drone.left();
							} else {
								my.drone.right();
								console.log('RIGHT');
							}
						}

						if(superiorPosition === zAxis){
							if(zDirection > 0){
								console.log('BACKWARDS');
								my.drone.back();
							} else {
								console.log('FORWARD');
								my.drone.forward();
							}
						}

						if(superiorPosition === yAxis){
							if(yDirection > 0){
								console.log('UP');
								my.drone.up(1);
							} else {
								console.log('DOWN');
								my.drone.down(1);
							}
						}
					} */
					
					
				})
			}
		})
  }
  }).start();


			
				
			
				
		
  
		
		
				
			
			
			
			
  