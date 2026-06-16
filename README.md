# PROJECT PULSE: Real-Time Athlete Fatigue Detection System

### 🏆 1st Place Winner - Sport-Tech Hackathon 2026

PULSE is an advanced, low-latency wearable telemetry system designed to monitor athletic workloads and predict fatigue-induced injuries before they happen. By tracking high-frequency biomarkers directly at the edge, the system processes multi-dimensional biometric inputs through a machine learning classifier and streams instantaneous safety metrics to a centralized coaching dashboard.

---

## Core Engineering Features

* **Isolated Dual I2C Bus Pipeline:** Eliminates hardware bus contention by physically decoupling the `HW-579` accelerometer (I2C Bus 1) from the `MAX30100` optical sensor (I2C Bus 0) on the Raspberry Pi.
* **Ultra-Low Latency UDP Transport:** Drops heavy connection protocols in favor of a raw UDP socket configuration, streaming tightly packed binary structs with a virtual **~0ms network overhead**.
* **Modern C++23 Edge Node:** Native data sampling and conditioning code compiled with strict `-O3` performance optimizations for stable high-frequency execution.
* **Edge Machine Learning:** Features a dynamic Python backend driving a trained Random Forest classification model to translate multi-variate telemetry into accurate fatigue scaling metrics.

---

##  System Architecture
[ HW-579 Accel ] ----> ( I2C Bus 1 ) ---

|---> [ Raspberry Pi Node ] ---> ( UDP Struct Payload ) ---> [ AI Inference Engine ] ---> [ WebSockets ] ---> [ Live Node.js Grid Dashboard ]
[ MAX30100 Pulse ] ---> ( I2C Bus 0 ) ---/       (Native C++23)               (~0ms Latency)            (Random Forest ML)


1. **Hardware Layer (Edge):** Low-level sensor interaction reads real-time kinetic acceleration vector magnitudes alongside infrared blood-volume variations.
2. **Processing Engine:** A C++ hybrid scaling engine monitors input magnitudes to filter out physical motion artifacts dynamically.
3. **Inference Matrix:** The raw metrics are evaluated across 9 biomarker trends against a model trained on 10K feature samples, achieving a **91% diagnostic accuracy**.
4. **Presentation Layer:** A Node.js streaming architecture pushes the state metrics up to a real-time reactive grid dashboard for sports science personnel.

---

## 📁 Repository Structure

```text
├── rpi_sensor.cpp      # Native C++23 high-frequency sensor engine and UDP socket client.
├── sensor_packet.hpp   # Fixed binary struct layout defining network data contracts.
├── processor.py        # Python data consumer driving the local Random Forest AI inference.
├── server.js           # Node.js backend managing local dashboard sockets and data pipelines.
└── start.sh            # Production automation script for clean compilation and orchestration.




🚀 Quick Start & Deployment
Prerequisites

    Raspberry Pi with I2C interfaces fully enabled (i2c-0 and i2c-1).

    Modern C++ compiler supporting standard c++23 flags.

    Node.js runtime environment installed.


Execution

Simply run the master orchestration shell script to clean-compile the native code and boot the processing stacks concurrently:


in the Bash
chmod +x start.sh
./start.sh
