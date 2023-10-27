# Network-Protocols-POP3
# Trabajo Práctico Especial - Protocolos de Comunicación
Implementación de un servidor POP3.

Alumnos:
- [Occhipinti, Abril		61159]
- [Ranucci, Santino Augusto	62092]
- [Rossi, Victoria			61131]
- [Zakalik, Agustín	        62068]


### Compilación
En la raíz del proyecto ejecutar:

```
make clean
make all
```

Esto genera el binario `project` en la carpeta `bin` y `client` en la carpeta `Client-Interface`. El primero es el servidor POP3 y el segundo es el cliente de monitoreo.


### Ejecución
Para correr el servidor, en la raíz del proyecto correr el comando:
```./bin/project <puerto_pasivo> -u <usuario1>:<contraseña1> .... -u <usuarioN>:<contraseñaN>```

Reemplazar ```<puerto_pasivo>``` por el puerto en el que el servidor tiene que atender nuevas conexiones.
A considerar: el ```<usuario>``` debe coincidir con un nombre de directorio en la carpeta mails. De lo contrario, el usuario podrá loggearse al servidor pero no tendrá mails.

Si se desea correr el servidor con más de un usuario se puede agregar con otro -u. Por ejemplo:

```
./bin/project <puerto_pasivo> -u <usuario1>:<contraseña1> -u <usuario2>:<contraseña2> ... -u <usuarioN>:<contraseñaN>
```


Para correr el cliente de monitoreo, en la carpeta `Client-Interface` correr el siguiente comando:
```./client <IPv4|IPv6> <Server IP> <Server Port/Service> [Optional: -WV|-BA|-MA]```

Donde  `<Server Port/Service>` es `6000`.

Opcionalmente, se puede agregar uno de los siguientes flags, para que se puedan ver las respuestas de errores del servidor :
- `WV` (Wrong Version) setea a todos los datagramas salientes con una version V2.0, que no es la del servidor, por lo tanto el servidor responderá siempre con un 101 Version Mismatch

- `BA` (Bad Authorization) setea todos los datagramas salientes con una autorización no correcta, el servidor siempre responderá un 100 (Unauthorized)

- `MA`  (Malformed Authorization) setea todos los datagramas salientes con una autorización mal formada (es decir no una cadena de 32 bytes), el servidor siempre responderá con 103 (Bad authorization Formation)

### Materiales
En la carpeta `docs` se encuentra el informe de este trabajo.

En la carpeta `test` se encuentran los archivos que se utilizaron para probar el servidor.

