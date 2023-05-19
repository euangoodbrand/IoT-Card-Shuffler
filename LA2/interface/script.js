$( document ).ready(function() {

  // Device
  var address = "172.20.10.8";
  var device = new Device(address);

  // Buttons
  $('#oppositeConstant').mousedown(function() {
    device.callFunction("oppositeConstant");
  });
  $('#oppositeConstantForm').submit(function(event) {
    event.preventDefault(); // prevent the form from submitting normally
    var numCards = $('#cardsNumber').val();
    device.callFunction("oppositeConstant", numCards);
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

  $('#oppositeConstantForm').submit(function(event) {
    event.preventDefault(); // prevent the form from submitting normally
    var numCards = $('#cardsNumber').val();
    var count = 0;
    var intervalId = setInterval(function() {
      $('#cardsShuffledCount').text("Cards shuffled: " + count);
      count++;
      if(count > numCards) {
        clearInterval(intervalId);
        $('#cardsShuffledCount').text("Shuffling complete!");
      }
    }, 1000);  // update the count every second
    device.callFunction("oppositeConstant", numCards);
    device.callFunction("stop");
  });
  

});

