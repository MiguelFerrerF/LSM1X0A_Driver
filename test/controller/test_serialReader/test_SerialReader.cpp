#include "SerialDriver.h"
#include <Arduino.h>
#include <unity.h>

#define TEST_RX_PIN 14
#define TEST_TX_PIN 33
#define TEST_BAUD_RATE 9600

// Mock callback function to test the OnReceiveCallback functionality
void mockCallback(const uint8_t *message, size_t length) {
  static char receivedMessage[256];
  strncpy(receivedMessage, (const char *)message, length);
  receivedMessage[length] = '\0';
  // The first message is expected to be a empty message (due to initialization)
  TEST_ASSERT_EQUAL_STRING("\r\n", receivedMessage);
}

// Test initialization of SerialDriver
void test_SerialDriver_init() {
  HardwareSerial mockSerial(2); // Use Serial2 for testing
  SerialDriver reader(mockSerial);

  bool result =
      reader.init(TEST_BAUD_RATE, TEST_RX_PIN, TEST_TX_PIN, mockCallback);
  TEST_ASSERT_TRUE(result);
}

// Test writing data using SerialDriver
void test_SerialDriver_write() {
  HardwareSerial mockSerial(2); // Use Serial1 for testing
  SerialDriver reader(mockSerial);

  uint8_t testData[] = "Test Data";
  size_t bytesWritten = reader.write(testData, sizeof(testData) - 1);

  TEST_ASSERT_EQUAL(sizeof(testData) - 1, bytesWritten);
}

// Test stopping the SerialDriver
void test_SerialDriver_stop() {
  HardwareSerial mockSerial(2); // Use Serial2 for testing
  SerialDriver reader(mockSerial);

  if (!reader.init(TEST_BAUD_RATE, TEST_RX_PIN, TEST_TX_PIN, mockCallback)) {
    TEST_FAIL_MESSAGE("Failed to initialize SerialDriver");
  }

  reader.stop();

  // Check if the task handle is null after stopping
  TEST_ASSERT_FALSE(reader.getTaskHandle());
}

// Test the read loop functionality
void test_SerialDriver_readLoop() {
  HardwareSerial mockSerial(2); // Use Serial2 for testing
  SerialDriver reader(mockSerial);

  reader.init(TEST_BAUD_RATE, TEST_RX_PIN, TEST_TX_PIN, mockCallback);

  // Simulate incoming data
  const char *testData = "ATZ\r\n";
  mockSerial.write((const uint8_t *)testData, strlen(testData));

  // Allow some time for the read loop to process the data
  delay(500);

  // The mock callback will validate the received message
}

// Setup and teardown functions
void setup() {
  UNITY_BEGIN();

  RUN_TEST(test_SerialDriver_init);
  RUN_TEST(test_SerialDriver_write);
  RUN_TEST(test_SerialDriver_stop);
  RUN_TEST(test_SerialDriver_readLoop);

  UNITY_END();
}

void loop() {
  // Empty loop
}