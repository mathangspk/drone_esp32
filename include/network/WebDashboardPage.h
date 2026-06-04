#ifndef WEBDASHBOARDPAGE_H
#define WEBDASHBOARDPAGE_H

const char* const kDashboardHTML = R"rawhtml(<!DOCTYPE html>
<html><head><title>ESP32 Drone Dashboard</title>
<meta name="viewport" content="width=device-width, initial-scale=1">
<style>
body { font-family: sans-serif; background: #121212; color: #e0e0e0; margin: 20px; }
h1, h2 { color: #00e676; }
.card { background: #1e1e1e; padding: 15px; border-radius: 8px; margin-bottom: 15px; }
.row { display: flex; flex-wrap: wrap; gap: 10px; margin-bottom: 10px; align-items: center; }
label { display: inline-block; width: 60px; }
input[type=number] { width: 60px; background: #333; color: #fff; border: 1px solid #555; padding: 4px; }
button { background: #00e676; color: #000; border: none; padding: 8px 16px; border-radius: 4px; cursor: pointer; font-weight: bold; }
button:hover { background: #00b359; }
textarea { width: 100%; height: 150px; background: #222; color: #00ff00; border: 1px solid #444; font-family: monospace; }
.bar { background: #333; height: 18px; border-radius: 4px; overflow: hidden; margin-top: 4px; width: 200px; }
.fill { background: #00e676; height: 100%; width: 0%; transition: width 0.1s; }
</style></head>
<body>
<h1>ESP32 Drone Dashboard</h1>
<div class="card">
  <h2>Tuning PID Parameters</h2>
  <form id="pidForm">
    <div class="row"><b>Rate Roll:</b> Kp<input type="number" step="0.001" id="r_kp"> Ki<input type="number" step="0.001" id="r_ki"> Kd<input type="number" step="0.001" id="r_kd"></div>
    <div class="row"><b>Rate Pitch:</b> Kp<input type="number" step="0.001" id="p_kp"> Ki<input type="number" step="0.001" id="p_ki"> Kd<input type="number" step="0.001" id="p_kd"></div>
    <div class="row"><b>Rate Yaw:</b> Kp<input type="number" step="0.001" id="y_kp"> Ki<input type="number" step="0.001" id="y_ki"> Kd<input type="number" step="0.001" id="y_kd"></div>
    <div class="row"><b>Angle Roll:</b> Kp<input type="number" step="0.1" id="ra_kp"> Kd<input type="number" step="0.1" id="ra_kd"></div>
    <div class="row"><b>Angle Pitch:</b> Kp<input type="number" step="0.1" id="pa_kp"> Kd<input type="number" step="0.1" id="pa_kd"></div>
    <button type="button" onclick="savePID()">Save PID</button>
  </form>
</div>
<div class="card">
  <h2>Receiver Monitor</h2>
  <div class="row">Throttle: <span id="val2">1000</span> <div class="bar"><div id="bar2" class="fill"></div></div></div>
  <div class="row">Roll: <span id="val0">1500</span> <div class="bar"><div id="bar0" class="fill"></div></div></div>
  <div class="row">Pitch: <span id="val1">1500</span> <div class="bar"><div id="bar1" class="fill"></div></div></div>
  <div class="row">Yaw: <span id="val3">1500</span> <div class="bar"><div id="bar3" class="fill"></div></div></div>
  <div class="row">AUX1: <span id="val4">1000</span> <div class="bar"><div id="bar4" class="fill"></div></div></div>
</div>
<div class="card">
  <h2>Joystick Override (Simulation)</h2>
  <label><input type="checkbox" id="rxTest" onchange="toggleRxTest()"> Enable Joystick Override</label>
  <div class="row" style="margin-top:10px;">
    Throttle: <input type="range" min="1000" max="2000" value="1000" id="joy2" oninput="setJoystick(2, this.value)">
    Roll: <input type="range" min="1000" max="2000" value="1500" id="joy0" oninput="setJoystick(0, this.value)">
    Pitch: <input type="range" min="1000" max="2000" value="1500" id="joy1" oninput="setJoystick(1, this.value)">
    Yaw: <input type="range" min="1000" max="2000" value="1500" id="joy3" oninput="setJoystick(3, this.value)">
    AUX1: <input type="range" min="1000" max="2000" value="1000" id="joy4" oninput="setJoystick(4, this.value)">
  </div>
</div>
<div class="card">
  <h2>Motor Test Mode (Disarmed Only)</h2>
  <label><input type="checkbox" id="mTest" onchange="toggleMTest()"> Enable Motor Test</label>
  <div class="row" style="margin-top:10px;">
    M1: <input type="range" min="1000" max="1150" value="1000" id="m1" oninput="setMotor(0, this.value)">
    M2: <input type="range" min="1000" max="1150" value="1000" id="m2" oninput="setMotor(1, this.value)">
    M3: <input type="range" min="1000" max="1150" value="1000" id="m3" oninput="setMotor(2, this.value)">
    M4: <input type="range" min="1000" max="1150" value="1000" id="m4" oninput="setMotor(3, this.value)">
  </div>
</div>
<div class="card">
  <h2>Flight Data Log (CSV)</h2>
  <button onclick="loadLog()">Fetch CSV Log</button>
  <button onclick="copyLog()" style="margin-left:10px;">Copy to Clipboard</button><br><br>
  <textarea id="logBox" readonly placeholder="Click Fetch to load CSV data..."></textarea>
</div>
<script>
function get(url, cb){fetch(url).then(r=>r.json()).then(cb);}
function post(url, d, cb){
  let b=Object.keys(d).map(k=>encodeURIComponent(k)+'='+encodeURIComponent(d[k])).join('&');
  fetch(url,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:b}).then(r=>r.json()).then(cb);
}
function loadPID(){ get('/api/pid', d=>{ for(let k in d){let el=document.getElementById(k); if(el)el.value=d[k];} }); }
function savePID(){
  let d={}; document.querySelectorAll('#pidForm input').forEach(i=>d[i.id]=parseFloat(i.value));
  post('/api/pid', d, r=>{ alert(r.status); });
}
function updateRX(){
  get('/api/receiver', d=>{
    for(let i=0; i<5; i++){
      let v=d.channels[i]; document.getElementById('val'+i).innerText=v;
      document.getElementById('bar'+i).style.width=((v-1000)/10)+'%';
    }
  });
}
function toggleRxTest(){
  let act = document.getElementById('rxTest').checked;
  post('/api/receiver', {active: act, channelIdx: -1, value: 1500}, r=>{});
}
function setJoystick(idx, val){
  if(!document.getElementById('rxTest').checked) return;
  post('/api/receiver', {active: true, channelIdx: idx, value: parseInt(val)}, r=>{});
}
function toggleMTest(){
  let act = document.getElementById('mTest').checked;
  post('/api/motor', {active: act, motorIdx: -1, value: 1000}, r=>{ if(!r.ok){document.getElementById('mTest').checked=false; alert(r.msg);} });
}
function setMotor(idx, val){
  if(!document.getElementById('mTest').checked) return;
  post('/api/motor', {active: true, motorIdx: idx, value: parseInt(val)}, r=>{});
}
function loadLog(){ fetch('/api/log').then(r=>r.text()).then(t=>{ document.getElementById('logBox').value=t; }); }
function copyLog(){
  let b=document.getElementById('logBox'); b.select(); document.execCommand('copy');
  alert('Copied raw CSV to clipboard!');
}
window.onload=()=>{ loadPID(); setInterval(updateRX, 250); };
</script></body></html>)rawhtml";

#endif // WEBDASHBOARDPAGE_H
