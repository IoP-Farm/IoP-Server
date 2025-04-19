#include "../../include/network/wifi_manager.h"

namespace farm::net
{
    // Конструктор
    MyWiFiManager::MyWiFiManager(std::shared_ptr<farm::log::ILogger> logger)
    {
        // Если логгер не передан, создаем новый с помощью фабрики
        if (!logger) 
        {
            this->logger = farm::log::LoggerFactory::createSerialLogger(farm::log::Level::Info);
        } 
        else 
        {
            this->logger = logger;
        }
        
        // Настройка WiFiManager
        wifiManager.setDebugOutput(false);
        wifiManager.setConnectTimeout(WIFI_CONNECT_TIMEOUT);
        wifiManager.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT);
        wifiManager.setConfigPortalBlocking(false);

        // Обратный вызов при активации точки доступа
        wifiManager.setAPCallback([this](WiFiManager* wm) {
            portalActive = true;
            this->logger->log(farm::log::Level::Info, 
                     "[WiFi] Portal activated: %s (password: %s)", 
                     apName.c_str(), apPassword.c_str());
        });
    }
    
    // Инициализация WiFi и попытка подключения
    bool MyWiFiManager::initialize()
    {
            // Настройка точки доступа и имени хоста
    setAccessPointCredentials("IoP-Farm-AP", "12345678");
    setHostName("iop-farm-device");

    // Включение встроенной отладки WiFiManager
#ifdef IOP_DEBUG
    setDebugOutput(true, "[DEBUG] [WM]   ");
#endif

        logger->log(farm::log::Level::Info, "[WiFi] Initializing");
        
        // Устанавливаем имя хоста, если оно было задано
        if (hostName.length() > 0) 
        {
            WiFi.setHostname(hostName.c_str());
        }
        
        // Попытка автоматического подключения
        logger->log(farm::log::Level::Debug, "[WiFi] Running autoConnect with timeout %d sec", WIFI_CONNECT_TIMEOUT);
        bool connected = wifiManager.autoConnect(
            apName.c_str(), 
            apPassword.length() > 0 ? apPassword.c_str() : nullptr
        );
        
        if (connected) 
        {
            // Если подключение успешно, значит настройки были сохранены или введены
            // в процессе работы портала конфигурации
            logger->log(farm::log::Level::Info, 
                      "[WiFi] Connected to %s (IP: %s, RSSI: %d dBm)", 
                      WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), WiFi.RSSI());
            
            // Если подключение произошло к новой сети через портал, дополнительно логируем
            if (portalActive) 
            {
                logger->log(farm::log::Level::Info,
                         "[WiFi] New network credentials saved");
            }
        }
        else
        {
            logger->log(farm::log::Level::Warning, 
                      "[WiFi] AutoConnect failed to establish connection");
        }
        
        return connected;
    }
    
    // Поддержание WiFi соединения - вызывать в цикле loop()
    void MyWiFiManager::maintainConnection()
    {
        // Обработка процессов WiFiManager в неблокирующем режиме
        wifiManager.process();
        
        // Обработка активного портала конфигурации
        if (portalActive) 
        {
            // Если подключение установлено через портал
            if (WiFi.status() == WL_CONNECTED) 
            {
                logger->log(farm::log::Level::Info, 
                          "[WiFi] Connection established via config portal");
                logger->log(farm::log::Level::Info, 
                          "[WiFi] Network: %s, IP: %s", 
                          WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
                
                // Закрываем портал только если он действительно активен
                if (isConfigPortalActive())
                {
                    logger->log(farm::log::Level::Debug, "[WiFi] Closing config portal");
                    wifiManager.stopConfigPortal();
                }
                
                // Сбрасываем флаги и счетчики
                portalActive = false;
                reconnectAttempts = 0;
                logger->log(farm::log::Level::Debug, "[WiFi] Portal inactive");
            }
            
            return; // Пока портал активен, не выполняем другие проверки
        }
        
        // Периодическая проверка WiFi соединения
        unsigned long currentMillis = millis();
        if (currentMillis - lastCheckTime >= WIFI_CHECK_INTERVAL) 
        {
            lastCheckTime = currentMillis;
            
            // Проверка наличия соединения
            if (WiFi.status() != WL_CONNECTED) 
            {
                // Первая потеря соединения или продолжающиеся попытки
                if (reconnectAttempts == 0) {
                    logger->log(farm::log::Level::Warning, "[WiFi] Connection lost");
                }
                
                // Не пытаемся переподключаться слишком часто
                if (currentMillis - lastReconnectTime >= RECONNECT_RETRY_INTERVAL) {
                    lastReconnectTime = currentMillis;
                    reconnectAttempts++;
                    
                    logger->log(farm::log::Level::Debug, 
                              "[WiFi] Reconnection attempt %d of %d", 
                              reconnectAttempts, MAX_RECONNECT_ATTEMPTS);
                    
                    // После достижения максимального числа попыток запускаем портал
                    if (reconnectAttempts >= MAX_RECONNECT_ATTEMPTS) 
                    {
                        logger->log(farm::log::Level::Warning, 
                                  "[WiFi] Failed to reconnect after %d attempts, starting config portal", 
                                  reconnectAttempts);
                        
                        // Запускаем портал конфигурации
                        portalActive = true;
                        wifiManager.startConfigPortal(
                            apName.c_str(), 
                            apPassword.length() > 0 ? apPassword.c_str() : nullptr
                        );
                        
                        // Сбрасываем счетчик попыток
                        reconnectAttempts = 0;
                    }
                    else 
                    {
                        // Простая попытка переподключения к последней сети
                        logger->log(farm::log::Level::Info, "[WiFi] Attempting reconnection");
                        WiFi.reconnect();
                    }
                }
            }
            else if (reconnectAttempts > 0) {
                // Если соединение восстановлено после попыток переподключения
                logger->log(farm::log::Level::Info, 
                          "[WiFi] Reconnected to %s (IP: %s, RSSI: %d dBm)", 
                          WiFi.SSID().c_str(), WiFi.localIP().toString().c_str(), WiFi.RSSI());
                reconnectAttempts = 0;
            }
        }
    }
    
    // Включение/выключение встроенной отладки WiFiManager
    void MyWiFiManager::setDebugOutput(bool enable)
    {
        wifiManager.setDebugOutput(enable);
        logger->log(farm::log::Level::Debug, 
                  "[WiFi] Debug output %s", 
                  enable ? "enabled" : "disabled");
    }

    // Включение/выключение встроенной отладки WiFiManager с префиксом
    void MyWiFiManager::setDebugOutput(bool enable, const String& prefix)
    {
        wifiManager.setDebugOutput(enable, prefix);
        logger->log(farm::log::Level::Debug, 
                  "[WiFi] Debug output %s with prefix: %s",
                  enable ? "enabled" : "disabled", prefix.c_str());
    }

    // Получение прямого доступа к WiFiManager для продвинутой настройки
    WiFiManager* MyWiFiManager::getWiFiManager()
    {
        return &wifiManager;
    }
    
    // Проверка наличия соединения
    bool MyWiFiManager::isConnected() const
    {
        return WiFi.status() == WL_CONNECTED;
    }
        
    // Проверка активности портала конфигурации через базовую библиотеку
    bool MyWiFiManager::isConfigPortalActive()
    {
        return wifiManager.getConfigPortalActive();
    }
        
    // Установка имени и пароля точки доступа
    void MyWiFiManager::setAccessPointCredentials(const String& name, const String& password)
    {
        apName = name;
        apPassword = password;
        logger->log(farm::log::Level::Debug, 
                  "[WiFi] AP credentials set: %s (password: %s)", 
                     apName.c_str(), apPassword.c_str());
    }
    
    // Установка имени хоста
    void MyWiFiManager::setHostName(const String& name)
    {
        hostName = name;
        if (name.length() > 0)
        {
            WiFi.setHostname(name.c_str());
            logger->log(farm::log::Level::Debug, 
                      "[WiFi] Hostname set: %s", name.c_str());
        }
    }
        
    // Сброс всех настроек WiFi
    bool MyWiFiManager::resetSettings()
    {   
        logger->log(farm::log::Level::Warning, "[WiFi] Erasing stored credentials");
        
        // Вызываем метод базового класса для сброса настроек
        wifiManager.resetSettings();
        
        logger->log(farm::log::Level::Info, "[WiFi] Network settings erased");
        
        return true; // Возвращаем успешность операции
    }
        
    // Проверка наличия сохраненных настроек WiFi
    // TODO: исправить ошибку. После сброса настроек все равно видит сохраненные настройки
    bool MyWiFiManager::checkWifiSaved()
    {
        bool isSaved = wifiManager.getWiFiIsSaved();
        
        if (isSaved) 
        {
            logger->log(farm::log::Level::Info, 
                      "[WiFi] Found saved credentials");
            
            // На этом этапе WiFi.SSID() еще недоступен, WiFiManager покажет SSID позже
            // во время подключения (см. лог "*wm:Connecting to SAVED AP: xxx")
        }
        else
        {
            logger->log(farm::log::Level::Info, 
                      "[WiFi] No saved networks found");
        }
        
        return isSaved;
    }
}
