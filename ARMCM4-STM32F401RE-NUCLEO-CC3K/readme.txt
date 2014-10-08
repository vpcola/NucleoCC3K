*****************************************************************************
** ChibiOS/RT port for ARM-Cortex-M4 STM32F401.                            **
*****************************************************************************

** TARGET **

The demo runs on an ST_NUCLEO_F401RE board.

** The Demo **

The demo uses the SparkFun's CC3000 wifi sheild to connect to a web server
where the Nucleo board pushes data via a POST http request. Data is archived
at the server side using mysql.

** Build Procedure **

The demo has been tested by using the free Codesourcery GCC-based toolchain
and YAGARTO.
Just modify the TRGT line in the makefile in order to use different GCC ports.

** Notes **

Some files used by the demo are not part of ChibiOS/RT but are copyright of
ST Microelectronics and are licensed under a different license.
Also note that not all the files present in the ST library are distributed
with ChibiOS/RT, you can find the whole library on the ST web site:

                             http://www.st.com
