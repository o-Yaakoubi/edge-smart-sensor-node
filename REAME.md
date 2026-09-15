Edge Smart Sensor Node

An ESP32 node that monitors machine vibration, classifies the machine state on the device, and publishes only alerts over MQTT.

Overview

In a factory, every rotating machine vibrates. When the vibration pattern changes, a fault is developing. Detecting that change early is the goal of predictive maintenance.

The naive approach is to stream every raw sensor reading to a central server. This fails at industrial scale: the bandwidth is enormous, the battery life of wireless sensors collapses, and the latency is too high for a timely alert.

This project takes a different approach. An ESP32 reads a vibration signal, computes two features (RMS and peak) on the device itself, classifies the machine state into three severity levels, and publishes only the alerts. The raw signal never leaves the device. The transmitted data is reduced by more than 99 percent compared to raw streaming.

The project follows the edge computing pattern used in modern Industry 4.0 systems: the decision is made at the machine, and only meaningful information is transmitted.

Problem

Three concrete problems make raw streaming impractical.

Bandwidth. A vibration sensor samples at 10 to 25 kHz. A factory with 200 machines produces more than 100 million data points per minute. No network can absorb this rate economically.

Battery life. Wireless sensors are battery-powered. Continuous transmission drains the battery in weeks. To last for years, the sensor must transmit as little as possible.

Latency. When a bearing starts to fail, the alert matters in seconds. The decision has to be made close to the machine.

Solution

The ESP32 performs the following steps every two seconds.

1. It samples the vibration signal every 100 milliseconds into a circular buffer of 50 samples.
2. It computes the RMS and the peak of the buffer.
3. It classifies the machine state into NORMAL, WARNING or CRITICAL based on the RMS value.
4. It publishes a small JSON message over MQTT.

The signal never leaves the device unprocessed. Only the alert is transmitted.

Severity classification

The firmware classifies the machine state with three thresholds.

NORMAL   - RMS below 50
WARNING  - RMS between 50 and 100
CRITICAL - RMS above 100

In a real deployment, these thresholds are learned from the machine itself. The vibration signal is recorded during healthy operation, then again before a known failure. The thresholds are placed between the two states.

Architecture

Four layers carry the signal from the sensor to the screen.

Edge node (ESP32). Samples the sensor, computes RMS and peak, classifies severity.

MQTT broker. Receives the alerts and forwards them to any subscriber.

Flow engine (Node-RED). Subscribes to the alerts, stores the latest one, and broadcasts it to every browser through a WebSocket.

Dashboard (HTML, CSS, JavaScript). Displays the status panel, the metric values, the trend chart, and the alert log.

Repository contents

wokwi/sketch.ino           ESP32 firmware
wokwi/diagram.json         Wokwi circuit definition
node-red/flow.json         Node-RED flow with the custom HTML dashboard
report/report.pdf          Full project report
figures/                   Screenshots of the simulation and the dashboard

How to run

1. Open the Wokwi project at wokwi.com. Paste wokwi/sketch.ino into the code editor and wokwi/diagram.json into the diagram editor.

2. Click Run. The serial monitor shows WiFi connected, MQTT connected, and a Published message every two seconds.

3. Start Node-RED on your computer with the command node-red.

4. Open http://localhost:1880 in your browser. Import node-red/flow.json.

5. Open http://localhost:1880/edge. The dashboard loads and starts receiving alerts.

6. Turn the potentiometer knob in Wokwi. The dashboard status panel changes colour, the RMS and peak values update, the trend chart grows, and the alert log fills.

Results

The system was validated end to end.

The ESP32 published alerts every two seconds on the agreed MQTT topic.
The dashboard received each alert and updated in real time.
The status panel changed colour between green, orange, and red according to the severity.
The audible alert triggered on the transition to Critical.

Measured data reduction over one hour of monitoring.

Raw streaming at 10 kHz:   36,000,000 messages, 72,000,000 bytes
Edge node:                 1,800 messages, 180,000 bytes
Reduction:                 99.995 percent by message count

Limitations

The vibration signal is simulated with a potentiometer, not a real accelerometer.
The thresholds are fixed, not derived from historical machine data.
The broker is public. In production, a private broker with authentication would be used.
The system was validated on a single simulated machine, not a fleet.

What I learned

The most important decision in this project was deciding what to compute at the edge and what to leave for a later stage. RMS and peak are cheap to compute and highly informative, so they belong on the device. Anything more complex belongs on a gateway. Making that separation correctly is the essence of edge computing.

The data reduction is not a side effect. It is the point. A system that transmits less can run on a smaller battery, on a slower network, and on a cheaper cloud plan.

Tools

ESP32 DevKit C V4
Wokwi simulator
MQTT over Wi-Fi
Node-RED
HTML, CSS, JavaScript

Author

Oumaima Yaakoubi
Instrumentation and Intelligent Systems, INSAT, Tunisia
LinkedIn: linkedin.com/in/oumaima-yaakoubi
GitHub: github.com/o-Yaakoubi

License

MIT License. See the LICENSE file for details.