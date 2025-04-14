#!/bin/bash

SESSION="cg2111a_moment"

# Start a new detached tmux session
tmux new-session -d -s $SESSION

# Split the bottom pane vertically (Pane 1 will be created below)
tmux send-keys -t $SESSION 'cd ~/cg2111_2425/pi' C-m
tmux send-keys -t $SESSION './alex-pi' C-m

# Pane 0 (top-left): Ru
tmux split-window -h -t $SESSION 
tmux send-keys -t $SESSION 'cd ~/cg2111_2425/slam_rpicam_env' C-m
tmux send-keys -t $SESSION 'source env/bin/activate' C-m
tmux send-keys -t $SESSION 'cd ~/cg2111_2425/colour_sensor; clear' C-m
tmux send-keys -t $SESSION 'python tcs3200_detector.py' C-m

# Pane 0 run AlexCameraStreamServer.py
tmux split-window -v -t $SESSION
tmux send-keys -t $SESSION 'cd ~/cg2111_2425/slam_rpicam_env' C-m
tmux send-keys -t $SESSION 'source env/bin/activate; clear' C-m
tmux send-keys -t $SESSION 'python AlexCameraStreamServer.py' C-m

# Split the top pane horizontally (Pane 2 will be created)
tmux split-window -v -t $SESSION
tmux send-keys -t $SESSION 'cd ~/cg2111_2425/slam_rpicam_env' C-m
tmux send-keys -t $SESSION 'source env/bin/activate; clear' C-m
tmux send-keys -t $SESSION 'python labs/SlamLab/alex_main.py' C-m

tmux split-window -v -t $SESSION
tmux send-keys -t $SESSION 'export PS1=""; clear; echo "Camera Server"; echo "http://$(hostname -I | awk '\''{print $1}'\''):8000"; echo "SLAM Bokeh Server"; echo "http://$(hostname -I | awk '\''{print $1}'\''):8181"' C-m



#Select Pane 0 (top-left) before attaching
tmux select-pane -t $SESSION:0.0

# Attach to the session
tmux attach -t $SESSION
