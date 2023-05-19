$(document).ready(function () {
  var address = "172.20.10.8";
  var device = new Device(address);

  var shufflingSpeed = $('#shufflingSpeed').val();

  $('#shufflingSpeedValue').text("Shuffling Speed: " + shufflingSpeed);

  $('#shufflingSpeed').on('input change', function() {
      shufflingSpeed = $(this).val();
      $('#shufflingSpeedValue').text("Shuffling Speed: " + shufflingSpeed);
  });

  function calculateInterval(shufflingSpeed) {
    var baseSpeed = 200;  // this value seems to work well for your setup
    return baseSpeed / (shufflingSpeed / 800); // Adjust this value as per your need
}



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
    var intervalDelay = calculateInterval(shufflingSpeed);
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
    }, intervalDelay);
    runShuffler("oppositeConstant", numCards);
});

function resetProgressBar() {
  $('#cardsShuffledCount').text("");
  $('#progressBar').css('width', '0%');
}

$('#alternating').mousedown(function () {
  resetProgressBar();
  runShuffler("alternating");
});

$('#randomMotion').mousedown(function () {
  resetProgressBar();
  runShuffler("randomMotion");
});

$('#stop').mousedown(function () {
  resetProgressBar();
  device.callFunction("stop");
});

});
