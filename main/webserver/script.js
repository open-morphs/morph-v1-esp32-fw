function alert(words){
    var iframe = document.createElement("IFRAME");
    iframe.setAttribute("src", 'data:text/plain,');
    document.documentElement.appendChild(iframe);
    window.frames[0].window.alert(words);
    iframe.parentNode.removeChild(iframe);
}

function validateSsid() {
    var ssid = document.forms["wifiForm"]["ssid"].value;

    if (ssid === "") {
        alert("WiFi ssid field can't be empty!");

        return false;
    }

    return true;
}

function getSettings() {
    var xhr = new XMLHttpRequest();
    var requestURL = "/status";
    xhr.open('GET', requestURL, true);
    xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

    xhr.onreadystatechange = function() { // Call a function when the state changes.
        if (this.readyState === XMLHttpRequest.DONE && this.status === 200) {
            var response = JSON.parse(xhr.responseText);

            var langsel = document.getElementById('langsel');
            langsel.selectedIndex = response.lang;

            var imagesel = document.getElementById('imagesel');
            imagesel.selectedIndex = response.image;

            var checkBox = document.getElementById('beepCheckBox');
            checkBox.checked = response.beep;

            // var ssidInput = document.getElementById('ssidInput');
            // var passInput = document.getElementById('passInput');

            // ssidInput.value = response.ssid
            // passInput.value = response.pass
        }
    }
    xhr.send();
}