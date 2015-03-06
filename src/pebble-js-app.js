var initialized = false;
var SET_PEBBLE_TOKEN = '2Q4U';

Pebble.addEventListener("ready", function() {
  console.log("ready called!");
  initialized = true;
});

Pebble.addEventListener('appmessage', function(e) {
  key = e.payload.action;
  if (typeof(key) != 'undefined') {
    var settings = localStorage.getItem(SET_PEBBLE_TOKEN);
    if (typeof(settings) == 'string') {
      try {
        Pebble.sendAppMessage(JSON.parse(settings));
      } catch (e) {
      }
    }
    var request = new XMLHttpRequest();
    request.open('GET', 'http://x.SetPebble.com/api/' + SET_PEBBLE_TOKEN + '/' + Pebble.getAccountToken(), true);
    request.onload = function(e) {
      if (request.readyState == 4)
        if (request.status == 200)
          try {
            Pebble.sendAppMessage(JSON.parse(request.responseText));
          } catch (e) {
          }
    };
    request.send(null);
  }
});

Pebble.addEventListener("showConfiguration", function() {
  Pebble.openURL('http://x.setpebble.com/' + SET_PEBBLE_TOKEN + '/' + Pebble.getAccountToken());
});

Pebble.addEventListener('webviewclosed', function(e) {
  if ((typeof(e.response) == 'string') && (e.response.length > 0)) {
    try {
      Pebble.sendAppMessage(JSON.parse(e.response));
      localStorage.setItem(SET_PEBBLE_TOKEN, e.response);
    } catch(e) {
    }
  }
});