# 🤖 ESP32-PET

Una **mascota virtual** basada en ESP32-C3 con pantalla OLED, animaciones expresivas, notificaciones Bluetooth en tiempo real y mini-juegos retro. Todo controlado con un único botón físico.

---

## ✨ Características principales

- 🎭 **Animaciones de personalidad** — la mascota tiene vida propia con una máquina de estados que cicla entre diferentes expresiones y mini-demos automáticas
- 🔔 **Notificaciones BLE** — se conecta al smartphone vía Bluetooth y muestra notificaciones (WhatsApp, Spotify, correo, etc.) con scroll automático
- 🕹️ **4 mini-juegos retro** — RUN, FLAPPY, SPACE y PONG, jugables con el botón físico
- 🕐 **Reloj integrado** — muestra la hora sincronizada desde el teléfono en modo idle
- 📡 **Indicador de conexión** — un pequeño círculo en pantalla indica si el Bluetooth está activo

---

## 🛠️ Hardware necesario

| Componente | Detalle |
|---|---|
| Microcontrolador | ESP32-C3 (cualquier variante) |
| Pantalla | OLED SSD1306 · 128×64 px · I²C |
| Botón | Pulsador normal (con pull-up interno) |

### Conexiones de pines

| Pin | Función |
|---|---|
| GPIO 8 | SDA (I²C – pantalla) |
| GPIO 10 | SCL (I²C – pantalla) |
| GPIO 9 | Botón (con `INPUT_PULLUP`) |

> La pantalla se conecta en la dirección I²C `0x3C`.

---

## 📦 Dependencias (librerías Arduino)

Instálalas desde el **Gestor de Librerías** del IDE de Arduino:

| Librería | Descripción |
|---|---|
| `Adafruit GFX Library` | Motor gráfico base |
| `Adafruit SSD1306` | Driver de la pantalla OLED |
| `ChronosESP32` | Bluetooth + notificaciones + reloj desde el móvil |

---

## 📲 Aplicación móvil

Para recibir notificaciones y sincronizar el reloj necesitas la app **Chronos** en tu smartphone:

- **Android**: busca *Chronos* en Google Play
- El dispositivo se anuncia como `"Guibas Robot"` — selecciónalo desde la app para emparejar

---

## 🎮 Cómo usar el botón

El dispositivo se maneja con **un solo botón** usando pulsaciones cortas y largas:

### Modo IDLE (mascota)
| Acción | Resultado |
|---|---|
| **Pulsación larga** (>600 ms) | Entra al **Menú de Juegos** |

### Menú de Juegos
| Acción | Resultado |
|---|---|
| **Clic corto** | Cambia al siguiente juego |
| **Pulsación larga** | Empieza a jugar el juego seleccionado |
| **5 clics rápidos** | Vuelve al Modo IDLE |

### Jugando
| Acción | Resultado |
|---|---|
| **Clic corto** | Acción del juego (saltar, disparar…) |
| **Pulsación larga** | Vuelve al Menú de Juegos |
| **Clic tras Game Over** | Reintentar partida |

### Notificación recibida
| Acción | Resultado |
|---|---|
| **Clic corto** | Descarta la notificación |

---

## 🎭 Animaciones en Modo IDLE

La mascota cicla automáticamente por las siguientes animaciones entre cada 3-10 segundos:

| Animación | Descripción |
|---|---|
| **Ojos normales** | Cara base + hora del reloj arriba |
| **Lengua** | Cara feliz sacando la lengua |
| **Pong (demo)** | Partida de Pong automática entre IAs |
| **Snake (demo)** | Serpiente con IA moviéndose por la pantalla |
| **Efecto Matrix** | Lluvia de caracteres al estilo Matrix |
| **Mirando lados** | Los ojos se mueven de lado a lado |
| **Corazón latiendo** | Corazón pulsando animado |
| **Cara enfadada** | Cejas fruncidas con intensidad variable |
| **Cara sorprendida** | Ojos muy abiertos y boca en O |
| **Guiño** | Un ojo guiña con sonrisa |
| **Tetris (demo)** | Piezas de Tetris cayendo |
| **Pac-Man (demo)** | Pac-Man recorriendo la pantalla |

Las transiciones entre estados usan una animación fluida de **apertura/cierre de ojos**.

---

## 🕹️ Mini-juegos

### RUN
Clásico juego de *endless runner* — salta sobre los obstáculos que vienen hacia ti. Cada 5 obstáculos superados, la velocidad aumenta.

### FLAPPY
Pasa el pájaro por los huecos entre las columnas. La gravedad actúa constantemente y cada clic es un aleteo.

### SPACE
Tu nave oscila verticalmente de forma automática — tú controlas cuándo disparar. Derriba al enemigo antes de que llegue a tu posición.

### PONG
Variante jugable del Pong: mueve tu pala (izquierda) con el botón para devolver la pelota. La pala derecha es controlada por la IA. La velocidad de la pelota aumenta con cada golpe.

---

## 🔔 Sistema de notificaciones

Al recibir una notificación del móvil:

1. Se muestra una **cabecera blanca** (app o título) en la parte superior
2. El **cuerpo del mensaje** aparece con scroll automático si es largo
3. Una **barra de progreso** en la parte inferior indica el tiempo restante
4. La duración en pantalla varía entre **5 y 15 segundos** según la longitud del texto
5. Las notificaciones de **Spotify / YouTube / Music** se muestran con formato especial (canción y artista)

---

## 🚀 Instalación y carga

1. Abre `reloj.ino` en el **Arduino IDE**
2. Selecciona la placa: `ESP32C3 Dev Module` (o la variante que uses)
3. Instala las librerías listadas más arriba
4. Conecta el ESP32 por USB y selecciona el puerto COM correcto
5. Haz clic en **Subir** (→)
6. Al arrancar verás una animación de intro con el texto `GUIBAS`
7. Abre la app **Chronos** en el móvil y conéctate a `Guibas Robot`

---

## 📁 Estructura del proyecto

```
reloj/
└── reloj.ino   # Código fuente completo (único archivo)
```

---

## 👤 Autor

**Marcos Guibas** · [@MGuibas](https://github.com/MGuibas)
