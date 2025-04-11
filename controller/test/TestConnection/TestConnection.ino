#include <WiFi.h>

const char* ssid = "VENOM";
const char* password = "12345678";

// Таймаут подключения (секунды)
const unsigned long CONNECT_TIMEOUT = 20;
unsigned long connectStartTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n\nStarting WiFi Diagnostics");
  
  // Вывод информации о чипе
  Serial.printf("Chip ID: 0x%08X\n", ESP.getEfuseMac());
  Serial.printf("SDK Version: %s\n", ESP.getSdkVersion());
  Serial.printf("MAC Address: %s\n\n", WiFi.macAddress().c_str());

  // Регистрация обработчиков событий WiFi
  WiFi.onEvent(wifiEventHandler);
  
  scanNetworks(); // Сканирование доступных сетей
  
  Serial.println("\nStarting connection attempt...");
  connectStartTime = millis();
  WiFi.begin(ssid, password);
}

void loop() {
  static bool connectionAttempted = false;
  
  if (WiFi.status() != WL_CONNECTED) {
    if (millis() - connectStartTime > CONNECT_TIMEOUT * 1000 && !connectionAttempted) {
      Serial.println("\n\n=== Connection Timeout ===");
      Serial.println("Possible reasons:");
      Serial.println("- Incorrect password");
      Serial.println("- Weak signal strength");
      Serial.println("- Router authentication issues");
      Serial.println("- IP address conflict");
      Serial.println("- Hardware issues");
      suggestTroubleshootingSteps();
      connectionAttempted = true;
    }
  } else {
    if (!connectionAttempted) {
      printConnectionDetails();
      connectionAttempted = true;
    }
  }
}

void wifiEventHandler(WiFiEvent_t event, WiFiEventInfo_t info) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_START:
      Serial.println("[EVENT] Station Started");
      break;
      
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("[EVENT] Connected to AP");
      Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
      Serial.printf("BSSID: %s\n", WiFi.BSSIDstr().c_str());
      Serial.printf("Channel: %d\n", WiFi.channel());
      break;
      
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("[EVENT] Got IP: ");
      Serial.println(WiFi.localIP());
      Serial.printf("Subnet Mask: %s\n", WiFi.subnetMask().toString().c_str());
      Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
      Serial.printf("DNS 1: %s\n", WiFi.dnsIP(0).toString().c_str());
      Serial.printf("DNS 2: %s\n", WiFi.dnsIP(1).toString().c_str());
      break;
      
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("[EVENT] Disconnected from AP");
      printDisconnectReason(info.wifi_sta_disconnected.reason);
      Serial.println("Attempting reconnection...");
      WiFi.reconnect();
      break;
      
    default: break;
  }
}

void scanNetworks() {
  Serial.println("Scanning WiFi networks...");
  int networks = WiFi.scanNetworks();
  
  if (networks == 0) {
    Serial.println("No networks found");
  } else {
    Serial.printf("Found %d networks:\n", networks);
    for (int i = 0; i < networks; i++) {
      Serial.printf("%2d: %-32s (%3d dBm) Ch%2d %s\n", 
                   i+1, 
                   WiFi.SSID(i).c_str(), 
                   WiFi.RSSI(i), 
                   WiFi.channel(i),
                   encryptionTypeToString(WiFi.encryptionType(i)));
    }
  }
  WiFi.scanDelete();
}

String encryptionTypeToString(wifi_auth_mode_t type) {
  switch (type) {
    case WIFI_AUTH_OPEN: return "Open";
    case WIFI_AUTH_WEP: return "WEP";
    case WIFI_AUTH_WPA_PSK: return "WPA";
    case WIFI_AUTH_WPA2_PSK: return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK: return "WPA/WPA2";
    default: return "Unknown";
  }
}

void printDisconnectReason(uint8_t reason) {
  switch (reason) {
    case WIFI_REASON_UNSPECIFIED:
      Serial.println("Reason: Unspecified error");
      break;
    case WIFI_REASON_AUTH_EXPIRE:
      Serial.println("Reason: Authentication expired");
      break;
    case WIFI_REASON_AUTH_LEAVE:
      Serial.println("Reason: Explicit deauthentication");
      break;
    case WIFI_REASON_ASSOC_EXPIRE:
      Serial.println("Reason: Association expired");
      break;
    case WIFI_REASON_ASSOC_TOOMANY:
      Serial.println("Reason: Too many associations");
      break;
    case WIFI_REASON_NOT_AUTHED:
      Serial.println("Reason: Not authenticated");
      break;
    case WIFI_REASON_NOT_ASSOCED:
      Serial.println("Reason: Not associated");
      break;
    case WIFI_REASON_ASSOC_LEAVE:
      Serial.println("Reason: Explicit disassociation");
      break;
    case WIFI_REASON_ASSOC_NOT_AUTHED:
      Serial.println("Reason: Association without authentication");
      break;
    case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
      Serial.println("Reason: 4-way handshake timeout (wrong password?)");
      break;
    default:
      Serial.printf("Reason: %d (see WiFiReasonCode.h)\n", reason);
  }
}

void printConnectionDetails() {
  Serial.println("\n=== Connection Details ===");
  Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
  Serial.printf("Signal Strength: %d dBm\n", WiFi.RSSI());
  Serial.printf("Channel: %d\n", WiFi.channel());
  Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("Subnet Mask: %s\n", WiFi.subnetMask().toString().c_str());
  Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
  Serial.printf("DNS Server: %s\n", WiFi.dnsIP().toString().c_str());
}

void suggestTroubleshootingSteps() {
  Serial.println("\n=== Troubleshooting Steps ===");
  Serial.println("1. Check if SSID is visible in network scan");
  Serial.println("2. Verify password correctness");
  Serial.println("3. Check router's security settings (WPA2 recommended)");
  Serial.println("4. Move closer to the router");
  Serial.println("5. Check for MAC address filtering in router");
  Serial.println("6. Try different WiFi channel (1, 6 or 11)");
  Serial.println("7. Test with another device to confirm network availability");
  Serial.println("8. Check for firmware updates on router and ESP32");
  Serial.println("9. Try power cycling both router and ESP32");
}