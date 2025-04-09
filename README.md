# CG2111a 2425 Sem 2 Grp 1A Repo
clone this on a Linux system (others might not work)

## Start Script
Start script runs a tmux panel that runs teleop, slam node, camera stream server

build start script 
```bash
chmod +x start_script.sh
```
run the start script
```bash
./start_script.sh
```

Alternatively, run the below commands:

## Commands
1. Open Alex/alex.ino in Arduino IDE, flash to Arduino mega328p

2. SSH into the RPi, 
```bash
cd pi
g++ alex-pi.cpp serial.cpp serialize.cpp -pthread -out alex-main
./alex-main
```

3. Start the camera server, do step a first, then
```bash
cd slam_rpicam_env
python AlexCameraStreamServer.py
```
Access the camera feed via http://localhost:8080

4. Start the SLAM node, do step a first, then
```bash
cd slam_rpicam_env
python labs/SlamLabs/alex-main.py
```
Access SLAM visualisation via http://localhost:8181



## Sourcing the SLAM/camera server env
To build the env:
```bash
sudo chmod +x setup_environment.sh
sudo ./setup_environment.sh
```

a. In a new terminal, source the env (used for SLAM and camera server)
```bash
cd slam_rpicam_env
source env/bin/activate
```

