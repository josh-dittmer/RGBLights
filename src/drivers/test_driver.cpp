#include "test_driver.h"

bool TestDriver::init() {
    get_logger().verbose("Test driver successfully initialized!");
    return true;
}

void TestDriver::write(uint8_t r, uint8_t g, uint8_t b) {}

void TestDriver::shutdown() {
    get_logger().verbose("Test driver successfully shutdown!");
}