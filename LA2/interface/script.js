$( document ).ready(function() {

  // Device
  var address = "172.20.10.8";
  var device = new Device(address);

  // Buttons

  $('#oppositeConstantForm').submit(function(event) {
    event.preventDefault(); // prevent the form from submitting normally
    var numCards = $('#cardsNumber').val();
    var count = 0;
    var intervalId = setInterval(function() {
      var percentage = Math.round((count / numCards) * 100);
      $('#cardsShuffledCount').text("Shuffling progress: " + percentage + "%");
      $('#progressBar').css('width', percentage + '%');
      count++;
      if(count > numCards) {
        clearInterval(intervalId);
        $('#cardsShuffledCount').text("Shuffling complete!");
        $('#progressBar').css('width', '100%');
      }
    }, 1000);  // update the count every second
    device.callFunction("oppositeConstant", numCards)
      .then(function() {
          device.callFunction("stop");
      });
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

