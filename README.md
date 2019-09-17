# IRS2
School work for IRS2

Author: Luka Gavric 2018/3194

Fully commented and working code version.

Safe unlocking device is controlled through UART serial port.
When connecting NUCLEO devkit to PC, serial port is available.

After reset, start message is displayed.
Program is started by sending 0xA5 byte via serial port.

Before starting the program, user can define new lock combination.
Lock combination is initialized after sending 0xB9 byte,
when system expects three numbers not greater than 15.
This process is finished by sending 0xF1 stop byte.

Only START and COMBINATION_SET command messages are accepted.

After starting application, rotary encoder is used as a safe model.
Angle of encoder defines input number displayed on PC application.
Pressing the button sets this number.

Successfull unlock is followed by LED flashing, while exceeding the
maximum number of fail attempts is marked by LED circle walk.
