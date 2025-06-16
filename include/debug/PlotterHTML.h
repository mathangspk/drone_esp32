#ifndef PLOTTER_HTML_H
#define PLOTTER_HTML_H

const char* PLOTTER_HTML = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Drone Debug Plotter</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
            background-color: #f5f5f5;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
        }
        .chart-container {
            background: white;
            border-radius: 8px;
            padding: 20px;
            margin: 20px 0;
            box-shadow: 0 2px 4px rgba(0,0,0,0.1);
        }
        .status {
            background: #e8f5e8;
            padding: 10px;
            border-radius: 5px;
            margin: 10px 0;
        }
        .grid {
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 20px;
        }
        @media (max-width: 768px) {
            .grid {
                grid-template-columns: 1fr;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>Drone Debug Plotter</h1>
        
        <div class="status" id="status">
            Status: Connected
        </div>

        <div class="grid">
            <div class="chart-container">
                <h3>Attitude (Roll, Pitch, Yaw)</h3>
                <canvas id="attitudeChart"></canvas>
            </div>
            
            <div class="chart-container">
                <h3>PID Output</h3>
                <canvas id="pidChart"></canvas>
            </div>
            
            <div class="chart-container">
                <h3>Motor Values</h3>
                <canvas id="motorChart"></canvas>
            </div>
            
            <div class="chart-container">
                <h3>System (Voltage, Current)</h3>
                <canvas id="systemChart"></canvas>
            </div>
        </div>
    </div>

    <script>
        // Chart configurations
        const maxDataPoints = 100;
        let updateInterval;
        
        // Attitude Chart
        const attitudeCtx = document.getElementById('attitudeChart').getContext('2d');
        const attitudeChart = new Chart(attitudeCtx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Roll',
                    data: [],
                    borderColor: 'rgb(255, 99, 132)',
                    backgroundColor: 'rgba(255, 99, 132, 0.1)',
                    tension: 0.1
                }, {
                    label: 'Pitch',
                    data: [],
                    borderColor: 'rgb(54, 162, 235)',
                    backgroundColor: 'rgba(54, 162, 235, 0.1)',
                    tension: 0.1
                }, {
                    label: 'Yaw',
                    data: [],
                    borderColor: 'rgb(75, 192, 192)',
                    backgroundColor: 'rgba(75, 192, 192, 0.1)',
                    tension: 0.1
                }]
            },
            options: {
                responsive: true,
                scales: {
                    y: {
                        beginAtZero: false
                    }
                },
                animation: {
                    duration: 0
                }
            }
        });

        // PID Chart
        const pidCtx = document.getElementById('pidChart').getContext('2d');
        const pidChart = new Chart(pidCtx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Roll PID',
                    data: [],
                    borderColor: 'rgb(255, 99, 132)',
                    backgroundColor: 'rgba(255, 99, 132, 0.1)',
                    tension: 0.1
                }, {
                    label: 'Pitch PID',
                    data: [],
                    borderColor: 'rgb(54, 162, 235)',
                    backgroundColor: 'rgba(54, 162, 235, 0.1)',
                    tension: 0.1
                }, {
                    label: 'Yaw PID',
                    data: [],
                    borderColor: 'rgb(75, 192, 192)',
                    backgroundColor: 'rgba(75, 192, 192, 0.1)',
                    tension: 0.1
                }]
            },
            options: {
                responsive: true,
                scales: {
                    y: {
                        beginAtZero: false
                    }
                },
                animation: {
                    duration: 0
                }
            }
        });

        // Motor Chart
        const motorCtx = document.getElementById('motorChart').getContext('2d');
        const motorChart = new Chart(motorCtx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Motor 1',
                    data: [],
                    borderColor: 'rgb(255, 99, 132)',
                    backgroundColor: 'rgba(255, 99, 132, 0.1)',
                    tension: 0.1
                }, {
                    label: 'Motor 2',
                    data: [],
                    borderColor: 'rgb(54, 162, 235)',
                    backgroundColor: 'rgba(54, 162, 235, 0.1)',
                    tension: 0.1
                }, {
                    label: 'Motor 3',
                    data: [],
                    borderColor: 'rgb(75, 192, 192)',
                    backgroundColor: 'rgba(75, 192, 192, 0.1)',
                    tension: 0.1
                }, {
                    label: 'Motor 4',
                    data: [],
                    borderColor: 'rgb(255, 205, 86)',
                    backgroundColor: 'rgba(255, 205, 86, 0.1)',
                    tension: 0.1
                }]
            },
            options: {
                responsive: true,
                scales: {
                    y: {
                        beginAtZero: true,
                        max: 100
                    }
                },
                animation: {
                    duration: 0
                }
            }
        });

        // System Chart
        const systemCtx = document.getElementById('systemChart').getContext('2d');
        const systemChart = new Chart(systemCtx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Voltage (V)',
                    data: [],
                    borderColor: 'rgb(255, 99, 132)',
                    backgroundColor: 'rgba(255, 99, 132, 0.1)',
                    tension: 0.1,
                    yAxisID: 'y'
                }, {
                    label: 'Current (A)',
                    data: [],
                    borderColor: 'rgb(54, 162, 235)',
                    backgroundColor: 'rgba(54, 162, 235, 0.1)',
                    tension: 0.1,
                    yAxisID: 'y1'
                }]
            },
            options: {
                responsive: true,
                scales: {
                    y: {
                        type: 'linear',
                        display: true,
                        position: 'left',
                        beginAtZero: true
                    },
                    y1: {
                        type: 'linear',
                        display: true,
                        position: 'right',
                        beginAtZero: true,
                        grid: {
                            drawOnChartArea: false,
                        },
                    }
                },
                animation: {
                    duration: 0
                }
            }
        });

        function updateCharts(data) {
            const timestamp = new Date().toLocaleTimeString();
            
            // Update Attitude Chart
            updateChart(attitudeChart, timestamp, [
                data.attitude.roll,
                data.attitude.pitch,
                data.attitude.yaw
            ]);
            
            // Update PID Chart
            updateChart(pidChart, timestamp, [
                data.pid.rollPID,
                data.pid.pitchPID,
                data.pid.yawPID
            ]);
            
            // Update Motor Chart
            updateChart(motorChart, timestamp, [
                data.motors.motor1,
                data.motors.motor2,
                data.motors.motor3,
                data.motors.motor4
            ]);
            
            // Update System Chart
            updateChart(systemChart, timestamp, [
                data.system.voltage,
                data.system.current
            ]);
        }

        function updateChart(chart, label, values) {
            chart.data.labels.push(label);
            chart.data.datasets.forEach((dataset, index) => {
                dataset.data.push(values[index]);
                if (dataset.data.length > maxDataPoints) {
                    dataset.data.shift();
                }
            });
            
            if (chart.data.labels.length > maxDataPoints) {
                chart.data.labels.shift();
            }
            
            chart.update('none');
        }

        function fetchData() {
            fetch('/debug')
                .then(response => response.json())
                .then(data => {
                    updateCharts(data);
                })
                .catch(error => {
                    console.error('Error fetching data:', error);
                    document.getElementById('status').innerHTML = 'Status: Error - ' + error.message;
                    document.getElementById('status').style.background = '#ffe8e8';
                });
        }

        // Start polling every 100ms (10Hz)
        updateInterval = setInterval(fetchData, 100);
        
        // Initial fetch
        fetchData();
    </script>
</body>
</html>
)rawliteral";

#endif // PLOTTER_HTML_H 