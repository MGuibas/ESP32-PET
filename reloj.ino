#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ChronosESP32.h>

// --- CONFIGURACIÓN FÍSICA ---
#define PIN_SDA 8
#define PIN_SCL 10  
#define PIN_BOTON 9 
#define ANCHO 128
#define ALTO 64

Adafruit_SSD1306 display(ANCHO, ALTO, &Wire, -1);
ChronosESP32 watch("Guibas Robot"); 

// --- MODOS PRINCIPALES ---
enum Modo { MODO_IDLE, MODO_JUEGOS };
Modo modoActual = MODO_IDLE;

// --- ESTADOS JUEGOS ---
enum GameState { MENU_JUEGOS, JUGANDO, GAMEOVER_JUEGOS };
GameState gameState = MENU_JUEGOS;
int gameSelected = 0;
const char* gameNames[] = { "RUN", "FLAPPY", "SPACE", "PONG" };

// --- SUB-MODOS ANIMACIÓN (IDLE) ---
enum AnimState { 
  OJOS_NORMALES, 
  CERRANDO_OJOS, // Transición
  SACANDO_LENGUA,
  JUGANDO_PONG,
  JUGANDO_SNAKE,
  EFECTO_MATRIX,
  MIRANDO_LADOS,
  CORAZON_LATIENDO,
  CARA_ENFADADA,
  CARA_SORPRENDIDA,
  GUINO_OJO,
  JUGANDO_TETRIS,
  JUGANDO_PACMAN,
  ABRIENDO_OJOS  // Transición
};
AnimState estadoAnimacion = OJOS_NORMALES;

// --- VARIABLES BOTÓN ---
bool botonEstadoAnterior = HIGH;
unsigned long btnPressTime = 0;
int contadorClics = 0;
unsigned long ultimoClic = 0;
const int LONG_PRESS_TIME = 600;
bool ignorarSoltar = false;
bool longPressEjecutado = false;
unsigned long tiempoEntradaMenu = 0;
const int TIEMPO_ESPERA_MENU = 1000;

// --- VARIABLES ANIMACIONES ---
unsigned long ultimoFrame = 0;
int frameCounter = 0;
int cicloAnimacion = 0; 

// Variables PONG mejoradas
float ballX = 64, ballY = 32;
float ballVX = 2, ballVY = 1.5;
int pad1Y = 20, pad2Y = 20;
int puntosJ1 = 0, puntosJ2 = 0;
float velocidadPala = 1.8;

// Variables LENGUA
int largoLengua = 0;
bool lenguaBajando = true;

// Variables SNAKE
int snakeX[20], snakeY[20];
int snakeLen = 3;
int comidaX = 80, comidaY = 40;
int dirX = 4, dirY = 0;
unsigned long ultimoMovSnake = 0;

// Variables MATRIX optimizado
int gotas[16];
unsigned long ultimoMatrix = 0;

// Variables nuevas animaciones
int parpadeoOjo = 0;
unsigned long ultimoParpadeo = 0;
int intensidadEnfado = 0;
bool enfadoCreciendo = true;

// Variables MIRAR LADOS
int miradaX = 0;
int direccionMirada = 1;
unsigned long ultimaMirada = 0;

// Variables CORAZÓN
int tamanoCorazon = 16;
bool corazonCreciendo = true;
unsigned long ultimoLatido = 0;

// Variables TETRIS
int tetrisY = 5;
int tetrisTipo = 0; // 0=L, 1=Z, 2=Cubo
unsigned long ultimoTetris = 0;

// Variables PACMAN
int pacmanX = 10, pacmanY = 32;
int pacmanDir = 1;
bool pacmanAbierto = true;
unsigned long ultimoPacman = 0;

// --- VARIABLES JUEGOS ---
int score = 0;
float objX = 0, objY = 0;
float playerY = 0, playerV = 0;
int enemyV = 4;
float wallX = 128;
int gapY = 30;
int bulletX = -10, bulletY = -10;
bool enemyAlive = true;
float ballXJuego = 64, ballYJuego = 32, ballVXJuego = 2, ballVYJuego = 1.5;

const unsigned char PROGMEM mario_run[] = { 0x0C,0x00,0x0F,0x00,0x1E,0x00,0x3D,0x80,0x1F,0x00,0x0F,0x80,0x1F,0xC0,0x24,0x40,0x06,0x00,0x0F,0x00 };
const unsigned char PROGMEM ship_bmp[] = { 0x80, 0xC0, 0xE0, 0xF0, 0xE0, 0xC0, 0x80 };

// --- VARIABLES NOTIFICACIONES ---
bool hayMensaje = false;
unsigned long tiempoLlegada = 0;
unsigned long duracionMensaje = 5000; 
String cabecera = "";
String cuerpo = "";

// --- VARIABLES DE SCROLL (AQUÍ ESTABA EL ERROR, YA AÑADIDAS) ---
int scrollY = 0; 
int alturaTexto = 0; 
unsigned long ultimoScroll = 0; 
bool bajando = true; 
unsigned long tiempoPausa = 0; 

// Las variables que faltaban:
int velocidadScroll = 100; // Velocidad del texto (ms)
int pausaLectura = 1500;   // Tiempo de espera arriba y abajo (ms)


// --- UTILIDADES ---
String limpiarTexto(String texto) {
  String salida = "";
  for (int i = 0; i < texto.length(); i++) {
    char c = texto.charAt(i);
    if (c >= 32 && c <= 126) salida += c;
  }
  return salida;
}

unsigned long calcularDuracion(String texto) {
  unsigned long tiempo = texto.length() * 150; 
  tiempo += 3000; 
  if (tiempo < 5000) tiempo = 5000;   
  if (tiempo > 15000) tiempo = 15000; 
  return tiempo;
}

int calcularAlturaPixel(String texto) {
  int lineas = (texto.length() / 21) + 1; 
  return lineas * 8; 
}

// --- DIBUJOS AVANZADOS ---

void dibujarOjos(int y, int apertura, int mirarX) {
    // OJO IZQ
    if (apertura > 2) {
       display.fillRoundRect(20, y + (25-apertura)/2, 30, apertura, 8, WHITE);
       if (apertura > 10) display.fillRect(30 + mirarX, y+8, 8, 8, BLACK);
    } else {
       display.fillRect(20, y+12, 30, 3, WHITE); 
    }
    // OJO DER
    if (apertura > 2) {
       display.fillRoundRect(78, y + (25-apertura)/2, 30, apertura, 8, WHITE);
       if (apertura > 10) display.fillRect(88 + mirarX, y+8, 8, 8, BLACK);
    } else {
       display.fillRect(78, y+12, 30, 3, WHITE); 
    }
}

void animarPong() {
  ballX += ballVX;
  ballY += ballVY;

  // Rebotes en paredes
  if (ballY <= 3) ballVY = abs(ballVY);
  if (ballY >= 61) ballVY = -abs(ballVY);
  
  // IA diferente para cada pala
  // Pala izquierda: más agresiva
  float distanciaIzq = ballY - (pad1Y + 10);
  if (ballVX < 0) { // Solo se mueve si la pelota viene hacia ella
    if (distanciaIzq > 3) pad1Y += 2.5;
    else if (distanciaIzq < -3) pad1Y -= 2.5;
  } else {
    // Movimiento aleatorio cuando no viene la pelota
    if (random(100) < 5) pad1Y += random(-2, 3);
  }
  
  // Pala derecha: más defensiva
  float distanciaDer = ballY - (pad2Y + 10);
  if (ballVX > 0) { // Solo se mueve si la pelota viene hacia ella
    if (distanciaDer > 2) pad2Y += 1.8;
    else if (distanciaDer < -2) pad2Y -= 1.8;
  } else {
    // Se centra lentamente
    if (pad2Y + 10 > 32) pad2Y -= 0.5;
    else if (pad2Y + 10 < 32) pad2Y += 0.5;
  }

  // Límites de palas
  pad1Y = constrain(pad1Y, 0, 44);
  pad2Y = constrain(pad2Y, 0, 44);

  // Rebotes en palas con efecto
  if (ballX <= 5 && ballX > 2 && ballY >= pad1Y-2 && ballY <= pad1Y+22) {
    ballVX = abs(ballVX) + 0.1;
    ballVY += (ballY - (pad1Y + 10)) * 0.15;
  }
  if (ballX >= 123 && ballX < 126 && ballY >= pad2Y-2 && ballY <= pad2Y+22) {
    ballVX = -abs(ballVX) - 0.1;
    ballVY += (ballY - (pad2Y + 10)) * 0.15;
  }

  // Puntos y reset
  if (ballX < -5) { 
    puntosJ2++; 
    ballX = 64; ballY = random(20, 44); 
    ballVX = random(15, 25) / 10.0; ballVY = random(-20, 21) / 10.0;
  }
  if (ballX > 133) { 
    puntosJ1++; 
    ballX = 64; ballY = random(20, 44); 
    ballVX = -random(15, 25) / 10.0; ballVY = random(-20, 21) / 10.0;
  }

  // Dibujar
  display.drawFastVLine(0, pad1Y, 20, WHITE);
  display.drawFastVLine(127, pad2Y, 20, WHITE);
  display.drawFastVLine(64, 0, 64, WHITE);
  display.fillCircle((int)ballX, (int)ballY, 2, WHITE);
  
  // Marcador
  display.setTextSize(1);
  display.setCursor(50, 5); display.print(puntosJ1);
  display.setCursor(70, 5); display.print(puntosJ2);
}

void animarLengua() {
  // Ojos felices ^ ^
  display.drawLine(20, 30, 35, 20, WHITE); display.drawLine(35, 20, 50, 30, WHITE);
  display.drawLine(78, 30, 93, 20, WHITE); display.drawLine(93, 20, 108, 30, WHITE);

  display.fillRect(44, 40, 40, 2, WHITE); // Boca

  if (lenguaBajando) {
    largoLengua += 2;
    if (largoLengua > 20) lenguaBajando = false;
  } else {
    largoLengua -= 2;
    if (largoLengua < 2) largoLengua = 0;
  }
  
  if (largoLengua > 0) {
     display.fillRoundRect(54, 42, 20, largoLengua, 4, WHITE);
     display.drawLine(64, 42, 64, 42 + largoLengua - 2, BLACK); 
  }
}

void animarSnake() {
  if (millis() - ultimoMovSnake > 250) {
    ultimoMovSnake = millis();
    
    // Mover cuerpo
    for (int i = snakeLen-1; i > 0; i--) {
      snakeX[i] = snakeX[i-1];
      snakeY[i] = snakeY[i-1];
    }
    
    // IA mejorada: calcular distancias
    int distX = comidaX - snakeX[0];
    int distY = comidaY - snakeY[0];
    
    // Verificar si la próxima posición chocaría con el cuerpo
    int nextX = snakeX[0];
    int nextY = snakeY[0];
    
    // Decidir dirección evitando colisiones
    bool puedeX = true, puedeY = true;
    
    if (abs(distX) > abs(distY)) {
      // Intentar moverse horizontalmente
      nextX = snakeX[0] + (distX > 0 ? 4 : -4);
      for (int i = 1; i < snakeLen; i++) {
        if (abs(nextX - snakeX[i]) < 4 && abs(snakeY[0] - snakeY[i]) < 4) {
          puedeX = false; break;
        }
      }
      if (puedeX && distX > 0 && dirX <= 0) { dirX = 4; dirY = 0; }
      else if (puedeX && distX < 0 && dirX >= 0) { dirX = -4; dirY = 0; }
      else puedeX = false;
    }
    
    if (!puedeX || abs(distY) > abs(distX)) {
      // Intentar moverse verticalmente
      nextY = snakeY[0] + (distY > 0 ? 4 : -4);
      for (int i = 1; i < snakeLen; i++) {
        if (abs(snakeX[0] - snakeX[i]) < 4 && abs(nextY - snakeY[i]) < 4) {
          puedeY = false; break;
        }
      }
      if (puedeY && distY > 0 && dirY <= 0) { dirY = 4; dirX = 0; }
      else if (puedeY && distY < 0 && dirY >= 0) { dirY = -4; dirX = 0; }
    }
    
    // Mover cabeza
    snakeX[0] += dirX;
    snakeY[0] += dirY;
    
    // Rebotes con cambio de dirección inteligente
    if (snakeX[0] < 0 || snakeX[0] > 124) {
      snakeX[0] = constrain(snakeX[0], 0, 124);
      dirX = 0;
      dirY = (comidaY > snakeY[0]) ? 4 : -4;
    }
    if (snakeY[0] < 0 || snakeY[0] > 60) {
      snakeY[0] = constrain(snakeY[0], 0, 60);
      dirY = 0;
      dirX = (comidaX > snakeX[0]) ? 4 : -4;
    }
    
    // Comer
    if (abs(snakeX[0] - comidaX) < 6 && abs(snakeY[0] - comidaY) < 6) {
      comidaX = random(10, 118);
      comidaY = random(10, 54);
      if (snakeLen < 15) snakeLen++;
    }
  }
  
  // Dibujar snake
  for (int i = 0; i < snakeLen; i++) {
    int tam = (i == 0) ? 4 : 3;
    display.fillRect(snakeX[i], snakeY[i], tam, tam, WHITE);
  }
  
  // Dibujar comida parpadeante
  if ((millis() / 300) % 2) display.fillCircle(comidaX, comidaY, 2, WHITE);
}

void animarMatrix() {
  if (millis() - ultimoMatrix > 50) {
    ultimoMatrix = millis();
    
    display.setTextColor(WHITE); 
    display.setTextSize(1);
    for (int i = 0; i < 16; i++) {
      display.setCursor(i * 8, gotas[i]);
      display.print((char)random(161, 255));
      gotas[i] += random(2, 6);
      if (gotas[i] > 64) gotas[i] = random(-20, 0);
    }
  } else {
    display.setTextColor(WHITE); 
    display.setTextSize(1);
    for (int i = 0; i < 16; i++) {
      display.setCursor(i * 8, gotas[i]);
      display.print((char)random(161, 255));
    }
  }
}

void animarCaraEnfadada() {
  if (millis() - ultimoLatido > 100) {
    ultimoLatido = millis();
    if (enfadoCreciendo) {
      intensidadEnfado += 2;
      if (intensidadEnfado > 15) enfadoCreciendo = false;
    } else {
      intensidadEnfado -= 2;
      if (intensidadEnfado < 0) enfadoCreciendo = true;
    }
  }
  
  // Cejas enfadadas
  display.drawLine(15, 25 - intensidadEnfado/3, 35, 30, WHITE);
  display.drawLine(93, 30, 113, 25 - intensidadEnfado/3, WHITE);
  
  // Ojos entrecerrados
  display.fillRect(20, 35, 30, 8, WHITE);
  display.fillRect(78, 35, 30, 8, WHITE);
  display.fillRect(25, 37, 8, 4, BLACK);
  display.fillRect(83, 37, 8, 4, BLACK);
  
  // Boca enfadada
  display.drawLine(50, 50, 78, 50, WHITE);
  display.drawLine(50, 50, 55, 45, WHITE);
  display.drawLine(78, 50, 73, 45, WHITE);
}

void animarCaraSorprendida() {
  // Ojos muy abiertos
  display.drawCircle(35, 35, 15, WHITE);
  display.drawCircle(93, 35, 15, WHITE);
  display.fillCircle(35, 35, 6, WHITE);
  display.fillCircle(93, 35, 6, WHITE);
  display.fillCircle(35, 35, 3, BLACK);
  display.fillCircle(93, 35, 3, BLACK);
  
  // Boca sorprendida
  display.drawCircle(64, 50, 8, WHITE);
}

void animarGuinoOjo() {
  if (millis() - ultimoParpadeo > 150) {
    ultimoParpadeo = millis();
    parpadeoOjo = (parpadeoOjo + 1) % 20;
  }
  
  // Ojo izquierdo guiñando
  if (parpadeoOjo < 10) {
    display.fillRect(20, 37, 30, 3, WHITE); // Cerrado
  } else {
    dibujarOjos(30, 25, 0); // Normal
    return;
  }
  
  // Ojo derecho normal
  display.fillRoundRect(78, 30, 30, 25, 8, WHITE);
  display.fillRect(88, 38, 8, 8, BLACK);
  
  // Sonrisa
  display.drawCircle(64, 45, 15, WHITE);
  display.fillRect(49, 30, 30, 15, BLACK);
}

void animarMirarLados() {
  if (millis() - ultimaMirada > 100) {
    ultimaMirada = millis();
    miradaX += direccionMirada;
    if (miradaX > 8 || miradaX < -8) direccionMirada *= -1;
  }
  
  dibujarOjos(30, 25, miradaX);
}

void animarCorazon() {
  if (millis() - ultimoLatido > 150) {
    ultimoLatido = millis();
    if (corazonCreciendo) {
      tamanoCorazon += 2;
      if (tamanoCorazon > 22) corazonCreciendo = false;
    } else {
      tamanoCorazon -= 2;
      if (tamanoCorazon < 16) corazonCreciendo = true;
    }
  }
  
  // Dibujar corazón simple
  int x = 64, y = 32;
  display.fillCircle(x-6, y-3, tamanoCorazon/3, WHITE);
  display.fillCircle(x+6, y-3, tamanoCorazon/3, WHITE);
  display.fillTriangle(x-10, y, x, y+tamanoCorazon/2, x+10, y, WHITE);
}

void animarTetris() {
  if (millis() - ultimoTetris > 300) {
    ultimoTetris = millis();
    
    // Mover pieza hacia abajo
    tetrisY += 6;
    if (tetrisY > 50) {
      tetrisY = 5;
      tetrisTipo = random(0, 3);
    }
  }
  
  // Dibujar campo
  display.drawRect(30, 5, 68, 54, WHITE);
  
  // Dibujar piezas fijas en el fondo
  // Fila Z
  display.fillRect(35, 45, 6, 6, WHITE);
  display.fillRect(41, 45, 6, 6, WHITE);
  display.fillRect(41, 39, 6, 6, WHITE);
  display.fillRect(47, 39, 6, 6, WHITE);
  
  // Algunos cubos
  display.fillRect(65, 51, 6, 6, WHITE);
  display.fillRect(71, 51, 6, 6, WHITE);
  display.fillRect(77, 51, 6, 6, WHITE);
  display.fillRect(83, 51, 6, 6, WHITE);
  
  // Dibujar pieza cayendo
  int x = 55;
  if (tetrisTipo == 0) { // L
    display.fillRect(x, tetrisY, 6, 6, WHITE);
    display.fillRect(x, tetrisY + 6, 6, 6, WHITE);
    display.fillRect(x, tetrisY + 12, 6, 6, WHITE);
    display.fillRect(x + 6, tetrisY + 12, 6, 6, WHITE);
  } else if (tetrisTipo == 1) { // Z
    display.fillRect(x, tetrisY, 6, 6, WHITE);
    display.fillRect(x + 6, tetrisY, 6, 6, WHITE);
    display.fillRect(x + 6, tetrisY + 6, 6, 6, WHITE);
    display.fillRect(x + 12, tetrisY + 6, 6, 6, WHITE);
  } else { // Cubo
    display.fillRect(x, tetrisY, 6, 6, WHITE);
    display.fillRect(x + 6, tetrisY, 6, 6, WHITE);
    display.fillRect(x, tetrisY + 6, 6, 6, WHITE);
    display.fillRect(x + 6, tetrisY + 6, 6, 6, WHITE);
  }
}

void animarPacman() {
  if (millis() - ultimoPacman > 150) {
    ultimoPacman = millis();
    
    // Mover Pacman
    pacmanX += pacmanDir * 3;
    if (pacmanX > 110 || pacmanX < 10) pacmanDir *= -1;
    
    // Alternar boca de Pacman
    pacmanAbierto = !pacmanAbierto;
  }
  
  // Dibujar puntos
  for (int i = 15; i < 115; i += 10) {
    if (abs(i - pacmanX) > 8) {
      display.fillCircle(i, 32, 1, WHITE);
    }
  }
  
  // Dibujar Pacman
  display.fillCircle(pacmanX, pacmanY, 8, WHITE);
  if (pacmanAbierto) {
    // Boca abierta
    if (pacmanDir > 0) {
      display.fillTriangle(pacmanX, pacmanY, pacmanX + 8, pacmanY - 4, pacmanX + 8, pacmanY + 4, BLACK);
    } else {
      display.fillTriangle(pacmanX, pacmanY, pacmanX - 8, pacmanY - 4, pacmanX - 8, pacmanY + 4, BLACK);
    }
  }
}

// --- JUEGOS ---
void resetGame() {
  score = 0;
  playerY = 32; playerV = 0; objX = 128; objY = random(10,50);
  
  switch(gameSelected) {
    case 0: playerY = 38; enemyV = 4; break;
    case 1: playerY = 32; wallX = 128; gapY = 30; break;
    case 2: bulletX = -10; enemyAlive = true; break;
    case 3: ballXJuego=64; ballYJuego=32; ballVXJuego=2; ballVYJuego=1.5; break;
  }
}

void updateRun() {
  playerY += playerV; playerV += 1;
  if (playerY >= 38) { playerY = 38; playerV = 0; }
  objX -= enemyV;
  if (objX < -10) { objX = 128; score++; if(score%5==0) enemyV++; }
  if (objX < 12 && objX > 2 && playerY > 28) gameState = GAMEOVER_JUEGOS;
  
  display.drawLine(0, 50, 128, 50, WHITE);
  display.drawBitmap(2, playerY, mario_run, 10, 10, WHITE);
  display.fillRect(objX, 42, 8, 8, WHITE);
}

void updateFlappy() {
  playerV += 0.5; playerY += playerV; wallX -= 2.5;
  if (wallX < -10) { wallX = 128; gapY = random(10, 45); score++; }
  if (playerY < 0 || playerY > 60) gameState = GAMEOVER_JUEGOS;
  if (wallX < 14 && wallX > 4) { if (playerY < gapY || playerY > gapY + 25) gameState = GAMEOVER_JUEGOS; }
  
  display.fillCircle(10, playerY, 3, WHITE);
  display.fillRect(wallX, 0, 8, gapY, WHITE);
  display.fillRect(wallX, gapY+25, 8, 64, WHITE);
}

void updateSpace() {
  playerY = 32 + sin(millis()/300.0) * 25;
  if (enemyAlive) { objX -= 2; if (objX < 0) gameState = GAMEOVER_JUEGOS; }
  else { objX = 130; objY = random(5, 55); enemyAlive = true; }
  if (bulletX > 0) {
     bulletX += 6; display.drawLine(bulletX, bulletY, bulletX+4, bulletY, WHITE);
     if (bulletX > 128) bulletX = -10;
     if (abs(bulletX - objX) < 8 && abs(bulletY - objY) < 8 && enemyAlive) { enemyAlive = false; score++; bulletX = -10; }
  }
  display.drawBitmap(2, playerY, ship_bmp, 8, 7, WHITE);
  if(enemyAlive) display.fillCircle(objX, objY, 4, WHITE);
}

void updatePongJuego() {
  playerV += 0.3; playerY += playerV;
  if (playerY < 0) { playerY=0; playerV=0; } if (playerY > 50) { playerY=50; playerV=0; }
  ballXJuego += ballVXJuego; ballYJuego += ballVYJuego;
  if (ballYJuego < 0 || ballYJuego > 60) ballVYJuego *= -1;
  if (ballXJuego > 124) ballVXJuego *= -1;
  if (ballXJuego < 8 && abs(playerY+7 - ballYJuego) < 10) { ballVXJuego *= -1; score++; ballVXJuego += 0.2; }
  if (ballXJuego < 0) gameState = GAMEOVER_JUEGOS;
  display.fillRect(2, playerY, 4, 14, WHITE); display.fillCircle(ballXJuego, ballYJuego, 2, WHITE); display.drawFastVLine(127, 0, 64, WHITE);
}

// --- INTRO ---
void animacionIntro() {
  for (int i=0; i<64; i+=2) {
     display.clearDisplay();
     display.drawCircle(64, 32, i, WHITE);
     display.display();
  }
  display.clearDisplay();
  display.setTextSize(2); display.setTextColor(WHITE);
  display.setCursor(28, 25); display.print("GUIBAS");
  display.display();
  delay(1000);
}

// --- CALLBACK ---
void notificationCallback(Notification notification) {
  String rawApp = notification.app;
  String rawTitle = limpiarTexto(notification.title);
  String rawMsg = limpiarTexto(notification.message);
  String todoJunto = rawApp + " " + rawTitle + " " + rawMsg;
  todoJunto.toLowerCase();
  
  bool esMusica = (todoJunto.indexOf("spotify") >= 0 || todoJunto.indexOf("music") >= 0 || todoJunto.indexOf("youtube") >= 0);

  if (esMusica) {
     cabecera = rawMsg; if (cabecera.length() < 1) cabecera = rawApp;
     cuerpo = rawTitle;
  } else {
     cabecera = rawTitle; if (cabecera == "") cabecera = rawApp;
     cuerpo = rawMsg; if (cuerpo.indexOf("mensajes nuevos") > 0) cuerpo = "[Varios mensajes]";
  }

  hayMensaje = true;
  tiempoLlegada = millis();
  duracionMensaje = calcularDuracion(cuerpo);
  scrollY = 18; alturaTexto = calcularAlturaPixel(cuerpo); bajando = true; tiempoPausa = millis(); 
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BOTON, INPUT_PULLUP);
  Wire.begin(PIN_SDA, PIN_SCL);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { for(;;); }
  
  display.setTextWrap(true); 
  animacionIntro(); 
  watch.setNotificationCallback(notificationCallback);
  watch.begin();
}

void loop() {
  watch.loop(); 
  display.clearDisplay();

  // --- BOTÓN ---
  bool lectura = digitalRead(PIN_BOTON);
  
  // Detectar pulsación
  if (botonEstadoAnterior == HIGH && lectura == LOW) {
    btnPressTime = millis();
    ignorarSoltar = false;
    longPressEjecutado = false;
  }
  
  // Detectar long press (solo una vez por pulsación)
  if (lectura == LOW && (millis() - btnPressTime > LONG_PRESS_TIME) && botonEstadoAnterior == LOW && !longPressEjecutado) {
    longPressEjecutado = true;
    ignorarSoltar = true;
    if (!hayMensaje) {
      if (modoActual == MODO_IDLE) {
        // Desde IDLE → Modo Juegos (Menú)
        modoActual = MODO_JUEGOS;
        gameState = MENU_JUEGOS;
        contadorClics = 0;
        delay(300);
        tiempoEntradaMenu = millis();
      } else if (modoActual == MODO_JUEGOS && (gameState == JUGANDO || gameState == GAMEOVER_JUEGOS)) {
        // Desde Jugando/GameOver → Menú Juegos
        gameState = MENU_JUEGOS;
        contadorClics = 0;
        delay(300);
        tiempoEntradaMenu = millis();
      }
    }
  }
  
  // Detectar clic corto al soltar (con tiempo de espera en menú)
  if (botonEstadoAnterior == LOW && lectura == HIGH) {
    if (!ignorarSoltar && millis() - btnPressTime < LONG_PRESS_TIME) {
      // Verificar si estamos en periodo de espera del menú
      if (modoActual == MODO_JUEGOS && gameState == MENU_JUEGOS && (millis() - tiempoEntradaMenu < TIEMPO_ESPERA_MENU)) {
        // Ignorar clics durante el primer segundo en el menú
        delay(50);
      } else if (hayMensaje) {
        hayMensaje = false;
      } else if (modoActual == MODO_JUEGOS) {
        // Lógica de juegos
        if (gameState == MENU_JUEGOS) {
          // En el menú: cambiar juego
          gameSelected++;
          if (gameSelected > 3) gameSelected = 0;
          
          // Contador de clics para salir al IDLE (5 clics rápidos)
          if (millis() - ultimoClic < 500) {
            contadorClics++;
            if (contadorClics >= 5) {
              modoActual = MODO_IDLE;
              estadoAnimacion = OJOS_NORMALES;
              ultimoFrame = millis();
              contadorClics = 0;
            }
          } else {
            contadorClics = 1;
          }
          ultimoClic = millis();
          
        } else if (gameState == JUGANDO) {
          // Acción del juego
          switch(gameSelected) {
            case 0: if (playerY >= 38) playerV = -6; break;
            case 1: playerV = -4; break;
            case 2: if (bulletX < 0) { bulletX = 10; bulletY = playerY + 3; } break;
            case 3: playerV = -5; break;
          }
        } else if (gameState == GAMEOVER_JUEGOS) {
          // Reintentar
          gameState = JUGANDO;
          resetGame();
        }
      }
    }
    delay(50);
  }
  botonEstadoAnterior = lectura;

  // --- PANTALLA ---
  if (hayMensaje) {
      if (millis() - tiempoLlegada > duracionMensaje) hayMensaje = false; 
      display.fillRect(0, 0, 128, 14, WHITE);
      display.setTextColor(BLACK); display.setTextSize(1); display.setCursor(2, 3);
      display.print(cabecera.substring(0, 20)); 

      int alturaNormal = calcularAlturaPixel(cuerpo); 
      if (alturaNormal > 45) {
         if (millis() - tiempoPausa > pausaLectura) { 
            if (millis() - ultimoScroll > velocidadScroll) {
               ultimoScroll = millis();
               if (bajando) { scrollY--; if (scrollY < -(alturaNormal - 40)) { bajando = false; tiempoPausa = millis(); } } 
               else { scrollY+=2; if (scrollY >= 18) { scrollY = 18; bajando = true; tiempoPausa = millis(); } }
            }
         }
      } else { scrollY = 18; }

      display.setTextColor(WHITE); display.setTextSize(1); display.setCursor(0, scrollY);
      display.println(cuerpo);
      int anchoBarra = map(millis() - tiempoLlegada, 0, duracionMensaje, 128, 0);
      display.fillRect(0, 62, anchoBarra, 2, WHITE);
  }

  // --- MODO IDLE (ANIMACIONES FLUIDAS) ---
  else if (modoActual == MODO_IDLE) {
      
      switch (estadoAnimacion) {
        
        case OJOS_NORMALES:
        {
          // Mostrar hora en modo idle
          display.setTextColor(WHITE);
          String tiempo = watch.getTime();
          display.setTextSize(2); 
          display.setCursor(34, 0); 
          display.print(tiempo.substring(0, 5));
          
          dibujarOjos(30, 25, 0);
          if (millis() - ultimoFrame > 3000) {
             estadoAnimacion = CERRANDO_OJOS;
             frameCounter = 25; 
          }
          break;
        }

        case CERRANDO_OJOS:
          dibujarOjos(30, frameCounter, 0);
          frameCounter -= 5; 
          if (frameCounter <= 0) {
             cicloAnimacion++;
             if (cicloAnimacion > 9) cicloAnimacion = 0; 
             
             switch(cicloAnimacion) {
               case 0: 
                 estadoAnimacion = SACANDO_LENGUA; 
                 largoLengua = 0; lenguaBajando = true;
                 break;
               case 1: 
                 estadoAnimacion = JUGANDO_PONG; 
                 ballX = 64; ballY = 32; puntosJ1 = puntosJ2 = 0;
                 ultimoFrame = millis(); 
                 break;
               case 2:
                 estadoAnimacion = JUGANDO_SNAKE;
                 for(int i = 0; i < 3; i++) { snakeX[i] = 20 - i*4; snakeY[i] = 32; }
                 snakeLen = 3; dirX = 4; dirY = 0;
                 ultimoFrame = millis();
                 break;
               case 3:
                 estadoAnimacion = EFECTO_MATRIX;
                 for(int i = 0; i < 16; i++) { gotas[i] = random(-20, 0); }
                 ultimoFrame = millis();
                 break;
               case 4:
                 estadoAnimacion = MIRANDO_LADOS;
                 miradaX = 0; direccionMirada = 1;
                 ultimoFrame = millis();
                 break;
               case 5:
                 estadoAnimacion = CORAZON_LATIENDO;
                 tamanoCorazon = 16; corazonCreciendo = true;
                 ultimoFrame = millis();
                 break;
               case 6:
                 estadoAnimacion = CARA_ENFADADA;
                 intensidadEnfado = 0; enfadoCreciendo = true;
                 ultimoFrame = millis();
                 break;
               case 7:
                 estadoAnimacion = JUGANDO_TETRIS;
                 tetrisY = 5; tetrisTipo = 0;
                 ultimoFrame = millis();
                 break;
               case 8:
                 estadoAnimacion = JUGANDO_PACMAN;
                 pacmanX = 10; pacmanDir = 1;
                 ultimoFrame = millis();
                 break;
               case 9:
                 estadoAnimacion = (random(2) == 0) ? CARA_SORPRENDIDA : GUINO_OJO;
                 parpadeoOjo = 0;
                 ultimoFrame = millis();
                 break;
             }
          }
          break;

        case SACANDO_LENGUA:
          animarLengua();
          if (!lenguaBajando && largoLengua <= 0) {
             estadoAnimacion = ABRIENDO_OJOS;
             frameCounter = 0; 
          }
          break;

        case JUGANDO_PONG:
          animarPong();
          if (millis() - ultimoFrame > 8000 || puntosJ1 >= 3 || puntosJ2 >= 3) {
             estadoAnimacion = ABRIENDO_OJOS;
             frameCounter = 0;
          }
          break;
          
        case JUGANDO_SNAKE:
          animarSnake();
          if (millis() - ultimoFrame > 10000) {
             estadoAnimacion = ABRIENDO_OJOS;
             frameCounter = 0;
          }
          break;
          
        case EFECTO_MATRIX:
          animarMatrix();
          if (millis() - ultimoFrame > 6000) {
             estadoAnimacion = ABRIENDO_OJOS;
             frameCounter = 0;
          }
          break;
          
        case MIRANDO_LADOS:
          animarMirarLados();
          if (millis() - ultimoFrame > 4000) {
             estadoAnimacion = ABRIENDO_OJOS;
             frameCounter = 0;
          }
          break;
          
        case CORAZON_LATIENDO:
          animarCorazon();
          if (millis() - ultimoFrame > 5000) {
             estadoAnimacion = ABRIENDO_OJOS;
             frameCounter = 0;
          }
          break;
          
        case CARA_ENFADADA:
          animarCaraEnfadada();
          if (millis() - ultimoFrame > 3000) {
             estadoAnimacion = ABRIENDO_OJOS;
             frameCounter = 0;
          }
          break;
          
        case CARA_SORPRENDIDA:
          animarCaraSorprendida();
          if (millis() - ultimoFrame > 2500) {
             estadoAnimacion = ABRIENDO_OJOS;
             frameCounter = 0;
          }
          break;
          
        case GUINO_OJO:
          animarGuinoOjo();
          if (millis() - ultimoFrame > 3000) {
             estadoAnimacion = ABRIENDO_OJOS;
             frameCounter = 0;
          }
          break;
          
        case JUGANDO_TETRIS:
          animarTetris();
          if (millis() - ultimoFrame > 7000) {
             estadoAnimacion = ABRIENDO_OJOS;
             frameCounter = 0;
          }
          break;
          
        case JUGANDO_PACMAN:
          animarPacman();
          if (millis() - ultimoFrame > 6000) {
             estadoAnimacion = ABRIENDO_OJOS;
             frameCounter = 0;
          }
          break;

        case ABRIENDO_OJOS:
          dibujarOjos(30, frameCounter, 0);
          frameCounter += 5;
          if (frameCounter >= 25) {
             estadoAnimacion = OJOS_NORMALES;
             ultimoFrame = millis();
          }
          break;
      }
      delay(30); 
  }
  
  // --- MODO JUEGOS ---
  else if (modoActual == MODO_JUEGOS) {
    if (gameState == MENU_JUEGOS) {
      display.fillTriangle(5, 30, 15, 25, 15, 35, WHITE);
      display.fillTriangle(123, 30, 113, 25, 113, 35, WHITE);
      display.setTextSize(2);
      int len = strlen(gameNames[gameSelected]);
      display.setCursor(64 - (len*6), 25);
      display.print(gameNames[gameSelected]);
      display.setTextSize(1);
      display.setCursor(15, 55); display.print("Click:Sig Hold:Go");
      
      // Detectar long press para entrar
      if (digitalRead(PIN_BOTON) == LOW && (millis() - btnPressTime > LONG_PRESS_TIME)) {
        gameState = JUGANDO;
        resetGame();
        delay(300);
      }
    }
    else if (gameState == JUGANDO) {
      switch(gameSelected) {
        case 0: updateRun(); break;
        case 1: updateFlappy(); break;
        case 2: updateSpace(); break;
        case 3: updatePongJuego(); break;
      }
      display.setCursor(110, 0); display.setTextSize(1); display.print(score);
    }
    else if (gameState == GAMEOVER_JUEGOS) {
      display.setCursor(10, 15); display.setTextSize(2); display.print("GAME OVER");
      display.setCursor(40, 35); display.setTextSize(1); display.print("Score: "); display.print(score);
      display.setCursor(20, 50); display.print("Click:Retry");
    }
    delay(30);
  }

  if (modoActual == MODO_IDLE && watch.isConnected()) display.fillCircle(122, 5, 3, WHITE);
  else if (modoActual == MODO_IDLE) display.drawCircle(122, 5, 3, WHITE); 
  
  display.display();
}