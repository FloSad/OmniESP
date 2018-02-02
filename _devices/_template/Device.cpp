#include "Device.h"
#include <Arduino.h>

//===============================================================================
//  Device
//===============================================================================

//-------------------------------------------------------------------------------
//  Device public
//-------------------------------------------------------------------------------
Device::Device(LOGGING &logging, TopicQueue &topicQueue, FFS& ffs)
               : logging(logging), topicQueue(topicQueue), ffs(ffs) {}

//...............................................................................
// device start
//...............................................................................
void Device::start() {

  logging.info("starting Device " + String(DEVICETYPE) + " V" + String(DEVICEVERSION));

  //device start instructions


  logging.info("Device running");

}

//...............................................................................
// handle - periodically called by the controller
//...............................................................................
void Device::handle() {
}

//...............................................................................
//  Device set
//...............................................................................

String Device::set(Topic &topic) {
  /*
  ~/set                (Itemlevel 2)
    └─function         (Itemlevel 3)
        └─sub function (Itemlevel 4)
  */

  logging.debug("Device set topic " + topic.topic_asString() + " to " +
                topic.arg_asString());

//e.g.
  if (topic.itemIs(3, "led")) {
    if (topic.itemIs(4, "01")){
      ///## your Event
      return TOPIC_OK;
    }else if (topic.itemIs(4, "02")){
      //## your Event
      return TOPIC_OK;
    }else{
      return TOPIC_NO;
    }
  }else{
    return TOPIC_NO;
  }
}
//...............................................................................
//  Device get
//...............................................................................

String Device::get(Topic &topic) {
  /*
  ~/get                (Itemlevel 2)
    └─function         (Itemlevel 3)
        └─sub function (Itemlevel 4)
  */

  logging.debug("Device get topic " + topic.topic_asString() + " to " +
                topic.arg_asString());

//e.g.
  if (topic.itemIs(3, "led")) {
    return "get answer";
  }else{
    return TOPIC_NO;
  }
}

//...............................................................................
// Eventhandler - called by the controller after receiving a topic (event)
//...............................................................................
void Device::on_events(Topic &topic) {
}

//-------------------------------------------------------------------------------
//  Device private
//-------------------------------------------------------------------------------
