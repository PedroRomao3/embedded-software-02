#include <unity.h>

#include "comm/communicator.hpp"
#include "model/systemDiagnostics.hpp"

#define BAMOCAR_VDC_HIGH 0x11
#define BAMOCAR_VDC_LOW 0x01
#define RES_READY_TRUE 0x01
#define RADIO_QUALITY_0 0x00
#define RADIO_QUALITY_100 0x64
#define RES_EMG_0 0x00
#define RES_EMG_3 0x00
#define RES_OK_0 0x01
#define RES_OK_3 0x80

SystemData sd;
Communicator communicator(&sd);
CAN_message_t msg;

/**
 * @brief Test if the message wheel rpm message is
 * created correctly for a 0.0 rpm value
 *
 */
void wss_message_zero_rpm(void) {
  std::array<uint8_t, 5> msg;
  create_left_wheel_msg(msg, 0.0);
  TEST_ASSERT_EQUAL_HEX8(0x00, msg[1]);
  TEST_ASSERT_EQUAL_HEX8(0x00, msg[2]);
  TEST_ASSERT_EQUAL_HEX8(0x00, msg[3]);
  TEST_ASSERT_EQUAL_HEX8(0x00, msg[4]);
}

/**
 * @brief Test if the message wheel rpm message is
 * created correctly for a large rpm value
 *
 */
void wss_message_high_rpm(void) {
  std::array<uint8_t, 5> msg;
  create_left_wheel_msg(msg, 1002.1231213);
  TEST_ASSERT_EQUAL_HEX8(0x74, msg[1]);
  TEST_ASSERT_EQUAL_HEX8(0x87, msg[2]);
  TEST_ASSERT_EQUAL_HEX8(0x01, msg[3]);
  TEST_ASSERT_EQUAL_HEX8(0x00, msg[4]);
}

/**
 * @brief Test if the message wheel rpm message is
 * created correctly for a negative rpm value
 *
 */
void wss_message_negative_rpm(void) {
  std::array<uint8_t, 5> msg;
  create_left_wheel_msg(msg, -10.324);
  TEST_ASSERT_EQUAL_HEX8(0x00, msg[1]);
  TEST_ASSERT_EQUAL_HEX8(0x00, msg[2]);
  TEST_ASSERT_EQUAL_HEX8(0x00, msg[3]);
  TEST_ASSERT_EQUAL_HEX8(0x00, msg[4]);
}

/**
 * @brief Test if the message wheel rpm message is
 * created correctly for a small rpm value
 *
 */
void wss_message_small_rpm(void) {
  std::array<uint8_t, 5> msg;
  create_left_wheel_msg(msg, 0.324235235);
  TEST_ASSERT_EQUAL_HEX8(0x20, msg[1]);
  TEST_ASSERT_EQUAL_HEX8(0x00, msg[2]);
  TEST_ASSERT_EQUAL_HEX8(0x00, msg[3]);
  TEST_ASSERT_EQUAL_HEX8(0x00, msg[4]);
}

/**
 * @brief Tests if the messages from C1 are correctly
 * parsed for Hydraulic Brake Pressure and Right Wheel Rpm's
 */
void test_c1(void) {
  msg.id = C1_ID;
  msg.len = 5;
  msg.buf[0] = HYDRAULIC_LINE;
  msg.buf[1] = 0x01;
  msg.buf[2] = 0x01;
  msg.buf[3] = 0x00;
  msg.buf[4] = 0x00;
  communicator.parse_message(msg);
  TEST_ASSERT_EQUAL(257, sd.hardware_data_._hydraulic_line_pressure);

  msg.buf[0] = RIGHT_WHEEL_CODE;
  msg.buf[1] = 0x00;
  msg.buf[2] = 0x08;
  communicator.parse_message(msg);

  if (sd.hardware_data_._right_wheel_rpm == 20.48)
    TEST_PASS();
  else
    TEST_FAIL();
}

/**
 * @brief Tests if the messages from Bamocar are correctly
 * parsed and the Tractive System State is matching received info
 */
void test_bamocar(void) {
  sd = SystemData();  // reset ready timestamp
  sd.failure_detection_.ts_on_ = true;

  msg.id = BAMO_RESPONSE_ID;
  msg.len = 4;
  msg.buf[0] = BTB_READY;
  msg.buf[1] = 0x00;
  msg.buf[2] = 0x00;
  msg.buf[3] = 0x00;
  communicator.parse_message(msg);
  TEST_ASSERT_EQUAL(false, sd.failure_detection_.ts_on_);

  msg.buf[1] = RES_READY_TRUE;
  communicator.parse_message(msg);
  TEST_ASSERT_EQUAL(false, sd.failure_detection_.ts_on_);

  msg.buf[0] = VDC_BUS;
  msg.buf[2] = BAMOCAR_VDC_HIGH;
  communicator.parse_message(msg);
  TEST_ASSERT_EQUAL(true, sd.failure_detection_.ts_on_);

  msg.buf[2] = BAMOCAR_VDC_LOW;
  communicator.parse_message(msg);
  TEST_ASSERT_EQUAL(false, sd.failure_detection_.ts_on_);
}

/**
 * @brief Tests if the messages from RES are correctly
 * parsed for R2D and Emergency activations, aswell as
 * for the radio quality indicator
 */
void test_res_state(void) {
  msg.id = RES_STATE;
  msg.len = 8;
  msg.buf[0] = 0x02;  // Assuming emergency signals are set
  msg.buf[1] = 0x00;
  msg.buf[2] = 0x00;
  msg.buf[3] = 0x80;
  msg.buf[4] = 0x00;
  msg.buf[5] = 0x00;
  msg.buf[6] = RADIO_QUALITY_0;  // radio quality
  msg.buf[7] = 0x00;
  communicator.parse_message(msg);
  TEST_ASSERT_EQUAL(false, sd.r2d_logics_.r2d);
  TEST_ASSERT_EQUAL(0, sd.failure_detection_.radio_quality);

  // wait ready timeout
  Metro time{READY_TIMEOUT_MS};
  while (!time.check()) {
  }

  msg.buf[6] = RADIO_QUALITY_100;  // def radio quality 100
  communicator.parse_message(msg);
  TEST_ASSERT_EQUAL(true, sd.r2d_logics_.r2d);
  TEST_ASSERT_EQUAL(100, sd.failure_detection_.radio_quality);

  // RES ok
  msg.buf[0] = RES_OK_0;
  msg.buf[3] = RES_OK_3;  // def emg bits
  communicator.parse_message(msg);
  TEST_ASSERT_EQUAL(false, sd.failure_detection_.emergencySignal);

  // RES Emergency
  msg.buf[0] = RES_EMG_0;
  msg.buf[3] = RES_EMG_3;  // def emg bits
  communicator.parse_message(msg);
  TEST_ASSERT_EQUAL(true, sd.failure_detection_.emergencySignal);
}

void setUp() {}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(wss_message_zero_rpm);
  RUN_TEST(wss_message_high_rpm);
  RUN_TEST(wss_message_negative_rpm);
  RUN_TEST(wss_message_small_rpm);
  RUN_TEST(test_c1);
  RUN_TEST(test_bamocar);
  RUN_TEST(test_res_state);
  return UNITY_END();
}