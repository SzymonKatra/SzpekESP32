# SzpekESP32

**[Szpek](https://www.szpek.pl) is a smog sensor farm project which helps to monitor air pollution in the cities.**

This project contains firmware for "Szpek" sensor device.

[User guide for sensor device (PL)](https://www.szpek.pl/manual)

## Details
A sensor device is build using:
- [ESP32-WROOM-32D system-on-chip](https://www.espressif.com/en/products/socs/esp32),
- [Plantower PMS7003](https://www.plantower.com/en/products_33/76.html) module for smog measurement,
- [Bosch BME280](https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/) sensor for humidity, temperature and pressure measurements.

PCB design was done using [KiCad](https://www.kicad.org/) software.

Sensor collects measurements and sends them to the service hosted in the cloud.  
Repositories for backend application and Web UI interface can be found here:
- [Backend service](https://github.com/wojciechN9/Szpek)
- [Web UI interface](https://github.com/wojciechN9/Szpek-UI)

## Photos

![Device photo](docs/sensor_photo.jpg)
![3D PCB view from KiCad](docs/szpek_pcb.png)

## License
