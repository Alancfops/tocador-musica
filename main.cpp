#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseClient.h>
#include <WebServer.h>

#define WIFI_SSID "ROBOTICA_IOT"
#define WIFI_PASSWORD "aulaiot2025"

#define DATABASE_URL "https://tocador-musica-default-rtdb.firebaseio.com/"
#define DB_SECRET "AIzaSyAuRB3ZLiR1cXer8HLwgToG_P3746Kuea0"

#define BUZZER_PIN 27  

WiFiClientSecure ssl;
AsyncClientClass client(ssl);
FirebaseApp app;
RealtimeDatabase Database;
LegacyToken token(DB_SECRET);
WebServer server(80);

void tocarNota(int frequencia, int duracao) {
  ledcSetup(0, frequencia, 8);
  ledcAttachPin(BUZZER_PIN, 0);
  ledcWrite(0, 127);
  delay(duracao);
  ledcWrite(0, 0);
}

// Música Mario
void musicaMario() {
  tocarNota(660, 100); delay(150);
  tocarNota(660, 100); delay(150);
  tocarNota(660, 100); delay(150);
  tocarNota(510, 100); delay(150);
  tocarNota(660, 100); delay(150);
  tocarNota(770, 100); delay(150);
  tocarNota(380, 100); delay(150);
}

// Música Star Wars
void musicaStarWars() {
  tocarNota(440, 500); delay(100);
  tocarNota(440, 500); delay(100);
  tocarNota(440, 500); delay(100);
  tocarNota(349, 350); delay(100);
  tocarNota(523, 150); delay(100);
}

// Música Zelda
void musicaZelda() {
  tocarNota(659, 250); delay(100);
  tocarNota(698, 250); delay(100);
  tocarNota(784, 250); delay(100);
  tocarNota(880, 250); delay(100);
  tocarNota(988, 250); delay(100);
}

// Música Harry Potter
void musicaHarryPotter() {
  tocarNota(659, 400); delay(150);
  tocarNota(698, 300); delay(100);
  tocarNota(784, 400); delay(150);
  tocarNota(659, 300); delay(100);
  tocarNota(987, 400); delay(150);
  tocarNota(880, 600); delay(300);
}

// Música Piratas do Caribe
void musicaPiratasCaribe() {
  tocarNota(659, 300); delay(100);
  tocarNota(698, 300); delay(100);
  tocarNota(784, 300); delay(100);
  tocarNota(523, 300); delay(100);
  tocarNota(587, 300); delay(100);
  tocarNota(659, 500); delay(300);
}

void incrementarContagem(String nomeMusica) {
  String caminho = "/contagem/" + nomeMusica;
  int valorAtual = Database.get<int>(client, caminho);
  if (client.lastError().code() != 0) {
    valorAtual = 0;
  }
  valorAtual++;
  Database.set<int>(client, caminho, valorAtual);

  if (client.lastError().code() == 0) {
    Serial.printf("Contagem de %s atualizada para %d\n", nomeMusica.c_str(), valorAtual);
  } else {
    Serial.printf("Erro ao atualizar contagem de %s: %s\n", nomeMusica.c_str(), client.lastError().message().c_str());
  }
}

void handleRoot() {
  String html = R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Tocador de Musica</title>
      <style>
        body { font-family: Arial; text-align: center; background-color: #f2f2f2; }
        h1 { color: #333; }
        button {
          padding: 15px 30px;
          margin: 10px;
          font-size: 18px;
          background-color: #4CAF50;
          color: white;
          border: none;
          border-radius: 5px;
          cursor: pointer;
        }
        button:hover {
          background-color: #45a049;
        }
      </style>
    </head>
    <body>
      <h1>Tocador de Musica</h1>
      <button onclick="tocar('mario')">Mario</button>
      <button onclick="tocar('starwars')">Star Wars</button>
      <button onclick="tocar('zelda')">Zelda</button>
      <button onclick="tocar('harrypotter')">Harry Potter</button>
      <button onclick="tocar('piratas')">Piratas do Caribe</button>
      <script>
        function tocar(musica) {
          fetch('/tocar?musica=' + musica);
        }
      </script>
    </body>
    </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

// Rota para tocar música
void handleTocar() {
  if (server.hasArg("musica")) {
    String musica = server.arg("musica");
    Serial.println("Música solicitada: " + musica);

    if (musica == "mario") {
      musicaMario(); incrementarContagem("mario");
    } else if (musica == "starwars") {
      musicaStarWars(); incrementarContagem("starwars");
    } else if (musica == "zelda") {
      musicaZelda(); incrementarContagem("zelda");
    } else if (musica == "harrypotter") {
      musicaHarryPotter(); incrementarContagem("harrypotter");
    } else if (musica == "piratas") {
      musicaPiratasCaribe(); incrementarContagem("piratas");
    } else {
      server.send(400, "text/plain", "Musica desconhecida");
      return;
    }

    server.send(200, "text/plain", "Tocando " + musica);
  } else {
    server.send(400, "text/plain", "Parametro 'musica' ausente");
  }
}

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Conectando ao WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.println("WiFi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  ssl.setInsecure();
  initializeApp(client, app, getAuth(token));
  app.getApp(Database);
  Database.url(DATABASE_URL);

  server.on("/", handleRoot);
  server.on("/tocar", handleTocar);
  server.begin();
  Serial.println("Servidor HTTP iniciado.");
}

void loop() {
  server.handleClient();
}
