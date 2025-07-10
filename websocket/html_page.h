/*
PROGMEM is used to store the HTML page in flash memory on an ESP32. 
Because the ESP32 has limited RAM, storing large strings in flash memory helps to conserve RAM for other tasks.
*/
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>Web Server ESP32</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.7.2/css/all.min.css" />
    <link rel="stylesheet" href="https://fonts.googleapis.com/css2?family=Montserrat:wght@400;600;700&display=swap" />
    <style>
      :root {
        --white-color: #fff;
        --primary-color: #002a5a;
        --secondary-color: #0053b2;
        --footer-color: #192e40;
        --site-max-width: 1300px;
        --font-size-l: 1.5rem;
      }
      * {
        box-sizing: border-box;
        margin: 0;
        padding: 0;
        font-family: "Montserrat", sans-serif;
      }
      html,
      body {
        height: 100%;
        display: flex;
        flex-direction: column;
      }
      main {
        flex: 1;
      }
      a {
        text-decoration: none;
      }
      .section-content {
        max-width: var(--site-max-width);
        margin: 0 auto;
        padding: 0rem 2rem;
      }
      header {
        background-color: var(--primary-color);
        color: var(--white-color);
        display: flex;
        flex-direction: column;
        align-items: center;
        padding: 20px;
        gap: 10px;
        text-align: center;
      }

      .section-content .grid-container {
        display: grid;
        grid-template-columns: repeat(auto-fit, minmax(240px, 1fr));
        gap: 30px;
        padding: 2rem;
      }
      .section-content .grid-container .card {
        border-radius: 10px;
        padding: 10px;
        box-shadow: 4px 4px 4px 4px rgba(0, 0, 0, 0.1);
        text-align: center;
        height: 150px;
        display: flex;
        flex-direction: column;
        justify-content: center;
        align-items: center;
        gap: 10px;
      }
      .switch {
        position: relative;
        display: inline-block;
        width: 80px;
        height: 40px;
      }
      .switch input {
        opacity: 0;
        width: 0;
        height: 0;
      }
      .slider {
        position: absolute;
        cursor: pointer;
        top: 0;
        left: 0;
        right: 0;
        bottom: 0;
        background-color: #ccc;
        -webkit-transition: 0.4s;
        transition: 0.4s;
        border-radius: 40px;
      }
      .slider:before {
        position: absolute;
        content: "";
        height: 30px;
        width: 30px;
        left: 8px;
        bottom: 5px;
        background-color: #fff;
        -webkit-transition: 0.4s;
        transition: 0.4s;
        border-radius: 40px;
      }
      input:checked + .slider {
        background-color: #2196f3;
      }
      input:checked + .slider:before {
        -webkit-transform: translateX(30px);
        -ms-transform: translateX(30px);
        transform: translateX(30px);
      }
      .title {
        font-size: var(--font-size-l);
        font-weight: bold;
      }
      .ledOn {
        color: green;
      }
      .ledOff {
        color: red;
      }
      .grid-container .temp-umid-card {
        color: var(--secondary-color);
      }
      .grid-container .light-card {
        color: green;
      }
      input[type="range"] {
        width: 100%;
        margin: 10px 0;
        border-radius: 20px;
        -webkit-appearance: none;
        appearance: none;
        background: #ddd;
        height: 10px;
        cursor: pointer;
        outline: none;
      }
      input[type="range"]::-webkit-slider-thumb {
        -webkit-appearance: none;
        appearance: none;
        width: 20px;
        height: 20px;
        border-radius: 50%;
        background: green;
        cursor: pointer;
      }
      input[type="range"]::-moz-range-thumb {
        width: 20px;
        height: 20px;
        border-radius: 50%;
        background: green;
        cursor: pointer;
      }
      .footer-section {
        background-color: var(--footer-color);
        color: var(--white-color);
        padding: 20px 0;
      }
      .footer-section .section-content {
        display: flex;
        align-items: center;
        justify-content: space-between;
      }
      .footer-section :where(.copyright-text, .social-link, .dev-link) {
        color: var(--white-color);
      }
      .footer-section .social-link-list {
        display: flex;
        gap: 20px;
      }
      .footer-section .social-link-list .social-link {
        font-size: var(--font-size-l);
      }
      @media screen and (max-width: 820px) {
        .footer-section .section-content {
          flex-direction: column;
          gap: 10px;
          text-align: center;
        }
      }
    </style>
  </head>
  <body>
    <header>
      <img src="https://embarcados.com.br/wp-content/uploads/2020/02/logo-238x76-1.png.webp" alt="logo_embarcados" />
      <h1>ESP32-C6 Web Server</h1>
    </header>
    <main>
      <section class="section-content">
        <div class="grid-container">
          <!-- CARD LED -->
          <div class="card">
            <p>
              <i class="fas fa-lightbulb fa-2x ledOff" id="iconLED18"></i>
              <strong class="title">(LED) 18</strong>
            </p>
            <p>GPIO state: <strong id="estadoLED18">Carregando...</strong></p>
            <label class="switch">
              <input type="checkbox" onchange="toggleLed(this)" id="led18" />
              <span class="slider"></span>
            </label>
          </div>

          <!-- Temperatura -->
          <div class="card temp-umid-card">
            <p>
              <i class="fas fa-thermometer-half fa-2x"></i>
              <strong class="title">Temperatura</strong>
            </p>
            <p class="title"><span>Carregando...</span>°C</p>
          </div>

          <!-- Umidade -->
          <div class="card temp-umid-card">
            <p>
              <i class="fas fa-water fa-2x"></i>
              <strong class="title">Umidade</strong>
            </p>
            <p class="title"><span>Carregando...</span>%</p>
          </div>

          <!-- Luz (LDR) -->
          <div class="card light-card">
            <p>
              <i class="fas fa-sun fa-2x"></i>
              <strong class="title">Luz</strong>
            </p>
            <p class="title"><span>Carregando...</span></p>
          </div>

          <!-- PWM -->
          <div class="card light-card">
            <p>
              <i class="fas fa-sun fa-2x"></i>
              <strong class="title">PWM</strong>
            </p>
            <input type="range" id="pwmSlider" min="0" max="255" value="0" onchange="setPWM(this.value)" oninput="atualizarPWM(this.value)" />
            <p class="title"><span id="pwmValue">0</span></p>
          </div>
        </div>
      </section>
    </main>
    <footer class="footer-section">
      <div class="section-content">
        <p class="copyright-text">&copy; Embarcados - Todos os direitos reservados</p>
        <div class="social-link-list">
          <a href="https://www.linkedin.com/company/embarcados" class="social-link" target="_blank" rel="noopener noreferrer" referrerpolicy="no-referrer">
            <i class="fa-brands fa-linkedin"></i>
          </a>
          <a href="https://www.instagram.com/portalembarcados" class="social-link">
            <i class="fa-brands fa-instagram"></i>
          </a>
        </div>
        <p class="dev-text">
          <a href="https://www.linkedin.com/in/iguilherme" class="dev-link">Desenvolvido por Guilherme Fernandes</a>
        </p>
      </div>
    </footer>

    <script>
      let websocket = new WebSocket(`ws://${window.location.hostname}/ws`);
      websocket.onopen = () => console.log("Connection established");
      websocket.onclose = () => console.log("Connection died");
      websocket.onerror = (error) => console.log("error");
      websocket.onmessage = (event) => {
        console.log("Message received: " + event.data);
        const partes = event.data.split(";");
        partes.forEach((p) => {
          const [key, val] = p.split("=");
          switch (key) {
            case "LED":
              console.log("LED state: " + val);
              atualizarEstado(val);
              break;
            case "TEMP":
              document.querySelector(".temp-umid-card span").innerText = val;
              break;
            case "UMID":
              document.querySelectorAll(".temp-umid-card span")[1].innerText = val;
              break;
            case "LDR":
              document.querySelector(".light-card span").innerText = val;
              break;
            case "PWM":
              document.getElementById("pwmValue").innerText = val;
              document.getElementById("pwmSlider").value = val;
              break;
          }
        });
      };
      function atualizarPWM(val) {
        document.getElementById("pwmValue").innerText = val;
      }
      function setPWM(val) {
        if (websocket && websocket.readyState === WebSocket.OPEN) {
          websocket.send("PWM=" + val);
          console.log("PWM enviado: " + val);
        } else {
          console.warn("WebSocket não conectado");
        }
      }
      function toggleLed(element) {
        if (websocket && websocket.readyState === WebSocket.OPEN) {
          const comando = element.checked ? "on" : "off";
          websocket.send(comando);
          console.log("Comando enviado via WebSocket: " + comando);
        } else {
          console.warn("WebSocket não conectado");
        }
      }
      function atualizarEstado(estado) {
        const estadoLED = document.getElementById("estadoLED18");
        const iconLED = document.getElementById("iconLED18");

        estadoLED.innerText = estado == "on" ? "ON" : "OFF";
        iconLED.classList.remove("ledOff", "ledOn");
        iconLED.classList.add(estado === "on" ? "ledOn" : "ledOff");

        document.getElementById("led18").checked = estado === "on";
      }
    </script>
  </body>
</html>
)rawliteral";