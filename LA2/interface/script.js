// This block of code waits for the page to finish loading
$(document).ready(function () {

  // Initialize the IP address of the device
  var address = "172.20.10.8";

  // Create a new instance of the Device class
  var device = new Device(address);

  // Get the current shuffling speed from the input
  var shufflingSpeed = $('#shufflingSpeed').val();

  // Set the initial text of the shuffling speed display
  $('#shufflingSpeedValue').text("Shuffling Speed: " + shufflingSpeed);

  // Update shufflingSpeed variable and the display whenever the speed input changes
  $('#shufflingSpeed').on('input change', function() {
    shufflingSpeed = $(this).val();
    $('#shufflingSpeedValue').text("Shuffling Speed: " + shufflingSpeed);
  });

  // Function to calculate the interval based on the shuffling speed
  function calculateInterval(shufflingSpeed) {
    var baseSpeed = 200;  
    return (baseSpeed / (shufflingSpeed / 800)) / 5; 
  }
  
  // Function to calculate the time based on the number of cards and the shuffling speed
  function calculateTime(numCards, shufflingSpeed) {
    var baseTime = numCards * (200 / 1000); // Time for speed 200
    return baseTime * (200 / shufflingSpeed); // Adjusted time for current speed
  }

  // Function to run the shuffling function on the device and stop it once it is done
  function runShuffler(functionName, numCards, calculatedTime) {
    var parameters = shufflingSpeed + "," + calculatedTime;
    device.callFunction(functionName, parameters)
        .then(function() {
            device.callFunction("stop");
        });
  }

  // Click event for the Opposite Constant button
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

  // Mousedown event for the Alternating button
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

  // Mousedown event for the Random Motion button
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

  // Click event for the Custom Shuffle button
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

  // Function to reset the progress bar and the shuffled cards count
  function resetProgressBar() {
    $('#cardsShuffledCount').text("");
    $('#progressBar').css('width', '0%');
  }

  // Mousedown event for the Stop button
  $('#stop').mousedown(function () {
    resetProgressBar();
    device.callFunction("stop");
  });
});
