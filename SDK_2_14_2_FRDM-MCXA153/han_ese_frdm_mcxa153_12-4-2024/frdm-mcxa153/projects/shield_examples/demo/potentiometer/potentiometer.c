#include "potentiometer.h"
#include "helper_functions/adc0_interrupt.h"
// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------
uint16_t adc_result = 0;

// -----------------------------------------------------------------------------
// Local function implementation
// -----------------------------------------------------------------------------

uint16_t map(const uint16_t x, const uint16_t in_min, const uint16_t in_max, const uint16_t out_min, const uint16_t out_max) 
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int potentiometer_main(void)
{
  adc0_start_channel_08();

  if (adc0_read_channel_08(&adc_result))
  {
    uint16_t save_adc_result = adc_result;
    // Start a new conversion
    adc0_start_channel_08();
    return save_adc_result;
  }
  else
  {
    return 0;
  }
}