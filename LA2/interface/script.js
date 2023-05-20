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
    var baseSpeed = 200;  
    return baseSpeed / (shufflingSpeed / 800); 
}

function calculateTime(numCards, shufflingSpeed) {
  var baseTime = numCards * (200 / 200); // Time for speed 200
  return baseTime * (200 / shufflingSpeed); // Adjusted time for current speed
}


function runShuffler(functionName, numCards, calculatedTime) {
    var parameters = shufflingSpeed + "," + calculatedTime;
    device.callFunction(functionName, parameters)
        .then(function() {
            device.callFunction("stop");
        });
}

$('#oppositeConstantForm').submit(function (event) {
  event.preventDefault();
  var numCards = $('#cardsNumber').val();
  var intervalDelay = calculateInterval(shufflingSpeed);
  var calculatedTime = calculateTime(numCards, shufflingSpeed);
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
  }, intervalDelay);
  runShuffler("oppositeConstant", numCards, calculatedTime);
});



function resetProgressBar() {
  $('#cardsShuffledCount').text("");
  $('#progressBar').css('width', '0%');
}

$('#alternating').mousedown(function () {
  resetProgressBar();
  var numCards = $('#cardsNumber').val();
  var calculatedTime = calculateTime(numCards, shufflingSpeed);
  runShuffler("alternating", numCards, calculatedTime);
});

$('#randomMotion').mousedown(function () {
  resetProgressBar();
  var numCards = $('#cardsNumber').val();
  var calculatedTime = calculateTime(numCards, shufflingSpeed);
  runShuffler("randomMotion", numCards, calculatedTime);
});


$('#stop').mousedown(function () {
  resetProgressBar();
  device.callFunction("stop");
});

});
