
ninja -C build

if [[ $? -eq 0 ]] then

   echo "Running debug configuration"
   echo "Spawning client 1"
   alacritty --hold --working-directory . --command bash -c "./build/executable-debug; exec bash" &
   echo "Spawning client 2"
   alacritty --hold --working-directory . --command bash -c "./build/executable-debug; exec bash" &
   
   echo "Server"
   ./build/executable-debug s

else
   echo "Compilaton failed"
fi

