const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
      <head>
            <meta charset="utf-8" name="viewport" content="width=device-width, initial-scale=1">
            <style>
                  body { text-align:center; }
                  .vert { margin-bottom: 10%; }
                  .hori{ margin-bottom: 0%; }
                  p{ text-align: left; }
            </style>
      </head>
      <body>
            <div id="container">
            <h1><b>SITUAÇÃO CLIMÁTICA ATUAL</b>:</h1>
            <p> --> Local: <b>Ribeirão Preto (região central) - SP - Brasil </b></p>
            <p> --><b>Dados atualizados a cada 10s</b></p>
            <p>Data: <b><span id="date_data">%DATE_DATA%</span></b></p>
            <p>Hora: <b><span id="hour_data">%HOUR_DATA%</span></b></p>
            <p>Temperatura: <b><span id="temp_data">%TEMP_DATA%</span> ºC</b></p>
            <p>Pressão: <b><span id="press_data">%PRESS_DATA%</span> hPa</b></p>
            <p>Umidade: <b><span id="humid_data">%HUMID_DATA%</span>%</b></p>
            <!-- <p>Nível de iluminação (0 a 4095): <b><span id="ldr_data">%LDR_DATA%</span></b></p> -->
            <p>Quantidade de lux: <b><span id="lux_data">%LUX_DATA%</span> lux</b></p>
            <br>
            
            <h1><b>ÚLTIMA FOTO TIRADA COM O ESP32-CAM</b></h1>
            <p> --> Para tirar uma foto, clique no botão <b>'TIRAR FOTO'</b>.</p>
            <p> --> Geralmente o processo de tirar a foto e atualizar no site é <b>maior que 10 segundos.</b>.</p>
            <p> --> Depois de 15 a 20 segundos após tirar a foto, clicar no botão <b>ATUALIZAR PÁGINA</b>.</p>
            <p>
            <button onclick="rotatePhoto();">GIRAR FOTO</button>
            <button onclick="capturePhoto()">TIRAR FOTO</button>
            <button onclick="location.reload();">ATUALIZAR PÁGINA</button>
            </p>
            </div>
            <div><img src="saved-photo" id="photo" width="90%"></div>
      </body>

      <!-- Script em JS para as atualizacoes dos dados -->
      <script>
            setInterval(function ( ) {
                  var xhttp = new XMLHttpRequest();
                  xhttp.onreadystatechange = function() {
                        if (this.readyState == 4 && this.status == 200) {
                              document.getElementById("date_data").innerHTML = this.responseText;
                        }
                  };
                  xhttp.open("GET", "/date_data", true);
                  xhttp.send();
            }, 10000 );
            
            setInterval(function ( ) {
                  var xhttp = new XMLHttpRequest();
                  xhttp.onreadystatechange = function() {
                        if (this.readyState == 4 && this.status == 200) {
                              document.getElementById("hour_data").innerHTML = this.responseText;
                        }
                  };
                  xhttp.open("GET", "/hour_data", true);
                  xhttp.send();
            }, 10000 ) ;
            
            setInterval(function ( ) {
                  var xhttp = new XMLHttpRequest();
                  xhttp.onreadystatechange = function() {
                        if (this.readyState == 4 && this.status == 200) {
                              document.getElementById("temp_data").innerHTML = this.responseText;
                        }
                  };
                  xhttp.open("GET", "/temp_data", true);
                  xhttp.send();
            }, 10000 ) ;
            
            setInterval(function ( ) {
                  var xhttp = new XMLHttpRequest();
                  xhttp.onreadystatechange = function() {
                        if (this.readyState == 4 && this.status == 200) {
                              document.getElementById("press_data").innerHTML = this.responseText;
                        }
                  };
                  xhttp.open("GET", "/press_data", true);
                  xhttp.send();
            }, 10000 ) ;
            
            setInterval(function ( ) {
                  var xhttp = new XMLHttpRequest();
                  xhttp.onreadystatechange = function() {
                        if (this.readyState == 4 && this.status == 200) {
                              document.getElementById("humid_data").innerHTML = this.responseText;
                        }
                  };
                  xhttp.open("GET", "/humid_data", true);
                  xhttp.send();
            }, 10000 ) ;
            
            setInterval(function ( ) {
                  var xhttp = new XMLHttpRequest();
                  xhttp.onreadystatechange = function() {
                        if (this.readyState == 4 && this.status == 200) {
                              document.getElementById("ldr_data").innerHTML = this.responseText;
                        }
                  };
                  xhttp.open("GET", "/ldr_data", true);
                  xhttp.send();
            }, 10000 ) 
            
            setInterval(function ( ) {
                  var xhttp = new XMLHttpRequest();
                  xhttp.onreadystatechange = function() {
                        if (this.readyState == 4 && this.status == 200) {
                              document.getElementById("lux_data").innerHTML = this.responseText;
                        }
                  };
                  xhttp.open("GET", "/lux_data", true);
                  xhttp.send();
            }, 10000 ) 
            
            setInterval(function ( ) {
                  var xhttp = new XMLHttpRequest();
                  xhttp.onreadystatechange = function() {
                        if (this.readyState == 4 && this.status == 200) {
                              document.getElementById("relay_data").innerHTML = this.responseText;
                        }
                  };
                  xhttp.open("GET", "/relay_data", true);
                  xhttp.send();
            }, 10000 )
            
            var deg = 0;
            function capturePhoto() {
                  var xhr = new XMLHttpRequest();
                  xhr.open('GET', "/capture", true);
                  xhr.send();
            }
            function rotatePhoto() {
                  var img = document.getElementById("photo");
                  deg += 90;
                  if(isOdd(deg/90)){ document.getElementById("container").className = "vert"; }
                  else{ document.getElementById("container").className = "hori"; }
                  img.style.transform = "rotate(" + deg + "deg)";
            }
            
            function isOdd(n) { return Math.abs(n % 2) == 1; }
            
      </script>
</html>)rawliteral";
