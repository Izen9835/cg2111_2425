#!/bin/bash

SESSION="cg2111a_moment"

# Start new detached session
tmux new-session -d -s $SESSION

# Pane 0
tmux send-keys -t $SESSION 'cd ~/cg2111_2425/slam_rpicam_env' C-m
tmux send-keys -t $SESSION 'source env/bin/activate' C-m
tmux send-keys -t $SESSION 'python AlexCameraStreamServer.py' C-m

# Split pane horizontally (creates pane 1)
tmux split-window -h -t $SESSION
tmux send-keys -t $SESSION:0.1 'cd ~/cg2111_2425/slam_rpicam_env' C-m
tmux send-keys -t $SESSION:0.1 'source env/bin/activate' C-m
tmux send-keys -t $SESSION:0.1 'python labs/SlamLab/alex_main.py' C-m


# Attach if you want
tmux attach -t $SESSION
