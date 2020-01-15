#ifndef CMD_HANDLER_H
#define CMD_HANDLER_H

#define CMD_MOTOR_START_ID      ((uint16_t) 0xC001)
#define CMD_MOTOR_STOP_ID       ((uint16_t) 0xC002)
#define CMD_MOTOR_SET_POWER_ID  ((uint16_t) 0xC003)
#define CMD_MONITORING_START_ID ((uint16_t) 0xC004)
#define CMD_MONITORING_STOP_ID  ((uint16_t) 0xC005)
#define CMD_TEST_ID             ((uint16_t) 0xC0FF)

egl_ptc_t* spi_llp(void);

#endif