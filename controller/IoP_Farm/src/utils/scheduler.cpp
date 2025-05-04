#include "utils/scheduler.h"
#include "utils/logger_factory.h"
#include "config/constants.h"
#include <cstdint>

using namespace farm::config::scheduler;
using namespace farm::config;
namespace farm::utils
{
    // Инициализация статического члена
    std::shared_ptr<Scheduler> Scheduler::instance = nullptr;
    
    // Конструктор
    Scheduler::Scheduler(std::shared_ptr<farm::log::ILogger> logger)
    {
        // Инициализация логгера
        if (!logger) 
        {
            this->logger = farm::log::LoggerFactory::createSerialLogger(farm::log::Level::Info);
        } 
        else 
        {
            this->logger = logger;
        }
    }
    
    // Деструктор
    Scheduler::~Scheduler()
    {
#ifdef USE_FREERTOS
        // Останавливаем задачу планировщика, если она запущена
        stopSchedulerTask();
        
        // Удаляем мьютекс
        if (eventsMutex != nullptr)
        {
            vSemaphoreDelete(eventsMutex);
            eventsMutex = nullptr;
        }
#endif
        
        logger->log(farm::log::Level::Debug, 
                  "[Scheduler] Освобождение ресурсов планировщика");
    }

    bool Scheduler::isNtpOnline() const
    {
        return NTP.online();
    }
    
    // Инициализация планировщика
    bool Scheduler::initialize(int8_t gmtOffset)
    {
        if (initialized)
        {
            logger->log(farm::log::Level::Warning,
                      "[Scheduler] Планировщик уже был инициализирован");
            return true;
        }
        
        logger->log(farm::log::Level::Farm,
                  "[Scheduler] Инициализация планировщика задач");
                  
        // Инициализация NTP
        NTP.setPeriod(time::NTP_PERIOD); // период синхронизации времени с NTP сервером
        NTP.begin(gmtOffset);

        delay(100);
        
#ifdef USE_FREERTOS
        // Инициализация мьютекса
        eventsMutex = xSemaphoreCreateMutex();
        if (eventsMutex == nullptr)
        {
            logger->log(farm::log::Level::Error, 
                      "[Scheduler] Не удалось создать мьютекс событий");
            return false;
        }
        
        logger->log(farm::log::Level::Debug,
                  "[Scheduler] Мьютекс событий создан");
#endif

        if (NTP.updateNow())
        {
            logger->log(farm::log::Level::Debug, 
                      "[Scheduler] NTP синхронизирован, текущее время: %s", NTP.toString().c_str());
        }
        else
        {
            // в дальнейшем будет проверка NTP.online() == true/false во всех методах Scheduler'a
            logger->log(farm::log::Level::Warning, 
                      "[Scheduler] NTP не синхронизирован, все функции планировщика отключены");
            logger->log(farm::log::Level::Warning, 
                      "[Scheduler] Подключите устройство к WiFi либо синхронизируйте время по RTC");
        }

        initialized = true;
        return true;
    }

    bool Scheduler::isInitialized() const
    {
        return initialized;
    }
    
    // Получение экземпляра синглтона
    std::shared_ptr<Scheduler> Scheduler::getInstance(std::shared_ptr<farm::log::ILogger> logger)
    {
        if (instance == nullptr) 
        {
            // Используем явное создание вместо make_shared, т.к. конструктор приватный
            instance = std::shared_ptr<Scheduler>(new Scheduler(logger));
        }
        return instance;
    }
    
    // Добавление одноразового события через указанное количество секунд
    std::uint64_t Scheduler::scheduleOnceAfter(uint32_t afterSeconds, std::function<void()> callback)
    {
        if (!initialized)
        {
            logger->log(farm::log::Level::Error,
                     "[Scheduler] Попытка добавить событие до инициализации планировщика");
            return 0;
        }

        if (!isNtpOnline()) return 0;
        
        uint32_t targetTime = NTP.getUnix() + afterSeconds;
        return scheduleOnceAt(targetTime, callback);
    }
    
    // Добавление одноразового события в конкретное время (Datime)
    std::uint64_t Scheduler::scheduleOnceAt(const Datime& dateTime, std::function<void()> callback)
    {
        if (!initialized)
        {
            logger->log(farm::log::Level::Error,
                     "[Scheduler] Попытка добавить событие до инициализации планировщика");
            return 0;
        }
        
        return scheduleOnceAt(dateTime.getUnix(), callback);
    }
    
    // Добавление одноразового события в конкретное время (unix)
    std::uint64_t Scheduler::scheduleOnceAt(uint32_t unixTime, std::function<void()> callback)
    {
        if (!initialized)
        {
            logger->log(farm::log::Level::Error,
                     "[Scheduler] Попытка добавить событие до инициализации планировщика");
            return 0;
        }
        
        if (!isNtpOnline()) return 0;
        
        if (!callback) 
        {
            logger->log(farm::log::Level::Error, 
                    "[Scheduler] Попытка добавить событие с пустым обработчиком");
            return 0;
        }
         
        uint32_t currentTime = NTP.getUnix();
        
        // Если запланированное время уже прошло, переносим на завтрашний день
        if (unixTime < currentTime) 
        {
            Datime unixTimeDatime(unixTime);
            logger->log(farm::log::Level::Info, 
                       "[Scheduler] Запланированное время %u уже прошло, переносим на завтра", unixTimeDatime.toString());
            
            // Извлекаем компоненты времени из unixTime (нам нужны только часы, минуты и секунды)
            Datime origTime(unixTime);
            
            // Берем текущее время и создаем дату на завтра
            Datime currentDate(currentTime);
            
            // Создаем временную точку для завтрашнего дня
            time_t tomorrow_time = currentTime + 24*60*60; // текущее время + 24 часа
            Datime tomorrow(tomorrow_time);
            
            // Устанавливаем время суток из оригинальной даты
            tomorrow.hour = origTime.hour;
            tomorrow.minute = origTime.minute;
            tomorrow.second = origTime.second;
            
            // Получаем новое unix-время
            unixTime = tomorrow.getUnix();
            
            logger->log(farm::log::Level::Info, 
                       "[Scheduler] Новое запланированное время: %u", tomorrow.toString());
        }
        
        // Создаем новое событие
        ScheduledEvent event;
        event.type = ScheduleType::ONCE;
        event.scheduledTime = unixTime;
        event.callback = callback;
        event.id = nextEventId++;

#ifdef USE_FREERTOS
        // Блокируем доступ к events с помощью мьютекса
        if (eventsMutex != nullptr && xSemaphoreTake(eventsMutex, portMAX_DELAY) != pdTRUE)
        {
            logger->log(farm::log::Level::Error, 
                      "[Scheduler] Не удалось получить мьютекс для добавления события");
            return 0;
        }
#endif

        events.push_back(event);
        
        Datime dt(unixTime);

        logger->log(farm::log::Level::Debug, 
                  "[Scheduler] Добавлено однократное событие #%llu на %s", 
                  event.id, dt.toString().c_str());
    
        
#ifdef USE_FREERTOS
        // Освобождаем мьютекс
        if (eventsMutex != nullptr)
        {
            xSemaphoreGive(eventsMutex);
        }
#endif

        return event.id;
    }
    
    // Добавление периодического события с началом через указанное количество секунд
    std::uint64_t Scheduler::schedulePeriodicAfter(uint32_t afterSeconds, uint32_t periodSeconds, std::function<void()> callback)
    {
        if (!initialized)
        {
            logger->log(farm::log::Level::Error,
                     "[Scheduler] Попытка добавить событие до инициализации планировщика");
            return 0;
        }
        
        if (!isNtpOnline()) return 0;
        
        uint32_t targetTime = NTP.getUnix() + afterSeconds;
        return schedulePeriodicAt(targetTime, periodSeconds, callback);
    }
    
    // Добавление периодического события с началом в конкретное время (Datime)
    std::uint64_t Scheduler::schedulePeriodicAt(const Datime& dateTime, uint32_t periodSeconds, std::function<void()> callback)
    {
        if (!initialized)
        {
            logger->log(farm::log::Level::Error,
                     "[Scheduler] Попытка добавить событие до инициализации планировщика");
            return 0;
        }
        
        return schedulePeriodicAt(dateTime.getUnix(), periodSeconds, callback);
    }
    
    // Добавление периодического события с началом в конкретное время (unix)
    std::uint64_t Scheduler::schedulePeriodicAt(uint32_t unixTime, uint32_t periodSeconds, std::function<void()> callback)
    {
        if (!initialized)
        {
            logger->log(farm::log::Level::Error,
                     "[Scheduler] Попытка добавить событие до инициализации планировщика");
            return 0;
        }
        
        if (!isNtpOnline()) return 0;
        
        if (!callback) 
        {
            logger->log(farm::log::Level::Error, 
                    "[Scheduler] Попытка добавить событие с пустым обработчиком");
            return 0;
        }
        
        if (periodSeconds == 0) 
        {
            logger->log(farm::log::Level::Error, 
                      "[Scheduler] Период не может быть равен нулю");
            return 0;
        }
        
        uint32_t currentTime = NTP.getUnix();
        
        // Если запланированное время уже прошло, переносим первый запуск на завтрашний день
        if (unixTime < currentTime) 
        {
            Datime unixTimeDatime(unixTime);
            logger->log(farm::log::Level::Info, 
                       "[Scheduler] Запланированное время периодического события %u уже прошло, переносим первый запуск на завтра", unixTimeDatime.toString());
            
            // Извлекаем компоненты времени из unixTime (нам нужны только часы, минуты и секунды)
            Datime origTime(unixTime);
            
            // Берем текущее время и создаем дату на завтра
            Datime currentDate(currentTime);
            
            // Создаем временную точку для завтрашнего дня
            time_t tomorrow_time = currentTime + 24*60*60; // текущее время + 24 часа
            Datime tomorrow(tomorrow_time);
            
            // Устанавливаем время суток из оригинальной даты
            tomorrow.hour = origTime.hour;
            tomorrow.minute = origTime.minute;
            tomorrow.second = origTime.second;
            
            // Получаем новое unix-время
            unixTime = tomorrow.getUnix();
            
            logger->log(farm::log::Level::Info, 
                       "[Scheduler] Новое время первого запуска: %u", unixTime);
        }
        
        // Создаем новое периодическое событие
        ScheduledEvent event;
        event.type = ScheduleType::PERIODIC;
        event.scheduledTime = unixTime;
        event.periodSeconds = periodSeconds;
        event.callback = callback;
        event.id = nextEventId++;

#ifdef USE_FREERTOS
        // Блокируем доступ к events с помощью мьютекса
        if (eventsMutex != nullptr && xSemaphoreTake(eventsMutex, portMAX_DELAY) != pdTRUE)
        {
            logger->log(farm::log::Level::Error, 
                      "[Scheduler] Не удалось получить мьютекс для добавления периодического события");
            return 0;
        }
#endif

        events.push_back(event);
        
        Datime dt(unixTime);
        logger->log(farm::log::Level::Debug, 
                  "[Scheduler] Добавлено периодическое событие #%llu с началом %s и периодом %d сек", 
                  event.id, dt.toString().c_str(), periodSeconds);
    
        
#ifdef USE_FREERTOS
        // Освобождаем мьютекс
        if (eventsMutex != nullptr)
        {
            xSemaphoreGive(eventsMutex);
        }
#endif

        return event.id;
    }
    
    // Удаление запланированного события по ID
    bool Scheduler::removeScheduledEvent(std::uint64_t eventId)
    {
        if (!initialized)
        {
            logger->log(farm::log::Level::Error,
                     "[Scheduler] Попытка удалить событие до инициализации планировщика");
            return false;
        }

        if (!isNtpOnline()) return false;
        
#ifdef USE_FREERTOS
        // Блокируем доступ к events с помощью мьютекса
        if (eventsMutex != nullptr && xSemaphoreTake(eventsMutex, portMAX_DELAY) != pdTRUE)
        {
            logger->log(farm::log::Level::Error, 
                      "[Scheduler] Не удалось получить мьютекс для удаления события");
            return false;
        }
#endif
        
        auto it = std::find_if(events.begin(), events.end(), 
                            [eventId](const ScheduledEvent& e) { return e.id == eventId; });
        
        bool result = false;
        
        if (it != events.end()) 
        {
            logger->log(farm::log::Level::Debug, 
                      "[Scheduler] Удалено событие с ID %llu", eventId);
            events.erase(it);
            result = true;
        }
        else 
        {
            logger->log(farm::log::Level::Warning, 
                      "[Scheduler] Событие с ID %llu не найдено", eventId);
            result = false;
        }
        
#ifdef USE_FREERTOS
        // Освобождаем мьютекс
        if (eventsMutex != nullptr)
        {
            xSemaphoreGive(eventsMutex);
        }
#endif
        
        return result;
    }
    
    // Удаление всех событий
    void Scheduler::clearAllEvents()
    {
        if (!isNtpOnline()) return;

        if (!initialized)
        {
            logger->log(farm::log::Level::Error,
                     "[Scheduler] Попытка очистить события до инициализации планировщика");
            return;
        }
        
#ifdef USE_FREERTOS
        // Блокируем доступ к events с помощью мьютекса
        if (eventsMutex != nullptr && xSemaphoreTake(eventsMutex, portMAX_DELAY) != pdTRUE)
        {
            logger->log(farm::log::Level::Error, 
                      "[Scheduler] Не удалось получить мьютекс для очистки событий");
            return;
        }
#endif
        
        events.clear();
        logger->log(farm::log::Level::Info, 
                  "[Scheduler] Все события удалены");
                  
#ifdef USE_FREERTOS
        // Освобождаем мьютекс
        if (eventsMutex != nullptr)
        {
            xSemaphoreGive(eventsMutex);
        }
#endif
    }
    
    // Проверка совпадения времени
    bool Scheduler::isTimeMatch(uint32_t currentTime, uint32_t scheduledTime) const
    {
        return currentTime >= scheduledTime;
    }
    
    // Проверка, выполнялось ли событие сегодня
    bool Scheduler::wasTodayExecuted(const ScheduledEvent& event, uint32_t currentTime) const
    {
        if (event.lastExecutionTime == 0) 
        {
            return false;
        }
        
        // Получаем начало текущего дня
        Datime current(currentTime);
        Datime startOfDay(current.year, current.month, current.day, 0, 0, 0);
        
        // Если последнее выполнение было сегодня
        return event.lastExecutionTime >= startOfDay.getUnix();
    }
    
    // Проверка и выполнение запланированных событий
    void Scheduler::checkSchedule()
    {
        if (!initialized) 
        {
            logger->log(farm::log::Level::Error,
                     "[Scheduler] Попытка проверить расписание до инициализации планировщика");
            return;
        }
        
        if (!isNtpOnline()) return;
        
        uint32_t currentTime = NTP.getUnix();
        
#ifdef USE_FREERTOS
        // Блокируем доступ к events с помощью мьютекса
        if (eventsMutex != nullptr && xSemaphoreTake(eventsMutex, portMAX_DELAY) != pdTRUE)
        {
            logger->log(farm::log::Level::Error, 
                      "[Scheduler] Не удалось получить мьютекс для проверки расписания");
            return;
        }
#endif
        
        // Создаем временный список для хранения событий, которые нужно удалить
        std::vector<size_t> eventsToRemove;
        
        // Обрабатываем все события
        for (size_t i = 0; i < events.size(); i++) 
        {
            auto& event = events[i];
            
            switch (event.type) 
            {
                case ScheduleType::ONCE:
                    // Проверяем одноразовые события
                    if (isTimeMatch(currentTime, event.scheduledTime) && !event.executed) 
                    {
                        logger->log(farm::log::Level::Debug, 
                                  "[Scheduler] Выполнение одноразового события #%llu", event.id);
                        
                        // Сохраняем копию callback перед освобождением мьютекса
                        // ситуация: callback может быть удален из вектора событий из основного кода
                        // во время выполнения callback
                        auto callback = event.callback;
                        
                        // Помечаем задачу как выполненную
                        event.executed = true;
                        event.lastExecutionTime = currentTime;
                        
                        // Добавляем индекс события в список для удаления
                        eventsToRemove.push_back(i);
                        
#ifdef USE_FREERTOS
                        // Освобождаем мьютекс перед вызовом callback
                        if (eventsMutex != nullptr)
                        {
                            xSemaphoreGive(eventsMutex);
                        }
#endif
                        
                        // Выполняем callback
                        callback();
                        
#ifdef USE_FREERTOS
                        // Снова получаем мьютекс после вызова callback
                        if (eventsMutex != nullptr && xSemaphoreTake(eventsMutex, portMAX_DELAY) != pdTRUE)
                        {
                            logger->log(farm::log::Level::Error, 
                                      "[Scheduler] Не удалось получить мьютекс после выполнения события");
                            return;
                        }
#endif
                    }
                    break;
                    
                case ScheduleType::PERIODIC:
                    // Проверяем периодические события
                    if (isTimeMatch(currentTime, event.scheduledTime)) 
                    {
                        // Если событие ещё не выполнялось или прошло достаточно времени с последнего выполнения
                        if (event.lastExecutionTime == 0 || 
                            (currentTime - event.lastExecutionTime) >= event.periodSeconds) 
                        {
                            logger->log(farm::log::Level::Debug, 
                                      "[Scheduler] Выполнение периодического события #%llu", event.id);
                            
                            // Сохраняем копию callback перед освобождением мьютекса
                            auto callback = event.callback;
                            
                            // Обновляем время последнего выполнения
                            event.lastExecutionTime = currentTime;
                            
#ifdef USE_FREERTOS
                            // Освобождаем мьютекс перед вызовом callback
                            if (eventsMutex != nullptr)
                            {
                                xSemaphoreGive(eventsMutex);
                            }
#endif
                            
                            // Выполняем callback
                            callback();
                            
#ifdef USE_FREERTOS
                            // Снова получаем мьютекс после вызова callback
                            if (eventsMutex != nullptr && xSemaphoreTake(eventsMutex, portMAX_DELAY) != pdTRUE)
                            {
                                logger->log(farm::log::Level::Error, 
                                          "[Scheduler] Не удалось получить мьютекс после выполнения события");
                                return;
                            }
#endif

                        }
                    }
                    break;

                default:
                    logger->log(farm::log::Level::Error,
                              "[Scheduler] Неизвестный тип события: %d", static_cast<int>(event.type));
                    break;
            }
        }        
        
        // Удаляем выполненные одноразовые события
        events.erase(
            std::remove_if(events.begin(), events.end(),
                       [](const ScheduledEvent& e) { 
                           return e.type == ScheduleType::ONCE && e.executed; 
                       }),
            events.end()
        );
        
#ifdef USE_FREERTOS
        // Освобождаем мьютекс
        if (eventsMutex != nullptr)
        {
            xSemaphoreGive(eventsMutex);
        }
#endif
    }

    size_t Scheduler::getEventCount() const
    {
        return events.size();
    }

#ifdef USE_FREERTOS
    // Функция задачи FreeRTOS
    void Scheduler::schedulerTaskFunction(void *parameters)
    {
        Scheduler* scheduler = static_cast<Scheduler*>(parameters);
        
        while (!scheduler->taskShouldExit)
        {
            // Проверяем и выполняем запланированные события
            scheduler->checkSchedule();
            
            // Ждем следующего интервала проверки
            vTaskDelay(scheduler->checkInterval);
        }
        
        scheduler->logger->log(farm::log::Level::Farm, 
                           "[Scheduler] Задача планировщика остановлена");
        
        // Удаляем задачу
        vTaskDelete(NULL);
    }
    
    // Запустить задачу планировщика
    bool Scheduler::startSchedulerTask(uint8_t priority, uint32_t stackSize)
    {
        if (!initialized)
        {
            logger->log(farm::log::Level::Error,
                     "[Scheduler] Попытка запустить задачу планировщика до инициализации");
            return false;
        }
        
        // Проверяем, запущена ли уже задача
        if (isSchedulerTaskRunning())
        {
            logger->log(farm::log::Level::Warning, 
                      "[Scheduler] Задача планировщика уже запущена");
            return false;
        }
        
        // Сбрасываем флаг выхода
        taskShouldExit = false;
        
        // Создаем задачу
        BaseType_t result = xTaskCreate(
            schedulerTaskFunction,   // Функция задачи
            "SchedulerTask",         // Имя задачи
            stackSize,               // Размер стека
            this,                    // Параметр (указатель на экземпляр класса)
            priority,                // Приоритет
            &schedulerTaskHandle     // Хэндл задачи
        );
        
        if (result != pdPASS)
        {
            logger->log(farm::log::Level::Error, 
                      "[Scheduler] Не удалось создать задачу планировщика");
            return false;
        }
        
        logger->log(farm::log::Level::Farm, 
                  "[Scheduler] Задача планировщика запущена с приоритетом %d", priority);
        
        return true;
    }
    
    // Остановить задачу планировщика
    bool Scheduler::stopSchedulerTask()
    {
        if (!initialized)
        {
            logger->log(farm::log::Level::Error,
                     "[Scheduler] Попытка остановить задачу планировщика до инициализации");
            return false;
        }
        
        // Проверяем, запущена ли задача
        if (!isSchedulerTaskRunning())
        {
            logger->log(farm::log::Level::Warning, 
                      "[Scheduler] Задача планировщика не запущена");
            return false;
        }
        
        // Устанавливаем флаг выхода
        taskShouldExit = true;
        
        // Ждем завершения задачи (с таймаутом)
        for (int i = 0; i < MAX_STOP_ATTEMPTS; i++)
        {
            if (!isSchedulerTaskRunning())
            {
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(TASK_STOP_CHECK_INTERVAL_MS));
        }
        
        // Если задача все еще запущена, удаляем ее принудительно
        if (isSchedulerTaskRunning())
        {
            logger->log(farm::log::Level::Warning, 
                      "[Scheduler] Задача планировщика не остановилась самостоятельно, удаляем принудительно");
            vTaskDelete(schedulerTaskHandle);
        }
        
        schedulerTaskHandle = nullptr;
        
        logger->log(farm::log::Level::Farm, 
                  "[Scheduler] Задача планировщика остановлена");
        
        return true;
    }
    
    // Проверить, запущена ли задача
    bool Scheduler::isSchedulerTaskRunning() const
    {
        if (!initialized)
        {
            return false;
        }
        
        if (schedulerTaskHandle == nullptr)
        {
            return false;
        }
        
        // Проверяем статус задачи
        eTaskState state = eTaskGetState(schedulerTaskHandle);
        
        return (state != eDeleted && state != eInvalid);
    }

    // Установить интервал проверки расписания
    void Scheduler::setCheckInterval(uint32_t intervalMs)
    {
        if (!initialized)
        {
            logger->log(farm::log::Level::Error,
                     "[Scheduler] Попытка установить интервал до инициализации планировщика");
            return;
        }
        
        // Преобразуем миллисекунды в тики
        checkInterval = pdMS_TO_TICKS(intervalMs);
        
        logger->log(farm::log::Level::Debug, 
                  "[Scheduler] Установлен интервал проверки расписания: %d мс", intervalMs);
    }
#endif
} 