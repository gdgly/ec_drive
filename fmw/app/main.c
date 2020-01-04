#define EGL_MODULE_NAME "MAIN"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "stm32f0xx.h"
#include "egl_lib.h"
#include "ecd_bsp.h"
#include "cmd_handler.h"

#define BLDC_LOAD_MAMPS_PER_DIGIT (12)
#define NUM_OF_APROXIMATIONS      (100)
#define CRC_EXPECTED              (0xD71C)

static int32_t convert_to_mamps(int32_t raw)
{
  return raw * BLDC_LOAD_MAMPS_PER_DIGIT;
}

static void calc_aprox_motor_params(uint16_t pwm)
{
  int32_t load = 0;
  uint32_t speed = 0;
  
  for(int i = 0; i < NUM_OF_APROXIMATIONS; i++)
    {
      egl_delay(ms, 100);

      /* Get current motor load and speed */
      load += egl_bldc_get_load(ecd_bldc_motor());
      speed += egl_bldc_get_speed(ecd_bldc_motor());
    }
  
  load /= NUM_OF_APROXIMATIONS;
  speed /= NUM_OF_APROXIMATIONS;
  
  EGL_TRACE_INFO("PWM: %d, Load: %d mA, Speed: %d rpm\r\n", pwm, convert_to_mamps(load), speed);
}

static void motor_params_print(uint16_t pwm)
{
  int16_t load = 0;
  uint16_t speed = 0;
  
  for(int i = 0; i < NUM_OF_APROXIMATIONS; i++)
    {
      egl_delay(ms, 100);

      /* Get current motor load and speed */
      load = egl_bldc_get_load(ecd_bldc_motor());
      speed = egl_bldc_get_speed(ecd_bldc_motor());

      EGL_TRACE_INFO("PWM: %d, Load: %d mA, Speed: %d rpm\r\n", pwm, convert_to_mamps(load), speed);
    }
}


static void spi(void)
{
  egl_result_t result         = EGL_SUCCESS;
  static uint8_t buff_in[64]  = {0};
  static uint8_t buff_out[64] = {0};
  size_t read_len             = sizeof(buff_in);
  size_t write_len            = sizeof(buff_out);

  result = egl_itf_read(ecd_spi(), buff_in, &read_len);
  if(result != EGL_SUCCESS)
    {
      EGL_TRACE_ERROR("SPI: read fail. Result: %s\r\n", EGL_RESULT());
      return;
    }

  if(read_len > 0)
    {
      EGL_TRACE_INFO("SPI: got %d\r\n", read_len);
      for(int i = 0; i < read_len; i++)
      {
        EGL_TRACE_INFO("0x%02x\r\n", buff_in[i]);
      }

      result = egl_ptc_decode(spi_llp(), buff_in, &read_len, buff_out, &write_len);
      if(result != EGL_SUCCESS && result != EGL_PROCESS)
      {
        EGL_TRACE_INFO("SPI: decode fail. Result: %s\r\n", EGL_RESULT());
        return;
      }

      EGL_TRACE_INFO("SPI: send %d\r\n", write_len);
      
      result = egl_itf_write(ecd_spi(), buff_out, &write_len);
      if(result != EGL_SUCCESS)
      {
        EGL_TRACE_ERROR("SPI: write fail. Result %s\r\n", EGL_RESULT());
        return;
      }

      EGL_TRACE_INFO("SPI: sent %d\r\n", write_len);
    }
}  

void motor_test(uint16_t pwm)
{
  egl_result_t result;

  result = egl_bldc_set_power(ecd_bldc_motor(), pwm);
  if(result != EGL_SUCCESS)
  {
    EGL_TRACE_ERROR("Set motor power - fail. Result: %s\r\n", EGL_RESULT());
    return;
  }

  EGL_TRACE_INFO("Motor power set\r\n");

  result = egl_bldc_start(ecd_bldc_motor());
  if(result != EGL_SUCCESS)
  {
    EGL_TRACE_ERROR("Start motor - fail. Result: %s\r\n", EGL_RESULT());
    return;
  }

  EGL_TRACE_INFO("Motor started\r\n");

  //calc_aprox_motor_params(pwm);
  motor_params_print(pwm);

  result = egl_bldc_stop(ecd_bldc_motor());
  if(result != EGL_SUCCESS)
  {
    EGL_TRACE_ERROR("Stop motor - fail. Result: %s\r\n", EGL_RESULT());
    return;
  }

  EGL_TRACE_INFO("Motor stoped\r\n");
}

void motor_measure_params(egl_bldc_dir_t dir, uint16_t start_pwm, uint16_t stop_pwm, uint16_t pwm_step)
{
  assert(egl_bldc_start(ecd_bldc_motor()) == EGL_SUCCESS);

  egl_bldc_set_dir(ecd_bldc_motor(), dir);

  for(uint16_t pwm = start_pwm; pwm < stop_pwm; pwm += pwm_step)
  {
      assert(egl_bldc_set_power(ecd_bldc_motor(), pwm) == EGL_SUCCESS);
      calc_aprox_motor_params(pwm);
  }

  assert(egl_bldc_stop(ecd_bldc_motor()) == EGL_SUCCESS);
}

void motor_speed_change_test(void)
{
  assert(egl_bldc_start(ecd_bldc_motor()) == EGL_SUCCESS);

  for(uint16_t pwm = 16; pwm < 320; pwm += 16)
  {
      assert(egl_bldc_set_power(ecd_bldc_motor(), pwm) == EGL_SUCCESS);
      egl_delay(ms, 50);
  }

  assert(egl_bldc_stop(ecd_bldc_motor()) == EGL_SUCCESS);  
}

int main(void)
{
  uint8_t test_data[] = {0x01, 0xC0, 0x00, 0x00};
  uint16_t crc = 0;

  ecd_bsp_init();

  egl_itf_open(ecd_dbg_usart());
  egl_trace_init(EGL_TRACE_LEVEL_DEBUG, ms, NULL, 0);
  
  EGL_TRACE_INFO("EC Drive v0.1\r\n");

  egl_crc_init(egl_crc16_xmodem(), 0, 0);
  egl_itf_open(ecd_spi());
  egl_led_off(ecd_led());
  egl_pio_set(ecd_int2_pin(), true);  

  crc = egl_crc16_calc(egl_crc16_xmodem(), test_data, sizeof(test_data));

  EGL_TRACE_INFO("Test CRC16: 0x%04x\r\n", crc);
  

  egl_led_on(ecd_led());
  
  //motor_test(16);  
  
  EGL_TRACE_INFO(" Measure motor params. Direction: Clockwise\r\n");
  motor_measure_params(EGL_BLDC_MOTOR_DIR_CW,  16, 320, 16);
  // egl_delay(ms, 5000);
  // EGL_TRACE_INFO(" Measure motor params. Direction: Contrclockwise\r\n");
  // motor_measure_params(EGL_BLDC_MOTOR_DIR_CCW, 16, 320, 16);

  //motor_speed_change_test();

  while(1)
  {
    //spi();
  }

  return 0;
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  __assert_func((const char *)file, line, NULL, NULL);
}
#endif

void __assert_func(const char * file, int line , const char * func, const char *expr)
{
  /* Disable interrupts */
  __disable_irq();

  /* Stop motor if running */
  egl_bldc_stop(ecd_bldc_motor());
  
  /* Swich to polling mode */
  egl_itf_ioctl(ecd_dbg_usart(), ECD_DBG_UART_WRITE_POLLING_IOCTL, NULL, 0);

  /* Trace fail message */
  EGL_TRACE_FAIL("Critical fail! file: %s, line: %d, func: %s, expr: %s\r\n",
                                                     file, line, func, expr);
  
  while (1)
    {
      /* Do nothing */
    }
}
