#ifndef DASHBOARD_HTML_H
#define DASHBOARD_HTML_H

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html lang="en">
<head>
  <meta charset="UTF-8">
  <title>Bioreacteur ESP32-S3 Dual-Brain</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px; background-color: #f4f7f6; }
    .card { background-color: white; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); padding: 20px; width: 300px; display: inline-block; margin: 10px; border-radius: 10px; vertical-align: top;}
    .chart-container { width: 80%; margin: auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5); margin-top: 20px;}
    h2 { color: #003366; }
    .value { font-size: 2.5rem; font-weight: bold; color: #2c3e50; }
    .unit { font-size: 1.2rem; color: #7f8c8d; }
    button { padding: 12px 24px; font-size: 1rem; cursor: pointer; border-radius: 5px; border: none; background-color: #3498db; color: white; margin: 5px; transition: background 0.3s; }
    button:hover { background-color: #2980b9; }
    .btn-download { background-color: #27ae60; }
    .status-error { color: #e74c3c; font-weight: bold; padding: 10px; background: #ffebeb; border: 1px solid #e74c3c; margin: 10px auto; width: 80%; border-radius: 5px; }
    .status-offline { color: #7f8c8d; font-weight: bold; padding: 10px; background: #eee; border: 1px solid #7f8c8d; margin: 10px auto; width: 80%; border-radius: 5px; }
    input { padding: 8px; width: 60px; margin: 5px; }
    .pump-active { color: #27ae60; font-weight: bold; }
    .pump-inactive { color: #7f8c8d; }
  </style>
</head>
<body>
  <h1>Bioreacteur ESP32-S3 Dual-Brain</h1>
  <div id="error-msg" class="status-error" style="display:none;">CONTROL NODE SENSOR ERROR - FAILSAFE ACTIVE</div>
  <div id="link-msg" class="status-offline" style="display:none;">CONTROL NODE OFFLINE - CHECK UART LINK</div>

  <div class="card">
    <h2>pH Level</h2>
    <p><span class="value" id="ph">--</span> <span class="unit">pH</span></p>
    <p><small>Voltage: <span id="ph-v">--</span>V</small></p>
  </div>

  <div class="card">
    <h2>OD / Temp</h2>
    <p><span class="value" id="od">--</span> <span class="unit">AU</span></p>
    <p><span class="value" id="temp">--</span> <span class="unit">°C</span></p>
  </div>

  <div class="card">
    <h2>Growth / UV</h2>
    <p>µ: <span class="value" id="mu">--</span> <span class="unit">h⁻¹</span></p>
    <p>UV: <span class="value" id="uv-v">--</span> <span class="unit">V</span></p>
  </div>

  <div class="card">
    <h2>Actuators</h2>
    <p>Acid: <span id="p_a" class="pump-inactive">OFF</span></p>
    <p>Base: <span id="p_b" class="pump-inactive">OFF</span></p>
    <p>Nutrient: <span id="p_n" class="pump-inactive">OFF</span></p>
    <p>Heater: <span id="heat" class="pump-inactive">OFF</span></p>
  </div>

  <div class="card">
    <h2>Configuration</h2>
    <label>pH Target:</label> <input type="number" id="target-ph" step="0.1"><br>
    <label>Temp Target:</label> <input type="number" id="target-temp" step="0.5"><br>
    <button onclick="updateSettings()">Apply Setpoints</button>
    <hr>
    <button onclick="togglePump('nutrient')">Manual Feed</button>
    <button onclick="togglePump('acid')">Acid</button>
    <button onclick="togglePump('base')">Base</button>
  </div>

  <div class="chart-container">
    <canvas id="bioChart"></canvas>
  </div>

  <script>
    var ctx = document.getElementById('bioChart').getContext('2d');
    var chart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: [],
            datasets: [{
                label: 'pH', borderColor: '#3498db', data: [], yAxisID: 'y'
            }, {
                label: 'OD', borderColor: '#e67e22', data: [], yAxisID: 'y1'
            }]
        },
        options: {
            scales: {
                y: { type: 'linear', position: 'left' },
                y1: { type: 'linear', position: 'right', grid: { drawOnChartArea: false } }
            }
        }
    });

    function updateSettings() {
      var ph = parseFloat(document.getElementById('target-ph').value);
      var temp = parseFloat(document.getElementById('target-temp').value);
      fetch('/set', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({phTarget: ph, tempTarget: temp})
      }).then(r => { if(r.ok) alert("Command sent to Control Node"); });
    }

    setInterval(function ( ) {
      fetch('/data').then(r => r.json()).then(data => {
        document.getElementById("ph").innerHTML = data.ph.toFixed(2);
        document.getElementById("ph-v").innerHTML = data.ph_v.toFixed(4);
        document.getElementById("od").innerHTML = data.od.toFixed(3);
        document.getElementById("temp").innerHTML = data.temp.toFixed(1);
        document.getElementById("mu").innerHTML = data.mu.toFixed(2);
        document.getElementById("uv-v").innerHTML = data.uv.toFixed(4);

        const setPump = (id, active) => {
            const el = document.getElementById(id);
            el.innerHTML = active ? "ON" : "OFF";
            el.className = active ? "pump-active" : "pump-inactive";
        };
        setPump("p_a", data.p_a);
        setPump("p_b", data.p_b);
        setPump("p_n", data.p_n);
        setPump("heat", data.heat);

        // Update Chart
        var now = new Date().toLocaleTimeString();
        chart.data.labels.push(now);
        chart.data.datasets[0].data.push(data.ph);
        chart.data.datasets[1].data.push(data.od);
        if(chart.data.labels.length > 20) {
          chart.data.labels.shift();
          chart.data.datasets[0].data.shift();
          chart.data.datasets[1].data.shift();
        }
        chart.update();

        document.getElementById("error-msg").style.display = data.err ? "block" : "none";
        document.getElementById("link-msg").style.display = data.lost ? "block" : "none";
      });
    }, 2000);

    function togglePump(pump) { fetch("/pump", {method: 'POST', headers: {'Content-Type': 'application/x-www-form-urlencoded'}, body: 'type=' + pump}); }
  </script>
</body>
</html>
)rawliteral";

#endif
