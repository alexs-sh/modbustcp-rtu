## About

[![Total alerts](https://img.shields.io/lgtm/alerts/g/alexs-sh/modbustcp-rtu.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/alexs-sh/modbustcp-rtu/alerts/)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/alexs-sh/modbustcp-rtu.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/alexs-sh/modbustcp-rtu/context:cpp)
[![Build Status](https://gitlab.com/alexssh/modbustcp-rtu/badges/master/pipeline.svg)](https://gitlab.com/alexssh/modbustcp-rtu/-/commits/master)
[![Latest Release](https://gitlab.com/alexssh/modbustcp-rtu/-/badges/release.svg)](https://gitlab.com/alexssh/modbustcp-rtu/-/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)


The ModbusTCP/RTU converter. It also supports UDP transport, queueing, and
statistic collecting.

### Run

```
mb_driver_ext -b -D/dev/ttyM1 -P512 -s19200 -peven -t750 -w0 -u0 –i0 –o0 –c8512
```

### Build

```
cmake .
make
```

You may use the BIG_ENDIAN flag to specify the byte order of a target device.

```
cmake -DBIG_ENDIAN=1 .
make
```

### App parameters

- b – run in background (off by default)
- v – run in interactive mode (off by default)
- U – port number for accepting UDP requests (off by default)
- I – interface IP for binding server port(s). (0.0.0.0 by default)
- P – port number accepting TCP connections (502 by default).
- D – serial port name (/dev/null by default)
- s – serial port speed (38400 by default)
- p – serial port parity ODD, EVEN, NONE (ODD by default)
- l – serial port byte length (8 bits by default)
- e – serial port number of stop bits (1 by default)
- t – serial port answer timeout, ms (50 by default)
- d – timeout for disconnecting interactive clients, ms (30000 by default)
- w – time to live of message in a queue, ms (1000 by default). 
    - 0 - disable TTL control 
- u – error code that will be reported in case of no answer from a serial device (11 by default). 
    - 0 - disable sending error code 
- o – error code that will be reported in case of request queue overflow (6 by default). 
    - 0 - disable sending error code
- i – error code that will be reported in case of receiving an invalid message. For example, wrong CRC, field, and so on. (3 by default).
    - 0 - disable sending error code
- c -  TCP port for connecting and getting information/statistics/traces.(disabled by default). The service connection supports several commands:
    - i – show short info and statistic 
    - t – enable/disable message tracing 
    - с – close service connection
    - h - show help
- h – show help


## Original README
## Конвертер ModbusTCP/UDP в ModbusRTU

[![Total alerts](https://img.shields.io/lgtm/alerts/g/alexs-sh/modbustcp-rtu.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/alexs-sh/modbustcp-rtu/alerts/)
[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/alexs-sh/modbustcp-rtu.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/alexs-sh/modbustcp-rtu/context:cpp)
[![Build Status](https://gitlab.com/alexssh/modbustcp-rtu/badges/master/pipeline.svg)](https://gitlab.com/alexssh/modbustcp-rtu/-/commits/master)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

### Запуск

```
mb_driver_ext -b -D/dev/ttyM1 -P512 -s19200 -peven -t750 -w0 -u0 –i0 –o0 –c8512
```

### Сборка

```
cmake .
make
```

### Ключи для запуска

- b  – запуск в фоновом режиме (по умолчанию отключен)
- v – запуск с поддержкой текстовых команд (по умолчанию отключен.)
- U – UDP порт для получения запросов Modbus (по умолчанию не используется)
- I – IP – адрес интерфейса, к которому привязан преобразователь (по умолчанию “0.0.0.0”)
- P – TCP порт для подключения Modbus мастеров (по умолчанию 502).
- D – имя последовательного порта, с которым работает преобразователь (по умолчанию “/dev/null”)
- s – скорость обмена по последовательному порту в бод/с (по умолчанию 38400)
- p – четность, ODD, EVEN, NONE (по умолчанию ODD)
- l – длина байта в сообщения, бит (по умолчанию 8)
- e – количество стоп – бит (по умолчанию 1)
- t – таймаут на ожидание ответа от устройства, мс (по умолчанию 50)
- d – таймаут на отключение неактивных клиентов, мс (по умолчанию 30000)
- w – время жизни сообщения в очереди, мс (по умолчанию 1000). Значение 0 приведет к отключению контроля времени жизни
- u – код ошибки, которая будет выдаваться клиенту при отсутствии ответа от устройства (по умолчанию 11). Значение 0 приведет к подавлению отправки сообщения с кодом ошибки.
- o – код ошибки, которая будет выдаваться клиенту при переполнении очереди сообщений (по умолчанию 6). Значение 0 приведет к подавлению отправки сообщения с кодом ошибки.
- i – код ошибки, которая будет выдаваться клиенту при получении битого сообщения (по умолчанию 3). Значение 0 приведет к подавлению отправки сообщения с кодом ошибки.
- c -  TCP порт для подключения клиента для сбора статистики и вывода трассировки сообщений (по умолчанию отключен). После подключения к порту в программу можно передать сл. команды управления:
    - i – вывести информацию о работе
    - t – включить / отключить трассировку сообщений
    - с – закрыть подключение
    - h - справка
- h –вывод справки


