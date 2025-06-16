#include "pwm_sunset_driver.h"

#include <pigpio.h>

const unsigned int PWMSunsetDriver::PWM_PIN_R = 17;
const unsigned int PWMSunsetDriver::PWM_PIN_G = 27;
const unsigned int PWMSunsetDriver::PWM_PIN_B = 22;
const unsigned int PWMSunsetDriver::PWM_PIN_W = 23;

bool PWMSunsetDriver::init() {
    if (gpioInitialise() < 0) {
        get_logger().error("Failed to initialize GPIO!");
        return false;
    }

    bool error = false;

    error = error || (gpioPWM(PWM_PIN_R, 0) != 0);
    error = error || (gpioPWM(PWM_PIN_G, 0) != 0);
    error = error || (gpioPWM(PWM_PIN_B, 0) != 0);
    error = error || (gpioPWM(PWM_PIN_W, 0) != 0);

    if (error) {
        get_logger().error("Failed to enable PWM on pins!");
        gpioTerminate();
        return false;
    }

    get_logger().verbose("GPIO successfully initialized!");

    m_init = true;

    return true;
}
void PWMSunsetDriver::write(uint8_t r, uint8_t g, uint8_t b) {
    if (!m_init) {
        get_logger().error("Cannot write, PWM not initialized!");
        return;
    }

    uint8_t w = (r + g + b) / 3;

    gpioPWM(PWM_PIN_R, r);
    gpioPWM(PWM_PIN_G, g);
    gpioPWM(PWM_PIN_B, b);
    gpioPWM(PWM_PIN_W, w);
}
void PWMSunsetDriver::shutdown() {
    if (!m_init) {
        get_logger().error("Cannot shutdown, PWM not initialized!");
        return;
    }

    gpioTerminate();

    get_logger().verbose("GPIO successfully shutdown!");
}