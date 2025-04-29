
ninja -C build

echo "Running debug configuration"
echo "Spawning client 1"
alacritty --hold --working-directory . --command build/executable-debug &
echo "Spawning client 2"
alacritty --hold --working-directory . --command build/executable-debug &

echo "Server"
./build/executable-debug s
