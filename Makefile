DIR=build
$(shell mkdir -p $(DIR))
$(shell gcc Program.c -o build/TextGTK `pkg-config --cflags --libs gtk+-2.0`)
$(shell echo "run.sh" > build/run.sh)
$(shell echo "./build/TextGTK" > build/run.sh)
