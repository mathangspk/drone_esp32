#include "network/WebServerManager.h"
#include "debug/PlotterHTML.h"

WebServerManager::WebServerManager(const char *ssid, const char *password, 
                                 ESCController &escController, StatusLED &led, 
                                 MPU6500 &mpu6500, BatteryMonitor &batteryMonitor,
                                 CurrentMonitor &currentMonitor, BME280Sensor &bme,
                                 SimpleMahony &mahony, DronePIDController &pidController,
                                 IBusReceiver &receiver)
    : _ssid(ssid), _password(password), server(80), _esc(escController), 
      _statusLED(led), _mpu6500(mpu6500), _battery(batteryMonitor),
      _current(currentMonitor), _bme280(bme), _mahony(mahony), 
      _pidController(pidController), _receiver(receiver) {}

void WebServerManager::begin()
{
  WiFi.softAP(_ssid, _password);
  Serial.println("ESP32 Access Point IP: ");
  Serial.println(WiFi.softAPIP());
  setupRoutes();
  server.begin();
}

void WebServerManager::handleClient()
{
  server.handleClient();
}

void WebServerManager::setupRoutes()
{
  server.on("/", [this]()
            {
      String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>ESC Control</title>
  <style>
  body {
    font-family: Arial, sans-serif;
    background-color: #f2f2f2;
    padding: 10px;
    text-align: center;
    font-size: 14px;
  }
  h2, h3 {
    color: #333;
    margin: 5px 0;
    font-size: 16px;
  }
  input[type="range"] {
    width: 100%;
    margin: 5px 0;
  }
  .value-label {
    font-size: 18px;
    font-weight: bold;
    margin-bottom: 5px;
    display: block;
  }
  button {
    padding: 6px 10px;
    font-size: 12px;
    margin: 4px;
    border: none;
    border-radius: 6px;
    background-color: #4285f4;
    color: white;
    cursor: pointer;
  }
  button:hover {
    background-color: #3367d6;
  }
  .button-group {
    display: flex;
    flex-wrap: wrap;
    justify-content: center;
    gap: 4px;
  }
  .esc-group {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(160px, 1fr));
    gap: 10px;
    margin-top: 10px;
  }
  .esc-group > div {
    background: #fff;
    border-radius: 8px;
    padding: 6px;
    box-shadow: 0 0 4px rgba(0,0,0,0.1);
  }
  #sensorData, #batteryVoltage {
    margin-top: 8px;
    font-size: 13px;
  }
</style>

</head>
<body>
  <h2>ESC Control</h2>
   <!-- Slider chung cho tất cả ESC -->
  <input type="range" id="rangeSlider" min="0" max="100" value="0" oninput="sliderChanged(this.value)">
  <span id="setLabel" class="value-label">0%</span>

  <div class="button-group">
    <button onclick="changeValue(-1)">⬇ Down</button>
    <button onclick="changeValue(1)">⬆ Up</button>
    <button onclick="stopAll()">🛑 Stop All</button>
  </div>
<!-- Từng ESC -->
  <div class="esc-group">
  <div>
    <h3>ESC Front Left (FL)</h3>
    <input type="range" id="escFL" min="0" max="100" value="0" oninput="escSliderChanged('fl', this.value)">
    <span id="escFLLabel" class="value-label">0%</span>
    <div class="button-group">
      <button onclick="changeValueESC('fl', -1)">⬇ Down FL</button>
      <button onclick="changeValueESC('fl', 1)">⬆ Up FL</button>
    </div>
</div>

<div>
    <h3>ESC Front Right (FR)</h3>
    <input type="range" id="escFR" min="0" max="100" value="0" oninput="escSliderChanged('fr', this.value)">
    <span id="escFRLabel" class="value-label">0%</span>
    <div class="button-group">
      <button onclick="changeValueESC('fr', -1)">⬇ Down FR</button>
      <button onclick="changeValueESC('fr', 1)">⬆ Up FR</button>
    </div>
</div>

<div>
    <h3>ESC Rear Left (RL)</h3>
    <input type="range" id="escRL" min="0" max="100" value="0" oninput="escSliderChanged('rl', this.value)">
    <span id="escRLLabel" class="value-label">0%</span>
    <div class="button-group">
      <button onclick="changeValueESC('rl', -1)">⬇ Down RL</button>
      <button onclick="changeValueESC('rl', 1)">⬆ Up RL</button>
    </div>
    </div>
<div>
    <h3>ESC Rear Right (RR)</h3>
    <input type="range" id="escRR" min="0" max="100" value="0" oninput="escSliderChanged('rr', this.value)">
    <span id="escRRLabel" class="value-label">0%</span>
    <div class="button-group">
      <button onclick="changeValueESC('rr', -1)">⬇ Down RR</button>
      <button onclick="changeValueESC('rr', 1)">⬆ Up RR</button>
    </div>
  </div>
</div>

  <h3>MPU6500 Sensor Data</h3>
  <div id="sensorData">
    Loading...
  </div>

  <div id="batteryVoltage">Voltage: -- V</div>
  <div id="currentESC">Current: -- A</div>
  <div id="bme280Temp">Temp: -- C</div>
  <div id="bme280Pressure">Pressure: -- Mpa</div>
  <div id="bmeHumidity">Humidity: -- %</div>
  <h3>Attitude Estimation</h3>
  <div id="attitudeData">Roll: --°, Pitch: --°, Yaw: --°</div>

  <script>
    let setValue = 0;

    function sliderChanged(val) {
      setValue = parseInt(val);
      updateUI();
      sendValues();
    }

    function changeValue(delta) {
      setValue = Math.min(100, Math.max(0, setValue + delta));
      updateUI();
      sendValues('all');
    }

    function updateUI() {
      document.getElementById("rangeSlider").value = setValue;
      document.getElementById("setLabel").innerText = setValue + "%";
    }

    function sendValues(group) {
      const params = new URLSearchParams();
      if (group === 'all') {
        params.append("fl", setValue);
        params.append("fr", setValue);
        params.append("rl", setValue);
        params.append("rr", setValue);
        }
      fetch("/set?" + params.toString())
        .then(res => res.text())
        .then(txt => console.log("Updated:", txt));
    }
    function stopAll() {
      fetch("/stop").then(() => {
        setValue = 0;
        updateUI();
      });
    }

    // Chức năng cho từng ESC riêng biệt
    function escSliderChanged(esc, val) {
      let escValue = parseInt(val);
      updateEscUI(esc, escValue);
      sendEscValues(esc, escValue);
    }

    function changeValueESC(esc, delta) {
      let currentValue = parseInt(document.getElementById(`esc${esc.toUpperCase()}Label`).innerText);
      let newValue = Math.min(100, Math.max(0, currentValue + delta));
      updateEscUI(esc, newValue);
      sendEscValues(esc, newValue);
    }

    // Cập nhật giao diện từng ESC
    function updateEscUI(esc, value) {
      document.getElementById(`esc${esc.toUpperCase()}Label`).innerText = value + "%";
      document.getElementById(`esc${esc.toUpperCase()}`).value = value;
    }

    function sendEscValues(esc, value) {
      const params = new URLSearchParams();
      params.append(esc, value);
      fetch("/set?" + params.toString())
        .then(res => res.text())
        .then(txt => console.log(`Updated ${esc}:`, txt));
    }

    function fetchSensorData() {
      fetch("/sensor")
        .then(res => res.json())
        .then(data => {
          document.getElementById("sensorData").innerHTML = `
            <strong>Accel:</strong> X=${data.ax.toFixed(2)}g, Y=${data.ay.toFixed(2)}g, Z=${data.az.toFixed(2)}g<br>
            <strong>Gyro:</strong> X=${data.gx.toFixed(2)}°/s, Y=${data.gy.toFixed(2)}°/s, Z=${data.gz.toFixed(2)}°/s<br>
            <strong>Temp:</strong> ${data.temp.toFixed(2)}°C
          `;
        });
  }

  function updateBattery() {
      fetch("/battery")
          .then(res => res.json())
          .then(data => {
              document.getElementById("batteryVoltage").innerText = `Voltage: ${data.voltage} V`;
          });
  }

   function updateCurrent() {
      fetch("/current")
          .then(res => res.json())
          .then(data => {
              document.getElementById("currentESC").innerText = `Current: ${data.currentESC} A`;
          });
  }

    function updateBme280() {
      fetch("/bme")
          .then(res => res.json())
          .then(data => {
              document.getElementById("bme280Pressure").innerText = `Pressure: ${data.bme280Pressure} Mpa`;
          });
  }

  function updateAttitude() {
  fetch("/attitude")
    .then(res => res.json())
    .then(data => {
      document.getElementById("attitudeData").innerText =
        `Roll: ${data.roll.toFixed(2)}°, Pitch: ${data.pitch.toFixed(2)}°, Yaw: ${data.yaw.toFixed(2)}°`;
    });
}

  setInterval(updateBattery, 1000);  // Cập nhật mỗi 1 giây
  setInterval(updateCurrent, 1000);  // Cập nhật mỗi 1 giây
  setInterval(fetchSensorData, 500);
  setInterval(updateBme280, 2000);
  setInterval(updateAttitude, 500);

  </script>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html); });

  server.on("/set", [this]()
            {
        if (server.hasArg("fl")) _esc.setESCValue(ESC_FL, server.arg("fl").toInt());
        if (server.hasArg("fr")) _esc.setESCValue(ESC_FR, server.arg("fr").toInt());
        if (server.hasArg("rl")) _esc.setESCValue(ESC_RL, server.arg("rl").toInt());
        if (server.hasArg("rr")) _esc.setESCValue(ESC_RR, server.arg("rr").toInt());
        _statusLED.toggle();
        Serial.printf("Updated ESC values: FL=%d, FR=%d, RL=%d, RR=%d\n",
                      _esc.getCurrentValue(ESC_FL),
                      _esc.getCurrentValue(ESC_FR),
                      _esc.getCurrentValue(ESC_RL),
                      _esc.getCurrentValue(ESC_RR));
        server.send(200, "text/plain", "OK"); });

  server.on("/stop", [this]()
            {
        _esc.stopAll();
        _statusLED.toggle();
        Serial.println("All ESC stopped");
        server.send(200, "text/plain", "Stopped all ESCs"); });

  server.on("/sensor", [this]()
            {
        SensorData data = _mpu6500.getData();
        String json = "{";
        json += "\"ax\":" + String(data.ax, 2) + ",";
        json += "\"ay\":" + String(data.ay, 2) + ",";
        json += "\"az\":" + String(data.az, 2) + ",";
        json += "\"gx\":" + String(data.gx, 2) + ",";
        json += "\"gy\":" + String(data.gy, 2) + ",";
        json += "\"gz\":" + String(data.gz, 2) + ",";
        json += "\"temp\":" + String(data.temp, 2);
        json += "}";
        server.send(200, "application/json", json); });

  server.on("/battery", [this]()
            {
      float voltage = _battery.getVoltage();
      String json = "{\"voltage\":" + String(voltage, 2) + "}";
      server.send(200, "application/json", json); });

   server.on("/current", [this]()
            {
      float current = _current.getCurrent();
      String json = "{\"currentESC\":" + String(current, 2) + "}";
      server.send(200, "application/json", json); });

    server.on("/bme", [this]()
      {
      float pressure = _bme280.readPressure();
      String json = "{\"bme280Pressure\":" + String(pressure, 2) + "}";
      server.send(200, "application/json", json); });

  // Thêm endpoint cho attitude data
  server.on("/attitude", [this]()
      {
      String json = "{";
      json += "\"roll\":" + String(_mahony.getRollDegrees(), 2) + ",";
      json += "\"pitch\":" + String(_mahony.getPitchDegrees(), 2) + ",";
      json += "\"yaw\":" + String(_mahony.getYawDegrees(), 2);
      json += "}";
      server.send(200, "application/json", json); });

  // Thêm endpoint cho receiver data
  server.on("/receiver", [this]()
      {
      String json = "{";
      json += "\"throttle\":" + String(_receiver.get_throttle()) + ",";
      json += "\"roll\":" + String(_receiver.get_roll()) + ",";
      json += "\"pitch\":" + String(_receiver.get_pitch()) + ",";
      json += "\"yaw\":" + String(_receiver.get_yaw()) + ",";
      json += "\"aux1\":" + String(_receiver.get_aux1()) + ",";
      json += "\"aux2\":" + String(_receiver.get_aux2());
      json += "}";
      server.send(200, "application/json", json); });

  // Thêm endpoint cho PID data
  server.on("/pid", [this]()
      {
      String json = "{";
      json += "\"rollPID\":" + String(_pidController.getRollOutput(), 2) + ",";
      json += "\"pitchPID\":" + String(_pidController.getPitchOutput(), 2) + ",";
      json += "\"yawPID\":" + String(_pidController.getYawOutput(), 2) + ",";
      json += "\"rollSetpoint\":0.0,";  // Cần thêm setpoint vào DronePIDController
      json += "\"pitchSetpoint\":0.0,";
      json += "\"yawSetpoint\":0.0";
      json += "}";
      server.send(200, "application/json", json); });

  // Thêm endpoint cho motor data
  server.on("/motors", [this]()
      {
      String json = "{";
      json += "\"motor1\":" + String(_esc.getCurrentValue(ESC_FL)) + ",";
      json += "\"motor2\":" + String(_esc.getCurrentValue(ESC_FR)) + ",";
      json += "\"motor3\":" + String(_esc.getCurrentValue(ESC_RL)) + ",";
      json += "\"motor4\":" + String(_esc.getCurrentValue(ESC_RR));
      json += "}";
      server.send(200, "application/json", json); });

  // Thêm endpoint cho plotter
  server.on("/plotter", [this]()
      {
      server.send(200, "text/html", PLOTTER_HTML); });

  // Thêm endpoint cho tất cả dữ liệu debug
  server.on("/debug", [this]()
      {
      SensorData sensorData = _mpu6500.getData();
      String json = "{";
      json += "\"timestamp\":" + String(millis()) + ",";
      json += "\"sensor\":{";
      json += "\"ax\":" + String(sensorData.ax, 2) + ",";
      json += "\"ay\":" + String(sensorData.ay, 2) + ",";
      json += "\"az\":" + String(sensorData.az, 2) + ",";
      json += "\"gx\":" + String(sensorData.gx, 2) + ",";
      json += "\"gy\":" + String(sensorData.gy, 2) + ",";
      json += "\"gz\":" + String(sensorData.gz, 2) + ",";
      json += "\"temp\":" + String(sensorData.temp, 2);
      json += "},";
      json += "\"attitude\":{";
      json += "\"roll\":" + String(_mahony.getRollDegrees(), 2) + ",";
      json += "\"pitch\":" + String(_mahony.getPitchDegrees(), 2) + ",";
      json += "\"yaw\":" + String(_mahony.getYawDegrees(), 2);
      json += "},";
      json += "\"receiver\":{";
      json += "\"throttle\":" + String(_receiver.get_throttle()) + ",";
      json += "\"roll\":" + String(_receiver.get_roll()) + ",";
      json += "\"pitch\":" + String(_receiver.get_pitch()) + ",";
      json += "\"yaw\":" + String(_receiver.get_yaw()) + ",";
      json += "\"aux1\":" + String(_receiver.get_aux1()) + ",";
      json += "\"aux2\":" + String(_receiver.get_aux2());
      json += "},";
      json += "\"pid\":{";
      json += "\"rollPID\":" + String(_pidController.getRollOutput(), 2) + ",";
      json += "\"pitchPID\":" + String(_pidController.getPitchOutput(), 2) + ",";
      json += "\"yawPID\":" + String(_pidController.getYawOutput(), 2) + ",";
      json += "\"rollSetpoint\":0.0,";  // Cần thêm setpoint vào DronePIDController
      json += "\"pitchSetpoint\":0.0,";
      json += "\"yawSetpoint\":0.0";
      json += "},";
      json += "\"motors\":{";
      json += "\"motor1\":" + String(_esc.getCurrentValue(ESC_FL)) + ",";
      json += "\"motor2\":" + String(_esc.getCurrentValue(ESC_FR)) + ",";
      json += "\"motor3\":" + String(_esc.getCurrentValue(ESC_RL)) + ",";
      json += "\"motor4\":" + String(_esc.getCurrentValue(ESC_RR));
      json += "},";
      json += "\"system\":{";
      json += "\"voltage\":" + String(_battery.getVoltage(), 2) + ",";
      json += "\"current\":" + String(_current.getCurrent(), 2) + ",";
      json += "\"pressure\":" + String(_bme280.readPressure(), 2);
      json += "}";
      json += "}";
      server.send(200, "application/json", json); });
  
}
