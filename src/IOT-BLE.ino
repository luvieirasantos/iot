// Inclui as bibliotecas principais do BLE do ESP32
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

// Ponteiros globais para o servidor BLE e para a característica BLE
BLEServer* pServer = nullptr;
BLECharacteristic* pCharacteristic = nullptr;

// Variável booleana que indica se há um cliente conectado
bool deviceConnected = false;

// UUIDs do serviço e da característica BLE (identificadores únicos para BLE)
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// Lista de possíveis zonas que podem ser enviadas via BLE
const char* zonas[] = {"A1", "A2", "B1", "B2", "C1", "C2"};
const int numZonas = sizeof(zonas) / sizeof(zonas[0]); // Calcula o número total de zonas

// Classe de callbacks personalizada para tratar eventos de conexão e desconexão do cliente BLE
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) override {
    // Quando um cliente conecta, define a variável como true e imprime mensagem
    deviceConnected = true;
    Serial.println("✅ Cliente conectado!");
  }

  void onDisconnect(BLEServer* pServer) override {
    // Quando um cliente desconecta, define a variável como false, imprime mensagem,
    // espera um pouco e reinicia o advertising para aceitar novas conexões
    deviceConnected = false;
    Serial.println("Cliente desconectado. Reiniciando BLE...");
    delay(100);
    BLEDevice::startAdvertising();
  }
};

void setup() {
  // Inicializa a comunicação serial para debug (velocidade 115200 bauds)
  Serial.begin(115200);

  // Inicializa o dispositivo BLE com o nome "Moto_01_v2"
  BLEDevice::init("Moto_01_v2");

  // Cria o servidor BLE e define os callbacks personalizados para eventos de conexão/desconexão
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Cria o serviço BLE utilizando o UUID definido
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Cria a característica BLE associada ao serviço, permitindo leitura e notificação
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );

  // Adiciona um descritor (necessário para notificações no BLE)
  pCharacteristic->addDescriptor(new BLE2902());
  // Define um valor inicial para a característica BLE
  pCharacteristic->setValue("ZONA:A1");

  // Inicia o serviço BLE (tem que ser feito antes do advertising)
  pService->start();

  // Prepara e inicia o advertising BLE para que outros dispositivos possam encontrar o ESP32
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  Serial.println("BLE pronto, aguardando conexão...");

  // Inicializa o gerador de números aleatórios usando um valor lido de um pino analógico (mais aleatório)
  randomSeed(analogRead(0));
}

void loop() {
  // Executa este bloco somente se houver um cliente BLE conectado
  if (deviceConnected) {
    // Seleciona aleatoriamente um índice da lista de zonas
    int idx = random(numZonas);
    // Monta a string da zona aleatória no formato "ZONA:Xn"
    String zonaAleatoria = "ZONA:" + String(zonas[idx]);
    
    // Atualiza o valor da característica BLE com a zona sorteada e notifica o cliente
    pCharacteristic->setValue(zonaAleatoria.c_str());
    pCharacteristic->notify();
    Serial.print("📤 Notificando: ");
    Serial.println(zonaAleatoria);

    // Aguarda 2 segundos antes de enviar a próxima notificação
    delay(2000);
  }
}
