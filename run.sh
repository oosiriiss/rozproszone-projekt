
ninja -C build

if [[ $? -eq 0 ]]; then

   tmux kill-session -t sess
   echo "Running debug configuration"

   tmux new-session -d -s sess 
   echo "spawning Server"
   tmux send-keys -t sess:0.0 "./build/executable-debug s" C-m

   echo "Spawning client 1"
   tmux split-window -h -t sess
   tmux send-keys -t sess:0.1 "./build/executable-debug" C-m

   echo "Spawning client 2"
   tmux split-window -v -t sess:0.1
   tmux send-keys -t sess:0.2 "./build/executable-debug" C-m

   # tmux select-layout -t sess tiled

   tmux attach-session -t sess
   

else
   echo "Compilaton failed"
fi

