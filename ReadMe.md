# PROGRAMAS C

## Volumen

Escriba un programa que lea un archivo wav y genere otro archivo wav cuyo volumen sea la mitad del primero. El programa recibe los nombres de los archivos de entrada y salida de su linea de comando; por ejemplo:

volumen entrada.wav salida.wav 

## Convolución

Escriba un programa que simule un circuito RC en configuración de pasa bajas con frecuencia de corte de 2000Hz y una frecuencia de muestreo de 44100 muestras por segundo, mediante la convolución de la respuesta al impulso de dicho circuito con el archivo wav de entrada; generando un archivo wav con la salida; los nombres de los archivos se pasan por la línea de comando:

convolución entrada.wav salida.wav

El programa tomara 100 muestras de la respuesta impulso para realizar la convolución; el factor de escala debe ser tal que al aplicar corriente directa se obtenga (después de 100 muestras) el mismo nivel en la salida que el que se aplica en la entrada.

## Transformada Discreta de Fourier

Escriba un programa que calcule la transformada discreta de Fourier del archivo wav de entrada (monoaural); generando un archivo wav con la salida (estéreo); los nombres de los archivos se pasan por la línea de comando:

tdf entrada.wav salida.wav

Aplicar I/N al calcular la TDF como único factor de escala (no buscar un máximo y escalar con respecto a él); de esta manera la tdf de 1 (corriente directa) debe subir hasta 1.

## Transformada discreta de Fourier Inversa

Escriba un programa que calcule la transformada discreta de Fourier inversa del archivo wav de entrada (estéreo); generando un archivo wav con la salida (estéreo); los nombres de los archivos se pasan por la línea de comando:

tdfi entrada.wav salida.wav

El resultado no se escalará solo se truncarán los valores que excedan los valor máximo y mínimo para evitar que aparezcan como valores con otro signo (se está asumiendo que el factor de escala de I/N se aplicó en el programa de la TDF).

## Transformada Rápida de Fourier

Escriba un programa que calcule la transformada rápida de Fourier del archivo wav de entrada (monoaural); generando un archivo wav con la salida (estéreo); los nombres de los archivos se pasan por la línea de comando:

fft entrada.wav salida.wav

Aplicar I/N al calcular la FFT como único factor de escala (no buscar un máximo y escalar con respecto a él); de esta manera la FFT de 1 (corriente directa) debe subir hasta 1.

Si el archivo de entrada no contiene un número de datos potencia de dos se deben completar los datos a la siguiente potencia de dos rellenando con ceros.

## Transformada Rápida de Fourier Inversa

Escriba un programa que calcule la transformada rápida de Fourier inversa del archivo wav de entrada (estéreo); generando un archivo wav con la salida (estéreo); los nombres de los archivos se pasan por la línea de comando: ffti entrada.wav salida.wav No aplicar ninguna escala al calcular la FFTI (no buscar un máximo y escalar con respecto a él); de esta manera la FFTI regresaría una señal constante de amplitud 1 en la parte real cuando se introduzca un impulso de amplitud uno en la parte real de la entrada. Se asume que la entrada debe tener un numero de datos que sea potencia de dos. 

## DTMF

Escriba un programa que reciba el nombre de un archivo wav conteniendo el sonido generado al presionar una tecla de un teléfono (código DTMF) y cuya única salida sea la impresión de la tecla que lo generó. El archivo será monoaural con frecuencia de muestreo de 44100 muestras por segundo, pero la duración es variable.

## Multiplicación

Escriba un programa que multiplique dos archivos wav de entrada y genere un archivo nuevo de salida; los nombres de los archivos se pasan por la línea de comando:

 programa entrada1.wav entrada2.wav salida.wav

El programa debe ser capaz de aceptar archivos monoaurales o archivos estéreo; en el primer caso se considerara que el canal corresponde a una señal real, mientras que en el caso de las señales estéreo se asumirá que se trata de señales complejas, cuyo primer canal corresponde a la parte real y el segundo canal contiene la parte imaginaria.

⌨️ con ❤️ por Erik Garfia Acevedo 😊
