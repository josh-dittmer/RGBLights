#include "pwm_strip_driver.h"

#include <pigpio.h>

const unsigned int PWMStripDriver::PWM_PIN_R = 27;
const unsigned int PWMStripDriver::PWM_PIN_G = 22;
const unsigned int PWMStripDriver::PWM_PIN_B = 23;

bool PWMStripDriver::init() {
    if (gpioInitialise() < 0) {
        get_logger().error("Failed to initialize GPIO!");
        return false;
    }

    // static const int FREQUENCY = 1000;
    // static const int DUTY_CYCLE = 255;

    bool error = false;

    error = error || (gpioPWM(PWM_PIN_R, 0) != 0);
    error = error || (gpioPWM(PWM_PIN_G, 0) != 0);
    error = error || (gpioPWM(PWM_PIN_B, 0) != 0);

    if (error) {
        get_logger().error("Failed to enable PWM on pins!");
        gpioTerminate();
        return false;
    }

    get_logger().verbose("GPIO successfully initialized!");

    m_init = true;

    return true;
}
void PWMStripDriver::write(uint8_t r, uint8_t g, uint8_t b) {
    if (!m_init) {
        get_logger().error("Cannot write, PWM not initialized!");
        return;
    }

    gpioPWM(PWM_PIN_R, r);
    gpioPWM(PWM_PIN_G, g);
    gpioPWM(PWM_PIN_B, b);
}
void PWMStripDriver::shutdown() {
    if (!m_init) {
        get_logger().error("Cannot shutdown, PWM not initialized!");
        return;
    }

    gpioTerminate();

    get_logger().verbose("GPIO successfully shutdown!");
}