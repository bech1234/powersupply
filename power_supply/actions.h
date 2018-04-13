class SendMessageAction;
class SendPowerOnAction;
class ApplyLedStateAction;
class TestColourAction;
class SelfTestAction;
class LoopWebsocketAction;
class ConnectWebsocketAction;
class ConnectingAction;
class ConnectWiFiAction;
class ScanningAction;
class ScanNetworksAction;

class SendMessageAction : public ActionInterface {
  private:
    String message;
  public:
    SendMessageAction(String m) : ActionInterface( &executor, micros(), 1000 ) {
      message = m;
    }

    unsigned short execute();
};


class SendPowerOnAction : public SendMessageAction {
  public:
    SendPowerOnAction() : SendMessageAction("{'messageType':'7ae4915c16862cf47d5b','messages':[{'device':'Power Supply'}]}") {}
};

class ApplyLedStateAction : public ActionInterface {
  public:
    ApplyLedStateAction(unsigned long b, unsigned long d) : ActionInterface( &executor, b, d ) {
    }

    unsigned short execute();
};

class TestColourAction : public ActionInterface {
  private:
    int colour = LED_OFF;

  public:
    TestColourAction(int c, unsigned long d) : ActionInterface( &executor, micros(), d ) {
      colour = c;
    }

    unsigned short execute();
};

class SelfTestAction : public ActionInterface {
  public:
    SelfTestAction() : ActionInterface( &executor, micros(), 1000 ) {
    }

    unsigned short execute();
};

class LoopWebsocketAction : public ActionInterface {
  private:

  public:
    LoopWebsocketAction(unsigned long d) : ActionInterface( &executor, micros(), d ) {
    }

    unsigned short execute();
};

class ConnectWebsocketAction : public ActionInterface {
  private:

  public:
    ConnectWebsocketAction(unsigned long d) : ActionInterface( &executor, micros(), d ) {
    }

    unsigned short execute();
};


class ConnectingAction : public ActionInterface {
  private:
    int lastBlue = -1;

  public:
    ConnectingAction(unsigned long d) : ActionInterface( &executor, micros(), d ) {
    }

    unsigned short execute();
};


class ConnectWiFiAction : public ActionInterface {
  private:
    const char* ssid[4] = {"Holmemosen18", "Holmemosen18_2", "KasperAP", "pnet"};
    const char* password[4] = { "fnidderfnadder", "fnidderfnadder", "fnidderfnadder", "sapsapsap" };

  public:
    ConnectWiFiAction(unsigned long d) : ActionInterface( &executor, micros(), d ) {
    }

    unsigned short execute();
};

class ScanningAction : public ActionInterface {
  private:
    int lastRed = -1;

  public:
    ScanningAction(unsigned long d) : ActionInterface( &executor, micros(), d ) {
    }

    unsigned short execute();
};

class ScanNetworksAction : public ActionInterface {
  private:

  public:
    ScanNetworksAction(unsigned long d) : ActionInterface( &executor, micros(), d ) {
    }

    unsigned short execute();
};


/********/



unsigned short SendMessageAction::execute() {
  webSocket.sendTXT(message);
  return COMPLETE;
}


unsigned short ApplyLedStateAction::execute() {
  addDelay(10000);

  for ( byte color = LED_GREEN; color <= LED_RED; color = color * 2 ) {
    for (int led = 7; led >= 0; led--) {
      digitalWrite(SER, ledState[led] & color);
      digitalWrite(CLOCK, HIGH);
      digitalWrite(CLOCK, LOW);
    }
  }
  digitalWrite(LATCH, HIGH);
  digitalWrite(LATCH, LOW);

  return REPEAT;
}


unsigned short TestColourAction::execute() {
  Serial.print("Test colour ");
  Serial.println(colour);
  for (int i = 0; i < 8; i++) {
    ledState[i] = colour;
  }

  return COMPLETE;
}

unsigned short SelfTestAction::execute() {
  Serial.println("Test LEDs");
  executor.addAction(new TestColourAction(LED_RED, 10));
  executor.addAction(new TestColourAction(LED_GREEN, 500000));
  executor.addAction(new TestColourAction(LED_BLUE, 1000000));
  executor.addAction(new TestColourAction(LED_OFF, 1500000));

  return COMPLETE;
}


unsigned short LoopWebsocketAction::execute() {
  addDelay(100000);
  webSocket.loop();

  return REPEAT;
}


unsigned short ConnectWebsocketAction::execute() {
  webSocket.onEvent(webSocketEvent);
  webSocket.setExtraHeaders("Authorization: Bearer c13ed8da6c113914a897d454b64e92");
  webSocket.beginSSL("iotmmss0007342869trial.hanatrial.ondemand.com", 443, "/com.sap.iotservices.mms/v1/api/ws/data/1f6c92c7-9c8f-4072-ba08-ea709528a624", "7a d8 d0 e6 a5 29 3f 47 55 b4 eb 80 54 4b a2 1b a5 b7 52 61", "wss");

  executor.addAction(new LoopWebsocketAction(1000));

  return COMPLETE;
}



unsigned short ConnectingAction::execute() {
  // Wait for connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    lastBlue = (lastBlue + 1) % 8;
    if (ledState[lastBlue] == LED_BLUE) {
      ledState[lastBlue] = LED_OFF;
    } else {
      ledState[lastBlue] = LED_BLUE;
    }
    addDelay(500000);
    return REPEAT;
  }

  for (int i = 0; i < 8; i++) {
    ledState[i] = LED_OFF;
  }

  Serial.println();

  executor.addAction(new ConnectWebsocketAction(100000));

  return COMPLETE;
}


unsigned short ConnectWiFiAction::execute() {
  int networks = WiFi.scanComplete();
  int maxStrength = -100;
  int networkIndex = -1;

  for (int i = 0; i < networks; i++) {
    if (WiFi.RSSI(i) > maxStrength) {
      for (int j = 0; j < 4; j++) {
        if (WiFi.SSID(i) == ssid[j]) {
          maxStrength = WiFi.RSSI(i);
          networkIndex = j;
          break;
        }
      }
    }
  }

  if (networkIndex == -1) {
    WiFi.scanDelete();
    executor.addAction(new ScanNetworksAction(10000000l));
    return COMPLETE;
  }

  Serial.print("Connecting ");
  Serial.println(ssid[networkIndex]);
  WiFi.begin(ssid[networkIndex], password[networkIndex]);
  executor.addAction(new ConnectingAction(10000));

  return COMPLETE;
}



unsigned short ScanningAction::execute() {
  // Wait for scan result
  int networks = WiFi.scanComplete();

  if (networks < 0) {
    Serial.print(".");
    lastRed = (lastRed + 1) % 8;
    if (ledState[lastRed] == LED_RED) {
      ledState[lastRed] = LED_OFF;
    } else {
      ledState[lastRed] = LED_RED;
    }
    addDelay(500000);
    return REPEAT;
  }

  for (int i = 0; i < 8; i++) {
    ledState[i] = LED_OFF;
  }

  Serial.println();
  for (int i = 0; i < networks; i++) {
    Serial.printf("%d: %s, %ddBm\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
  }


  executor.addAction(new ConnectWiFiAction(1000));

  return COMPLETE;
}



unsigned short ScanNetworksAction::execute() {
  Serial.println("Scanning for WiFi networks");
  WiFi.scanNetworks(true, false);
  executor.addAction(new ScanningAction(10000));

  return COMPLETE;
}




