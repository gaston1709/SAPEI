# Firmware de SAPEI
Este firmware esta programado para correr en un arduino NANO.
## Compilacion 
Para compilar el proyecto se puede usar el Arduino IDE  o con el comando make.
Para utilizar make debes tener instalado [Arduino-cli](https://github.com/arduino/arduino-cli).
### Dependencias
Debes terner instaladas las siguientes librerias de Arduino
* MFRC522

para instalar las dependencias con make debes correr
```
make dependencies
```
## Subir al arduino
```
make upload PORT=<puerto>
```
