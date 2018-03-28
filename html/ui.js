"use strict";

// ---------------------------
// panel handling
// ---------------------------

// show panel panelName if radio has value
function showDetails(panelName, radio, value) {
  var panel = $("#" + panelName);
  var v = radio.val();
  //logmsg("panel " + panelName + ", radio is " + v + ", show if " + value);
  if (v.match(value)) {
    panel.show();
  } else {
    panel.hide();
  }

}

function setRadioHandlers() {
  logmsg("Setting radio handlers...");
  $(document).on("change", "input[name=wifi]", function() {
    showDetails("wifi_net_details", $(this), "manual");
    showDetails("wifi_details", $(this), "dhcp|manual");
  });
  $(document).on("change", "input[name=lan]", function() {
    showDetails("lan_net_details", $(this), "manual");
  });
  $(document).on("change", "input[name=ntp]", function() {
    showDetails("ntp_details", $(this), "on");
  });
  $(document).on("change", "input[name=mqtt]", function() {
    showDetails("mqtt_details", $(this), "on");
  });
}

// ---------------------------
// actions
// ---------------------------

//
// check if alive
//
function ping() {
  $.ajax({
    url: "/",
    success: function(result) {
      startOver();
    }
  })
}

function reloadIfAlive() {
  logmsg("reload if alive...");
  setInterval(ping, 3000);
}

//
// post action reboot
//
function reboot() {
  $("#popupRebooting").popup("open");
  call("~/set/esp/restart");
  reloadIfAlive();
}

//
// set time
//
function setTime() {
  call("~/set/clock/forceNTPUpdate");
}

//
// update
//
function update() {
  // show popup
  var file = $("#update_localpath").prop("files")[0];
  if (file) {
    logmsg("Uploading " + file.name + " (" + file.size + " bytes)");
    $.ajax({
      type: "POST",
      url: "/update.html",
      cache: false,
      contentType: false,
      processData: false,
      data: new FormData($("#update_form")[0]),
      success: function(data) {
        //$("#popupUpdating").popup("open");
        //reloadIfAlive;
        logmsg("firmware update: "+data);
        if(data == "ok") {
          $("#popupUpdateOk").popup("open");
        } else {
          $("#update_error").html(data);
          $("#popupUpdateFail").popup("open");
        }
      }
    });

  }
}

// ---------------------------
// API
// ---------------------------

function call(topicsArgs, callback) {
  logmsg("API async call "+topicsArgs);
  // show popup
  //$("#popupProcessing").popup("open");
  // this is aynchronous!
  $.post(
      "/api.html",
      "call="+topicsArgs,
      function(data, status) {
            //logmsg("API status: "+status);
            logmsg("API async result: "+data);
            if(callback) {
              callback(data);
            }
      },
      "text"
  ).fail(function() { startOver(); });
  //$("#popupProcessing").popup("close");
}


// ---------------------------
// config
// ---------------------------

//
// retrieve configuration from device
//
function retrieveConfig(callback) {
  logmsg("Retrieving configuration from device...");
  return call("~/get/ffs/cfg/root", callback);
}

//
// sent configuration to device
//
function saveConfig() {
  logmsg("Saving configuration in device...");
  call("~/set/ffs/cfg/saveFile");
}

function sendConfig(json) {
  logmsg("Sending configuration to device...");
  call("~/set/ffs/cfg/root " + json, saveConfig);
}

//
// set inputs from config
//

// set text input
function setText(name, value) {

  //debugmsg("Setting input " + name + " to " + value);
  var input= $("#"+name).filter(":input");
  input.val(value);
}

// set radio button
function setRadio(name, value) {

  //debugmsg("Setting radio " + name + " to " + value);
  var radio = $('input:radio[name="' + name + '"]');
  var button = radio.filter('[value="' + value + '"]');
  button.prop('checked', true);
  radio.checkboxradio('refresh');
  button.trigger('change');
}

// set inputs from config
function setConfig(json) {
  logmsg("Setting inputs from configuration...")
  var config= JSON.parse(json);
  var configForm= $("#config");
  // set text inputs
  configForm.find('input:text').val(function() {
    //logmsg("Setting text input " + this.name + " to " + config[this.name]);
    return config[this.name];
  });
  // set password inputs
  configForm.find('input:password').val(function() {
    //logmsg("Setting password input " + this.name + " to " + config[this.name]);
    return config[this.name];
  });
  // set radio groups
  setRadio("wifi", config["wifi"]);
  setRadio("lan", config["lan"]);
  setRadio("ntp", config["ntp"]);
  setRadio("update", config["update"]);
  setRadio("mqtt", config["mqtt"]);
}

//
// get configuration from inputs
//

// get radio button
function getRadio(name) {
  //debugmsg("Getting radio "+name);
  var button = $('input:radio[name="' + name + '"]:checked');
  return button.val();
}

// get text input
function getText(name, value) {

  //debugmsg("Getting input " + name);
  var input= $("#"+name).filter(":input");
  return input.val();
}


// get configuration from inputs
function getConfig() {
  logmsg("Getting configuration from inputs...");

  var config = {};
  var configForm= $("#config");
  // get text inputs
  configForm.find('input:text').each(function() {
    config[$(this).attr("id")]= $(this).val();
  })
  // get password inputs
  configForm.find('input:password').each(function() {
    config[$(this).attr("id")]= $(this).val();
  })
  // get radio groups
  config["wifi"]= getRadio("wifi");
  config["lan"]= getRadio("lan");
  config["ntp"]= getRadio("ntp");
  config["update"]= getRadio("update");
  config["mqtt"]= getRadio("mqtt");
  // convert to json
  return JSON.stringify(config);
}

//
// applyConfiguration
//
function apply() {
  var json= getConfig();
  //logmsg(json);
  sendConfig(json);
}