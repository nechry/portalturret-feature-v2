#include <WebSocketsServer.h>

WebSocketsServer webSocket = WebSocketsServer(81);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{ 
  // When a WebSocket message is received
  switch (type)
  {
  case WStype_TEXT:
    debug("Websocket text received: ");
    debugln((char *)payload);
    break;
  case WStype_ERROR:
    debugln("Error: ");
    debugln((int)payload);
    break;
  case WStype_BIN:
    switch (payload[0])
    {
    case 0:
      if (isOpen())
      {
        debug("Setting servo to: ");
        debugln(payload[1]);
        rotateServo.write(payload[1]);
      }
      break;
    case 1:
      debugln("Firing");      
      setManualState(ManualState::Firing);
      break;
    case 2:
      if (payload[1] == 0)
      {
        debugln("Setting manual state to stationary");
        currentRotateDirection = 0;
      }
      else if (payload[1] == 1)
      {
        debugln("Setting manual state to opening");
        currentRotateDirection = 1;
      }
      else if (payload[1] == 2)
      {
        debugln("Setting manual state to closing");
        currentRotateDirection = -1;
      }
      break;
    }
    break;
  case WStype_DISCONNECTED: // if the websocket is disconnected
    debugln("Websocket disconnected");
    break;
  case WStype_CONNECTED: // if a new websocket connection is established
    IPAddress ip = webSocket.remoteIP(num);
    // Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
    debug("Websocket connected from ");
    debugln(ip);
    break;
  // case WStype_TEXT: // if new text data is received
  //   // Serial.printf("[%u] get Text: %s
  //   debug("Websocket text received: ");
  //   debugln((char *)payload);
  //   break;
  }
}

void startWebSocket()
{
  // Start a WebSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent); // if there's an incoming websocket message, go to function 'webSocketEvent'
  websocketStarted = true;  
}
