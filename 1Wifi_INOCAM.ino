#include <WebServer.h>
#include <WiFi.h>
#include <esp32cam.h>

// ESTE PROGRAMA ENVIA IMAGEN SI SE COLOCA EN IP WEB, PERO SI SE COLOCA EN PYTHON ENVIA VIDEO POR LAS ITERACIONES. . . (SI FUNCIONA EN PYTHON)
const char* WIFI_SSID = "RNRED";
const char* WIFI_PASS = "A542256b7";

WebServer server(80); //servidor en el puerto 80

static auto loRes = esp32cam::Resolution::find(320, 240); //baja resolucion
static auto hiRes = esp32cam::Resolution::find(800, 600); //alta resolucion 
//static auto hiRes = esp32cam::Resolution::find(640, 480); //alta resolucion  (para tazas de fps) (IP CAM APP)


void
serveJpg() //captura imagen .jpg
{
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));

  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);  // y envia a un cliente (en este caso sera python)
}

void
handleJpgLo()  //permite enviar la resolucion de imagen baja
{
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
}

void
handleJpgHi() //permite enviar la resolucion de imagen alta
{
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  serveJpg();
}



void
setup()
{
  Serial.begin(115200);
  Serial.println();

  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);

    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMARA OK" : "CAMARA FAIL");
  }

  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS); //nos conectamos a la red wifi
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/cam-lo.jpg");//para conectarnos IP res baja

  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/cam-hi.jpg");//para conectarnos IP res alta

  server.on("/cam-lo.jpg",handleJpgLo);//enviamos al servidor
  server.on("/cam-hi.jpg", handleJpgHi);

  server.begin();
}

void loop()
{
  server.handleClient();
}
