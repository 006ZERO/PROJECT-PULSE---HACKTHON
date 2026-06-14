
#!/bin/bash
cd "$(dirname "$0")"

pkill -f "./backend"        2>/dev/null
pkill -f "simulator_stress" 2>/dev/null
pkill -f "node server.js"   2>/dev/null
pkill -f "processor.py"     2>/dev/null
sleep 1

./backend &
sleep 1
./simulator_stress &
sleep 1
node server.js &
sleep 2
python3 processor.py &

echo ""
echo "=============================="
echo "  Sport-Tech is running!"
echo "  Open: http://localhost:3000"
echo "=============================="

wait

# replace simulator with ./rpi_sensor & if were working with real sensors
