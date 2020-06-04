# RCLegoPic
Adaptador de receptor de radiocontrol para Lego NXT con 16F88

![LegoRC](/LegoRC.jpg)

Para poder usar un radio control con un lego hace falta procesar las señales de salida del receptor (analógicas) y convertirlas a un formato que pueda leer el ladrillo del Lego NXT. Los sensores del NXT funcionan bajo I2C, así que se necesita como mínimo un temporizador y un módulo de comunicaciones en I2C. Un microcontrolador como el 16F88 incluye tanto los convertidores como el módulo de comunicaciones. Además, el 16F88 no necesita un cristal ni componentes externos para funcionar, se puede alimentar con el propio bus de lego y es fácil de programar. El lenguaje de programación es C, bajo el entorno Hi Tech C de Microchip.