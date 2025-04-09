#!/bin/bash

SESSION="cg2111a_moment"

# Start a new detached tmux session
tmux new-session -d -s $SESSION

# Split the bottom pane vertically (Pane 1 will be created below)
tmux send-keys -t $SESSION 'cd ~/cg2111_2425/pi' C-m
tmux send-keys -t $SESSION './alex-main' C-m

# Pane 0 (top-left): Run AlexCameraStreamServer.py
tmux split-window -h -t $SESSION 
tmux send-keys -t $SESSION 'cd ~/cg2111_2425/slam_rpicam_env' C-m
tmux send-keys -t $SESSION 'source env/bin/activate' C-m
tmux send-keys -t $SESSION 'python AlexCameraStreamServer.py' C-m

# Split the top pane horizontally (Pane 2 will be created)
tmux split-window -v -t $SESSION
tmux send-keys -t $SESSION 'cd ~/cg2111_2425/slam_rpicam_env' C-m
tmux send-keys -t $SESSION 'source env/bin/activate' C-m
tmux send-keys -t $SESSION 'python labs/SlamLab/alex_main.py' C-m

tmux split-window -v -t $SESSION
#tmux send-keys -t $SESSION 'export PS1=""; clear' C-m
#tmux send-keys -t $SESSION 'echo "Camera Server"' C-m
#tmux send-keys -t $SESSION 'echo "http://$(hostname -I | awk '{print $1}'):8080"' C-m
#tmux send-keys -t $SESSION 'echo "SLAM Bokeh Server"' C-m
#tmux send-keys -t $SESSION 'echo "http://$(hostname -I | awk '{print $1}'):8181"' C-m
tmux send-keys -t $SESSION 'export PS1=""; clear; echo "Camera Server"; echo "http://$(hostname -I | awk '\''{print $1}'\''):8080"; echo "SLAM Bokeh Server"; echo "http://$(hostname -I | awk '\''{print $1}'\''):8181"' C-m



#Select Pane 0 (top-left) before attaching
tmux select-pane -t $SESSION:0.0

# Attach to the session
tmux attach -t $SESSION
