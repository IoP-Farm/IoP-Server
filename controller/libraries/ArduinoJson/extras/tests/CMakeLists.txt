# ArduinoJson - https://arduinojson.org
# Copyright © 2014-2025, Benoit BLANCHON
# MIT License

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

link_libraries(ArduinoJson)

# Failing builds should only link with ArduinoJson, not catch
add_subdirectory(FailingBuilds)

add_subdirectory(catch)
link_libraries(catch)

include_directories(Helpers)
add_subdirectory(Cpp17)
add_subdirectory(Cpp20)
add_subdirectory(Deprecated)
add_subdirectory(IntegrationTests)
add_subdirectory(JsonArray)
add_subdirectory(JsonArrayConst)
add_subdirectory(JsonDeserializer)
add_subdirectory(JsonDocument)
add_subdirectory(JsonObject)
add_subdirectory(JsonObjectConst)
add_subdirectory(JsonSerializer)
add_subdirectory(JsonVariant)
add_subdirectory(JsonVariantConst)
add_subdirectory(ResourceManager)
add_subdirectory(Misc)
add_subdirectory(MixedConfiguration)
add_subdirectory(MsgPackDeserializer)
add_subdirectory(MsgPackSerializer)
add_subdirectory(Numbers)
add_subdirectory(TextFormatter)
