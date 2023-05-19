$(document).ready(function () {
  var address = "172.20.10.8";
  var device = new Device(address);

  var shufflingSpeed = $('#shufflingSpeed').val();

  $('#shufflingSpeedValue').text("Shuffling Speed: " + shufflingSpeed);

  $('#shufflingSpeed').on('input change', function() {
      shufflingSpeed = $(this).val();
      $('#shufflingSpeedValue').text("Shuffling Speed: " + shufflingSpeed);
  });

  function runShuffler(functionName, numCards) {
    var parameters = shufflingSpeed + "," + numCards;
    device.callFunction(functionName, parameters)
        .then(function() {
            device.callFunction("stop");
        });
  }
  

  $('#oppositeConstantForm').submit(function (event) {
      event.preventDefault();
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
      }, 1000);
      runShuffler("oppositeConstant", numCards);
  });

  $('#alternating').mousedown(function () {
      runShuffler("alternating");
  });
  $('#alternating').mouseup(function () {
      device.callFunction("stop");
  });

  $('#randomMotion').mousedown(function () {
      runShuffler("randomMotion");
  });
  $('#randomMotion').mouseup(function () {
      device.callFunction("stop");
  });

  $('#stop').mousedown(function () {
      device.callFunction("stop");
  });
});
