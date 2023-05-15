$( document ).ready(function() {

  // Device
  var address = "172.20.10.8";
  var device = new Device(address);

  // Buttons
  $('#oppositeConstant').mousedown(function() {
    device.callFunction("oppositeConstant");
  });
  $('#oppositeConstant').mouseup(function() {
    device.callFunction("stop");
  });

  $('#alternating').mousedown(function() {
    device.callFunction("alternating");
  });
  $('#alternating').mouseup(function() {
    device.callFunction("stop");
  });

  $('#randomMotion').mousedown(function() {
    device.callFunction("randomMotion");
  });
  $('#randomMotion').mouseup(function() {
    device.callFunction("stop");
  });

  $('#stop').mousedown(function() {
    device.callFunction("stop");
  });

});

