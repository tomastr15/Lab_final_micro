# Lab_final_micro
**Integrantes:**
- Simón Martínez
- Carlos Vargas
- Martín Polo
- Tomás Trujillo

**Imagen Montaje Físico Parte 1:**

<img width="1600" height="1200" alt="image" src="https://github.com/user-attachments/assets/c3cf2d07-2dba-494a-8d71-c910bb4f59f4" />

**Imagen Montaje Físico Parte 2:**

<img width="1280" height="960" alt="image" src="https://github.com/user-attachments/assets/2201a417-835e-4cd3-a54f-be3b9b4b4d33" />
<img width="1280" height="960" alt="image" src="https://github.com/user-attachments/assets/75394697-37d1-4b52-8f06-1620b691f0f5" />

## Investigación Parte 3:

### Respuesta corta

No, no es factible usar la comunicación serial nativa del Arduino (UART TTL a 5 V) con cable AWG 24 simple sobre 20 metros, especialmente entre edificios distintos. El problema no es realmente el calibre del cable, sino tres factores combinados: el tipo de señalización (single-ended TTL), la susceptibilidad al ruido electromagnético y, sobre todo, las diferencias de potencial de tierra entre edificios. Por lo tanto, se requieren modificaciones obligatorias al montaje propuesto en la Parte II antes de implementarlo físicamente en este escenario.

### Análisis del cable AWG 24 en sí

El calibre AWG 24 no es el cuello de botella del sistema. Sus parámetros eléctricos relevantes para una tirada de 20 metros son una resistencia DC de aproximadamente 0.084 Ω/m, lo que da unos 1.7 Ω por conductor (3.4 Ω contando ida y vuelta), valor completamente despreciable para señales lógicas. La capacitancia típica en par trenzado sin blindaje ronda los 50 pF/m, lo que se traduce en cerca de 1 nF a lo largo de los 20 m, y la inductancia distribuida es del orden de 10 µH en total.

A 9600 baudios el tiempo de bit es aproximadamente 104 µs, así que el ancho de banda efectivo requerido (alrededor de 10 kHz) es muy bajo. La constante RC equivalente del cable, considerando la impedancia de salida típica del Arduino, queda en el orden de decenas de nanosegundos, varias órdenes de magnitud por debajo del tiempo de bit. En otras palabras, el cable como medio físico podría transportar perfectamente la información; lo que falla es cómo se está usando esa información a nivel eléctrico.

### Por qué no funciona tal cual

El primer problema es la **señalización TTL single-ended**. El UART del ATmega328P opera con niveles de 0 V y 5 V referidos a la tierra local de cada Arduino. Las normas prácticas para TTL plano sin acondicionamiento dan distancias máximas confiables de uno a tres metros antes de empezar a observar errores. La limitación no viene de la atenuación sino del margen de ruido: cualquier interferencia electromagnética acoplada al conductor se suma directamente a la señal, porque el receptor no tiene ningún mecanismo de rechazo de modo común.

El segundo problema es la **susceptibilidad a interferencia electromagnética**. Veinte metros de cable sin blindaje funcionan como una antena bastante eficiente. Líneas de potencia cercanas, motores eléctricos, lámparas fluorescentes, balastros, transformadores y equipos de conmutación inducen ruido en el cable que un receptor TTL puede malinterpretar como bits válidos. En un trayecto de 20 m entre edificios es casi seguro que el cable correrá en algún tramo cerca de cableado eléctrico, ductos o equipos ruidosos, por lo que este efecto no es opcional sino esperable.

El tercer problema, y de lejos el más grave, es la **diferencia de potencial de tierra entre edificios**. Dos edificios tienen tomas de tierra físicas distintas, conectadas a varillas o mallas separadas. Es perfectamente normal medir entre 1 V y varias decenas de voltios de diferencia entre ambos planos de tierra en condiciones estables, y miles de voltios durante un transitorio como el arranque de un motor grande, una falla a tierra o una descarga atmosférica cercana. Cuando se conectan las tierras de los dos Arduinos con un solo cable de cobre, se crea un lazo de tierra que produce consecuencias en cadena.

En el mejor caso, una corriente parásita circula por el conductor de retorno y desplaza el nivel de referencia del receptor, corrompiendo los datos de manera intermitente y muy difícil de diagnosticar. En el caso típico, los pines RX o TX exceden los límites absolutos de ±0.5 V respecto a Vcc o GND del receptor, lo que destruye silenciosamente esos pines o el chip entero después de unas horas o días. En el peor caso, una descarga atmosférica induce kilovoltios en el cable y destruye ambas placas instantáneamente, además de presentar un riesgo eléctrico serio para las personas que estén tocando los equipos. Esto, en instalaciones inter-edificio, es de manejo obligatorio según códigos eléctricos como el artículo 800 del NEC; en Colombia, el RETIE recoge principios equivalentes.

### Modificaciones requeridas

Una primera modificación posible es pasar a **RS-232 con un MAX232** o similar. Los niveles de ±12 V mejoran el margen de ruido respecto al TTL y el estándar contempla distancias del orden de 15 m a 9600 baudios. Sin embargo, RS-232 sigue siendo una señalización single-ended y no resuelve en absoluto el problema del lazo de tierra entre edificios, así que es útil para extender la comunicación dentro de un mismo edificio pero resulta marginal a 20 m y francamente riesgoso entre edificios distintos. No es una solución recomendable para este caso.

La solución estándar de la industria para este escenario es **RS-485 con transceptores tipo MAX485 o SN75176**. RS-485 usa señalización diferencial sobre par trenzado: el receptor observa la diferencia de voltaje entre las líneas A y B, de modo que el ruido inducido por igual en ambos conductores (modo común) se cancela. El estándar tolera un rango de modo común de −7 V a +12 V respecto a la tierra local del receptor, lo que permite cierto desbalance entre tierras aunque no las decenas de voltios que pueden aparecer entre edificios. La distancia nominal alcanza hasta 1200 metros a 100 kbps con par trenzado adecuado, por lo que 20 m es trivial. Requiere resistencias de terminación de 120 Ω en cada extremo del bus y, deseablemente, resistencias de polarización para mantener un estado definido cuando ningún transmisor está activo. La operación es half-duplex con dos hilos (A, B) o full-duplex con cuatro, y el pin de habilitación DE/RE del transceptor se controla desde un GPIO del Arduino. En este esquema el cable AWG 24 sí funciona perfectamente, siempre que sea par trenzado, y un cable Cat5 o Cat6 estándar es ideal: se aprovecha un par para A/B y otros pares para retorno o incluso alimentación auxiliar.

La opción más robusta, y la recomendada para una instalación real entre edificios, es **RS-485 con aislamiento galvánico**. Consiste en lo mismo que la opción anterior pero añadiendo aislamiento óptico o magnético entre el transceptor RS-485 y el lado del Arduino, lo que rompe completamente el lazo de tierra y protege la electrónica frente a diferencias de potencial grandes y transitorios rápidos. Se puede implementar con transceptores RS-485 aislados integrados como el ADM2483, el ISO1500 o el ADM2587E (este último particularmente cómodo porque incluye un convertidor DC/DC aislado en el mismo chip y no requiere fuente flotante externa). Como alternativa de menor costo se puede usar un MAX485 común alimentado por una fuente aislada, con la interfaz hacia el Arduino implementada con optoacopladores rápidos como el 6N137 en las líneas RX, TX y DE/RE. Es recomendable añadir además diodos TVS bidireccionales (tipo SM712 u otros específicos para RS-485) entre las líneas A, B y la tierra local en ambos extremos, para absorber transitorios rápidos.

Otra opción a considerar es la **fibra óptica**, que es la única solución verdaderamente inmune tanto a EMI como a descargas atmosféricas, y que además ofrece aislamiento galvánico inherente porque no hay conductor eléctrico entre los dos extremos. Se requieren convertidores TTL-a-fibra como los HFBR-1414/HFBR-2412 o módulos comerciales serial-a-fibra en cada extremo. Es más costosa que RS-485 aislado, pero se vuelve indispensable si hay riesgo frecuente de tormentas eléctricas en la zona o si en el futuro se requieren distancias mayores.

Finalmente, una alternativa que muchas veces resulta la más sencilla y económica en términos prácticos es eliminar el cable por completo y usar **comunicación inalámbrica**. Módulos LoRa como el RFM95, módulos nRF24L01+PA+LNA, módulos XBee o incluso un puente WiFi con ESP8266 o ESP32 cubren sin problemas 20 metros con línea de vista o con paredes ligeras de por medio. Esta opción aporta aislamiento total entre los dos sistemas, no requiere obra civil ni canalización entre edificios y simplifica mucho el cumplimiento normativo, a costa de tener que gestionar protocolo inalámbrico y alimentación independiente en cada extremo.

### Recomendación práctica

Para implementar físicamente el montaje de la Parte II a 20 metros entre edificios, la combinación más sensata desde el punto de vista de robustez, costo y mantenimiento es usar **RS-485 aislado** (un ADM2587E por lado, o bien MAX485 con 6N137 y DC/DC aislado) sobre cable Cat5e o Cat6, que es precisamente par trenzado AWG 24. Se usa un par para las líneas A y B, y los blindajes o pantallas del cable se conectan a tierra en un solo extremo para evitar lazos de tierra adicionales. Hay que instalar resistencias de terminación de 120 Ω en los extremos del bus y resistencias de polarización (aproximadamente 680 Ω hacia Vcc y hacia GND) en uno de los nodos para mantener el estado idle bien definido. Es muy recomendable añadir diodos TVS y, si la zona es propensa a tormentas, descargadores de gas en la entrada del cable en cada edificio para protección contra sobretensiones. A nivel de software, hay que ajustar el código de la Parte II para manejar el pin DE/RE del transceptor: ponerlo en modo transmisión antes de cada `Serial.write`, esperar con `Serial.flush()` a que termine de salir el último bit, y solo entonces regresar al modo recepción.

Si el presupuesto y la complejidad de cableado no son una restricción, o si en la zona hay tormentas eléctricas frecuentes, conviene saltarse directamente a fibra óptica o a un enlace inalámbrico, que evitan toda la discusión de tierras y reducen drásticamente el riesgo eléctrico.

### Conclusión

Conectar dos Arduinos a 20 metros entre edificios con cable AWG 24 simple y los pines TX/RX directos no es viable. En el mejor escenario habrá errores constantes de comunicación difíciles de diagnosticar; en el escenario realista se dañarán los microcontroladores la primera vez que aparezca una diferencia de potencial significativa o un transitorio inducido. El cable AWG 24 en sí es adecuado para esta distancia, siempre que sea par trenzado, pero hay que cambiar la capa eléctrica desde TTL single-ended hacia una señalización diferencial como RS-485 y añadir aislamiento galvánico por seguridad inter-edificio. La alternativa más robusta —y muchas veces más barata cuando se contabilizan todos los componentes de protección y la mano de obra de canalización— es ir directamente a fibra óptica o a un enlace inalámbrico de corto alcance.
