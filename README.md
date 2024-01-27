![image](https://github.com/invpe/ETHERFOG/assets/106522950/6efdde3d-2a52-4932-a277-3117ba8989a3)

# ETHERFOG
WiFi Pollution Sensor

![image](https://github.com/invpe/ETHERFOG/assets/106522950/a111f4a4-e36d-4327-8d8a-76a9c8a21ae4)

EtherFog is an open-source device designed to monitor and visualize the density of WiFi traffic in a given area, referred to as "WiFi pollution." Utilizing the ESP32's promiscuous mode, EtherFog passively scans WiFi channels and tallies the number of packets, giving users a real-time indication of the WiFi activity surrounding them.

# Features:

- Channel Scanning: Hops across all WiFi channels, collecting data on WiFi traffic.
- Real-Time Data: Displays the total count of WiFi packets detected as a live graph.
- Traffic Averaging: Implements a moving average algorithm to smooth short-term fluctuations and highlight overall WiFi activity trends.
- Visual Feedback: Includes an RGB LED that changes color based on the level of WiFi traffic, offering an at-a-glance pollution indicator.

# How It Works:

EtherFog operates by continuously hopping between WiFi channels (1 to 14), staying on each channel for a predefined interval. Every packet detected is counted, and this data is processed to compute a moving average, providing a stable measure of WiFi activity over time. The result is then visually represented through an RGB LED—green for low activity, transitioning to red for high activity levels, indicating the presence of dense WiFi traffic or "pollution."


# Applications:

EtherFog is ideal for those who are sensitive to WiFi exposure or for anyone interested in the digital infrastructure of their environment. It can be used in homes, workplaces, or public spaces where understanding WiFi signal density is essential.

# Getting Started:

To get started with EtherFog, clone the repository, follow the setup instructions, and flash your ESP32 with the provided code. Detailed documentation is available to guide you through the setup process and help you interpret the results.
