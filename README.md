# Attendance System base on ESP32 microcontroller

- Automatic audio playback upon receiving
- Message is sent by MQTT protocol
- Can play upto 25MB of mp3 audio

| Agenda   | Status |
| -------- | ------- |
| Playing audio with speaker  | 🚀 Works |
| Playing audio with speaker received over http  | 🚀 Works |
| Sending Data over MQTT | 🚀 Works |
| Incorporate ESP32 mutli-core for networking and playing parallely | ⏳ Won't work with mqtt |
| Receiving whole audio over MQTT and playback | 🚀 Works |
| Volume level is persistant across reboots | 🚀 Works |

| Future plans   | Importance |
| -------- | ------- |
| Receiving audio over HTTP to reduce latency | High |
| Adding authentication | High |
| Handling multiple audio and playback | Medium |
| Being able to configure over the air | High |
| Improve audio quality using the features of arduino-audio-tools | Low |
