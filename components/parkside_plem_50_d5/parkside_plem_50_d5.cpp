#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/log.h"
#include "parkside_plem_50_d5.h"
#include <cstring>

namespace esphome {
namespace parkside_plem_50_d5 {

static const char *TAG = "parkside_plem_50_d5";

static const int BUFFER_SIZE = 100;

void ParksidePlem50D5Component::setup() {
  // nothing to do here
}

void ParksidePlem50D5Component::read_message(char buffer[])
{
  uint8_t buffer_index = 0;

  memset (buffer, 0, BUFFER_SIZE);

  for (int i = 0; buffer_index < BUFFER_SIZE; i++) {
    if (i > 10000)
    {
      ESP_LOGE(TAG, "Timeout, message not complete. Received: %s", (char *) buffer);
      break;
    }
    if (!available())
    {
      delay(1);
      continue;
    }
    uint8_t data;
    read_byte(&data);
    buffer[buffer_index++] = (char) data;
    if ('&' == data)
    {
      break;
    }
  }
  // terminate the string
  buffer[buffer_index] = 0;
  ESP_LOGD(TAG, "Received: %s", (char *) buffer);
}

void ParksidePlem50D5Component::write_message(const char * message)
{
  ESP_LOGD(TAG, "Sending: %s", message);
  write_str(message);
}

void ParksidePlem50D5Component::wait_for(const char * waitForString)
{
  char buffer[BUFFER_SIZE];

  this->read_message(buffer);

  if (strcmp (waitForString, buffer))
  {
    ESP_LOGW(TAG, "Didn't receive expected value. Expected: '%s' Received: '%s'", waitForString, buffer);
  }
}

void ParksidePlem50D5Component::update() {
  char buffer[BUFFER_SIZE];

  int i = 0;
  do
  {
    // init laser
    this->write_message("$0003260130&"); // light on

    this->wait_for("$0003260130&"); // ack?
    this->read_message(buffer); // returns something

    // do measurement
    this->write_message("$00022123&");
    this->wait_for("$0003260130&"); // ack?
    this->read_message(buffer); // measured value
  } while (this->process_measurement(buffer) && ++i < this->attempt_count_);

}

void ParksidePlem50D5Component::process_error (const char * buffer, const char * errorText)
{
  char err_msg[100] = "";
  sprintf(err_msg, "%s, buffer: '%s'", errorText, buffer);
  ESP_LOGE(TAG, "%s", err_msg);
  this->error_sensor_->publish_state(err_msg);
}

int ParksidePlem50D5Component::process_measurement (const char * measurement)
{
  // measurement looks like $0006210000058416&
  // where 000621000 is something (doesn't matter)
  // 00584 is distance from sensor head in mm
  // 16 is BCD checksum
  if (!strncmp ("<0E", measurement, 3)) // TODO
  {
    this->process_error (measurement, "Error received");
    return -1;
  }
  if (strncmp ("$", measurement, 1))
  {
    this->process_error (measurement, "Response should start with '$'");
    return -2;
  }

  // get value len
  int valueLen = strlen (measurement);
  if ( 18 != valueLen )
  {
    this->process_error (measurement, "Measurement length is not 18");
    return -3;
  }

  int val = atoi(measurement + 9) / 100; // /100 because of BCD - cut it off
  if (val <= 0)
  {
    this->process_error (measurement, "Cannot get meaningfull value");
    return -5;
  }
  ESP_LOGD(TAG, "Buffer: '%s', value is %d", measurement, val);
  this->distance_sensor_->publish_state(val);
  this->error_sensor_->publish_state("");

  return 0;
}

void ParksidePlem50D5Component::dump_config() {
  ESP_LOGCONFIG(TAG, "ParksidePlem50D5 Component:");
  ESP_LOGCONFIG(TAG, "  attempt_count: %d", this->attempt_count_);
}

} // parkside_plem_50_d5
} // esphome
