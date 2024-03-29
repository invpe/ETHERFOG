# Run

Simply download the sketch file and open it with your Arduino IDE to compile and upload to your ESP32 board.

# Modify

If required, feel free to modify the sources to fit your needs

# Data

![image](https://github.com/invpe/ETHERFOG/assets/106522950/aaff89ac-a343-4ee4-bc72-f51879f1ee4e)


EtherFog is only capturing count of wireless frames and averages the number when running,
it can also report this number to [GridShell](https://github.com/invpe/GridShell) for visualization and data analysis purposes.

However for simplicity use the Arduino Plotting functionality within the `tools` menu.

# GridShell 

You can enable this feature by enabling `#define GRIDSHELL` flag and providing your user id in the code.
The data will be sent as an average value of 10 minutes run.

Review the submitted data by opening [explorer](https://explorer.gridshelll.net:3000/) endpoint.
Details are available [here](https://github.com/invpe/GridShell/blob/main/Documentation/Tutorials/Explorer.md)


