#include "config.h"

#include <LittleFS.h>

#include "../sensor/sensor_BME280.h"
#include "../finiteStateMachine/finiteStateMachine.h"

struct StoredConfig
{
    uint32_t magic;
    uint32_t measureIntervalMinutes;
    uint32_t ventilationIntervalMinutes;
    uint32_t waitIntervalMinutes;
    int16_t dewPointThreshold;
    int16_t minInsideTemperatureCutoff;
    int16_t minOutsideTemperatureCutoff;
    uint16_t crc;
};

static constexpr uint32_t CONFIG_MAGIC = 0xC0F1AEE7;
static constexpr const char *CONFIG_FILE_PATH = "/config.bin";

static bool littleFsReady = false;

static bool ensureFileSystem()
{
    if (littleFsReady)
    {
        return true;
    }

    if (!LittleFS.begin())
    {
        Serial.println("CONFIG: LittleFS mount failed");
        return false;
    }

    littleFsReady = true;
    return true;
}

static uint16_t calculateCrc(const StoredConfig &config)
{
    const uint8_t *data = reinterpret_cast<const uint8_t *>(&config);
    uint16_t crc = 0;
    for (size_t i = 0; i < offsetof(StoredConfig, crc); ++i)
    {
        crc = crc ^ data[i];
        crc = (crc << 1) | (crc >> 15);
    }
    return crc;
}

void loadConfig()
{
    if (!ensureFileSystem())
    {
        Serial.println("CONFIG: file system unavailable, using defaults");
        return;
    }

    if (!LittleFS.exists(CONFIG_FILE_PATH))
    {
        Serial.println("CONFIG: no saved settings found, using defaults");
        return;
    }

    File file = LittleFS.open(CONFIG_FILE_PATH, "r");
    if (!file)
    {
        Serial.println("CONFIG: failed to open config file for reading");
        return;
    }

    if (file.size() != sizeof(StoredConfig))
    {
        Serial.println("CONFIG: invalid config file size, ignoring saved settings");
        file.close();
        return;
    }

    StoredConfig stored;
    if (file.read(reinterpret_cast<uint8_t *>(&stored), sizeof(stored)) != sizeof(stored))
    {
        Serial.println("CONFIG: failed to read config file, ignoring saved settings");
        file.close();
        return;
    }
    file.close();

    if (stored.magic != CONFIG_MAGIC)
    {
        Serial.println("CONFIG: magic mismatch, ignoring saved settings");
        return;
    }

    if (stored.crc != calculateCrc(stored))
    {
        Serial.println("CONFIG: checksum mismatch, ignoring saved settings");
        return;
    }

    measureInterval = stored.measureIntervalMinutes * 60UL * 1000UL;
    ventilationInterval = stored.ventilationIntervalMinutes * 60UL * 1000UL;
    waitInterval = stored.waitIntervalMinutes * 60UL * 1000UL;
    dewPointThreshold = stored.dewPointThreshold;
    minInsideTemperatureCutoff = stored.minInsideTemperatureCutoff;
    minOutsideTemperatureCutoff = stored.minOutsideTemperatureCutoff;

    Serial.println("CONFIG: loaded saved settings");
}

void saveConfig()
{
    if (!ensureFileSystem())
    {
        Serial.println("CONFIG: file system unavailable, unable to save settings");
        return;
    }

    StoredConfig stored;
    stored.magic = CONFIG_MAGIC;
    stored.measureIntervalMinutes = measureInterval / 1000UL / 60UL;
    stored.ventilationIntervalMinutes = ventilationInterval / 1000UL / 60UL;
    stored.waitIntervalMinutes = waitInterval / 1000UL / 60UL;
    stored.dewPointThreshold = dewPointThreshold;
    stored.minInsideTemperatureCutoff = minInsideTemperatureCutoff;
    stored.minOutsideTemperatureCutoff = minOutsideTemperatureCutoff;
    stored.crc = calculateCrc(stored);

    File file = LittleFS.open(CONFIG_FILE_PATH, "w");
    if (!file)
    {
        Serial.println("CONFIG: failed to open config file for writing");
        return;
    }

    if (file.write(reinterpret_cast<const uint8_t *>(&stored), sizeof(stored)) != sizeof(stored))
    {
        Serial.println("CONFIG: failed to write config file");
        file.close();
        return;
    }
    file.close();

    Serial.println("CONFIG: saved settings to LittleFS");
}

void initConfig()
{
    loadConfig();
}
