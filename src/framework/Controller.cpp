#include "framework/Controller.h"

//###############################################################################
//  Controller
//###############################################################################

/*
 * The controller has two main entry points: call() and handleEvent().
 * call() is called from the API and calls get or set functions in the
 * devices. The devices may react with events which they put in the
 * topicQueue to be handled by handleEvent(). Other spontaneously
 * received information from the devices are also pout in the topicQueue.
 * handleEvent() dispatches the events to the views via viewsUpdate().
 * This routine also contains the particular logic of the firmware:
 * depending on the event received, an action is carried out, i.e. a
 * function of a device is called.
 */

//-------------------------------------------------------------------------------
//  Controller public
//-------------------------------------------------------------------------------

//...............................................................................
//  constructor
//...............................................................................
Controller::Controller()
    : logging(clock), ffs(logging), clock(topicQueue),
      espTools(logging), wifi(logging, ffs), device(logging, topicQueue, ffs) {

  // callback Events
  // WiFi
  wifi.set_callback(std::bind(&Controller::on_wifiConnected, this),
                    std::bind(&Controller::on_wifiDisconnected, this));
}

//...............................................................................
//  API set callback
//...............................................................................
void Controller::setTopicFunction(TopicFunction topicFn) {
  topicFunction= topicFn;
}

//-------------------------------------------------------------------------------
//  start
//-------------------------------------------------------------------------------
void Controller::start() {

  //
  // bootstrapping
  //

  // start the clock
  clock.start();

  // enable the logging subsystem
  logging.start();
  logging.info(SysUtils::fullVersion());

  // start esp tools
  espTools.start();

  // fire up the FFS
  ffs.mount();

  // set ESP name
  deviceName= ffs.cfg.readItem("device_name");
  if (deviceName == "") {
    deviceName = "ESP8266_" + espTools.genericName();
    logging.info("setting device name " + deviceName + " for the first time");
    ffs.cfg.writeItem("device_name", deviceName);
    logging.info("setting access point SSID " + deviceName +
                 " for the first time");
    ffs.cfg.writeItem("ap_ssid", deviceName);
    ffs.cfg.saveFile();
  }

  // start WiFi for the first time
  wifi.start();

  // startup the device
  device.start();
  logging.info("controller started");
}

//-------------------------------------------------------------------------------
//  getDeviceName()
//-------------------------------------------------------------------------------
String Controller::getDeviceName() {
  return deviceName;
}

//...............................................................................
//  handle connection
//...............................................................................
void Controller::handle() {

  //if (!wifi.handle()) {
  //  wifi.start();
  //}
  wifi.handle();  //check wifi-status continuous and start if offline
  ftpSrv.handleFTP();
  clock.handle();
  device.handle();

  //handle events
  while (topicQueue.count) {
    String topicsArgs = topicQueue.get();
    yield();
    handleEvent(topicsArgs);
  }
}

//...............................................................................
//  handle Event
//...............................................................................
void Controller::handleEvent(String &topicsArgs) {

  //
  // this is the central routine that dispatches events from devices
  // and views
  //
  time_t t= clock.now();

  //logging.debug("handling event " + topicsArgs);
  Topic topic(topicsArgs);

  // propagate event to views and the device
  viewsUpdate(t, topic);
  device.on_events(topic);

}

//...............................................................................
//  EVENT Wifi has connected
//...............................................................................
void Controller::on_wifiConnected() {
  logging.info("WiFi has connected");
  topicQueue.put("~/event/wifi/connected", 1);
  on_netConnected();
}

//...............................................................................
//  EVENT wifi has disconnected
//...............................................................................
void Controller::on_wifiDisconnected() {
  logging.info("WiFi has disconnected");
  topicQueue.put("~/event/wifi/connected", 0);
  on_netDisconnected();
}

//...............................................................................
//  EVENT LAN has connected
//...............................................................................
void Controller::on_lanConnected() {
  logging.info("LAN has connected");
  topicQueue.put("~/event/lan/connected", 1);
  on_netConnected();
}

//...............................................................................
//  EVENT lan has disconnected
//...............................................................................
void Controller::on_lanDisconnected() {
  logging.info("LAN has disconnected");
  topicQueue.put("~/event/lan/connected", 0);
  on_netDisconnected();
}
//...............................................................................
//  EVENT Network has connected
//...............................................................................
void Controller::on_netConnected() {
  if (ffs.cfg.readItem("ntp") == "on") {
    // start the clock with NTP updater
    String ntpServer = ffs.cfg.readItem("ntp_serverip");
    char txt[128];
    sprintf(txt, "starting NTP client for %.127s", ntpServer.c_str());
    logging.info(txt);
    clock.start(ntpServer.c_str(), NO_TIME_OFFSET, NTP_UPDATE_INTERVAL);
  } else {
    logging.info("NTP client is off");
  }

  // add FTP to web-config!
  //if (ffs.cfg.readItem("ftp") == "on") {

    logging.info("starting FTP-Server");
    ftpSrv.begin("esp8266","esp8266");

    //ftpSrv.begin(ffs.cfg.readItem("ftp_username"),
    //             ffs.cfg.readItem("ftp_password"));
  //} else {
    //logging.info("FTP-Server is off");
  //}

  logging.info("Network connection established");
  topicQueue.put("~/event/net/connected", 1);
}

//...............................................................................
//  EVENT lan has disconnected
//...............................................................................
void Controller::on_netDisconnected() {
  //if LAN is presend
  //check if LAN AND WiFi are disconnected!!
  logging.info("Network connection aborted");
  topicQueue.put("~/event/net/connected", 0);
}

//...............................................................................
//  API
//...............................................................................

String Controller::call(Topic &topic) {

  // D("Controller: begin call");
  // set
  if (topic.itemIs(1, "set")) {
    if (topic.itemIs(2, "ffs")) {
      return ffs.set(topic);
    } else if (topic.itemIs(2, "clock")) {
      return clock.set(topic);
    } else if (topic.itemIs(2, "esp")) {
      return espTools.set(topic);
    } else if (topic.itemIs(2, "device")) {
      return device.set(topic);
    } else {
      return TOPIC_NO;
    }
    // get
  } else if (topic.itemIs(1, "get")) {
    if (topic.itemIs(2, "ffs")) {
      return ffs.get(topic);
    } else if (topic.itemIs(2, "clock")) {
      return clock.get(topic);
    } else if (topic.itemIs(2, "esp")) {
      return espTools.get(topic);
    } else if (topic.itemIs(2, "wifi")) {
      return wifi.get(topic);
    } else if (topic.itemIs(2, "device")) {
      return device.get(topic);
    } else {
      return TOPIC_NO;
    }
  } else {
    return TOPIC_NO;
  }
  // D("Controller: end call");
}

//...............................................................................
//  idle timer
//...............................................................................
void Controller::t_1s_Update() {}

void Controller::t_short_Update() {
  espTools.debugMem();
  logging.debug("uptime: "+SysUtils::uptimeStr(clock.uptime()));
};

void Controller::t_long_Update() {}

//-------------------------------------------------------------------------------
//  Controller private
//-------------------------------------------------------------------------------

//...............................................................................
//  Start WiFi Connection
//...............................................................................
bool Controller::startConnections() {

  logging.info("starting network connections");
  bool result = wifi.start();
  if (result) {

    //start after wifi is connected

  }
  return result;
}

//...............................................................................
//  update Views
//...............................................................................
void Controller::viewsUpdate(time_t t, Topic &topic) {

  if (topicFunction != nullptr)
    // this is done in the API topic.setItem(0, deviceName.c_str());
    topicFunction(t, topic);
}
