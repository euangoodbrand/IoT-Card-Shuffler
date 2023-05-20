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
    return (baseSpeed / (shufflingSpeed / 800)) / 5; 
  }
  

function calculateTime(numCards, shufflingSpeed) {
  var baseTime = numCards * (200 / 1000); // Time for speed 200
  return baseTime * (200 / shufflingSpeed); // Adjusted time for current speed
}


function runShuffler(functionName, numCards, calculatedTime) {
    var parameters = shufflingSpeed + "," + calculatedTime;
    device.callFunction(functionName, parameters)
        .then(function() {
            device.callFunction("stop");
        });
}

$('#oppositeConstant').click(function (event) {
  event.preventDefault();
  var numCards = $('#cardsNumber').val();
  var intervalDelay = calculateInterval(shufflingSpeed);
  var calculatedTime = calculateTime(numCards, shufflingSpeed);
  var count = 0;
  var intervalId = setInterval(function() {
    var percentage = Math.round((count / numCards) * 100);
    $('#shufflingProgress').text("Shuffling progress: " + percentage + "%");
    $('#cardsShuffledCount').text("Cards shuffled: " + count);
    $('#progressBar').css('width', percentage + '%');
    count++;
    if(count > numCards) {
      clearInterval(intervalId);
      $('#shufflingProgress').text("Shuffling complete!");
      $('#cardsShuffledCount').text("Cards shuffled: " + numCards);
      $('#progressBar').css('width', '100%');
    }
  }, intervalDelay);
  runShuffler("oppositeConstant", numCards, calculatedTime);
});



$('#alternating').mousedown(function () {
  resetProgressBar();
  var numCards = $('#cardsNumber').val();
  var intervalDelay = calculateInterval(shufflingSpeed);
  var calculatedTime = calculateTime(numCards, shufflingSpeed);
  var count = 0;
  var intervalId = setInterval(function() {
    var percentage = Math.round((count / numCards) * 100);
    $('#shufflingProgress').text("Shuffling progress: " + percentage + "%");
    $('#cardsShuffledCount').text("Cards shuffled: " + count);
    $('#progressBar').css('width', percentage + '%');
    count++;
    if(count > numCards) {
      clearInterval(intervalId);
      $('#shufflingProgress').text("Shuffling complete!");
      $('#cardsShuffledCount').text("Cards shuffled: " + numCards);
      $('#progressBar').css('width', '100%');
    }
  }, intervalDelay);
  runShuffler("alternating", numCards, calculatedTime);
});

$('#randomMotion').mousedown(function () {
  resetProgressBar();
  var numCards = $('#cardsNumber').val();
  var intervalDelay = calculateInterval(shufflingSpeed);
  var calculatedTime = calculateTime(numCards, shufflingSpeed);
  var count = 0;
  var intervalId = setInterval(function() {
    var percentage = Math.round((count / numCards) * 100);
    $('#shufflingProgress').text("Shuffling progress: " + percentage + "%");
    $('#cardsShuffledCount').text("Cards shuffled: " + count);
    $('#progressBar').css('width', percentage + '%');
    count++;
    if(count > numCards) {
      clearInterval(intervalId);
      $('#shufflingProgress').text("Shuffling complete!");
      $('#cardsShuffledCount').text("Cards shuffled: " + numCards);
      $('#progressBar').css('width', '100%');
    }
  }, intervalDelay);
  runShuffler("randomMotion", numCards, calculatedTime);
});

$('#customShuffle').click(function (event) {
  event.preventDefault();
  var numLeftCards = $('#leftCardsNumber').val();
  var numRightCards = $('#rightCardsNumber').val();
  var numCards = $('#cardsNumber').val();
  var intervalDelay = calculateInterval(shufflingSpeed);
  var calculatedTime = calculateTime(numCards, shufflingSpeed); 
  var count = 0;
  var intervalId = setInterval(function() {
    var percentage = Math.round((count / numCards) * 100);
    $('#shufflingProgress').text("Shuffling progress: " + percentage + "%");
    $('#cardsShuffledCount').text("Cards shuffled: " + count);
    $('#progressBar').css('width', percentage + '%');
    count++;
    if(count > numCards) {
      clearInterval(intervalId);
      $('#shufflingProgress').text("Shuffling complete!");
      $('#cardsShuffledCount').text("Cards shuffled: " + numCards);
      $('#progressBar').css('width', '100%');
    }
  }, intervalDelay);
  var parameters = shufflingSpeed + "," + numCards + "," + numLeftCards + "," + numRightCards;
  device.callFunction("customShuffle", parameters)
    .then(function() {
      device.callFunction("stop");
    });
});





function resetProgressBar() {
  $('#cardsShuffledCount').text("");
  $('#progressBar').css('width', '0%');
}


$('#stop').mousedown(function () {
  resetProgressBar();
  device.callFunction("stop");
});

});
