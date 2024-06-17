# Assignment - Neopixel

# Goal

To control neopixels with the FlexPWM module.

# Hardware requirements

- FRDM-MCXA153 board
- Three neopixels (SK6812 or compatible)
- Type-C USB cable

# Neopixels

Neopixels are RGB LEDs that are controller by sending 24-bits over is single wire.

```txt
              3V3
               |
         +----------+
 24-bits | neopixel |
-------->|          |
         +----------+
               |
              GND
```

Each colour is controlled by 8-bits. Sending a single bit takes approximately 1us. A pause greater than 50us is an indication to show the transmitted value on the LEDs.

The width of a pulse determines if a logic 0 or 1 is transmitted. The period of a pulse is always the same. This is depicted in the following timing diagram, with example values for the [SK6812](https://cdn-shop.adafruit.com/product-files/1138/SK6812+LED+datasheet+.pdf) neopixel.

```txt
         ___
logic 0 |///|_________|
         T0H    T0L
        300ns  900ns

         ______
logic 1 |//////|______|
          T1H    T1L
         600ns  600ns
```

For a single neopixel, 24 of these pulses must be transmitted. If one pulse takes 1.2 us, 24 pulses will take 28.8 us.
Furthermore, the 24-bit date structure is as follows:

```txt
  |<----Green---->|<-----Red----->|<----Blue---->|
   _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
__|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|________->t
```

Multiple neopixels can be connected in cascade. The first 24-bits that are transmitted will appear on the latest neopixel in the chain.

# Functional requirements

The application uses three neopixels connected to P3_8. Create an LED chaser as follows:

- The LED chaser runs with 10 Hz.
- At startup the first neopixel is red, the others are off.
- After 100 ms, the second neopixel is red, the others are off.
- After another 100 ms, the third neopixel is red, the others are off.
- After another 100 ms, the first neopixel is green, the others are off.
- After another 100 ms, the second neopixel is green, the others are off.
- After another 100 ms, the third neopixel is green, the others are off.
- After another 100 ms, the first neopixel is blue, the others are off.
- After another 100 ms, the second neopixel is blue, the others are off.
- After another 100 ms, the third neopixel is blue, the others are off.
- Pressing SW3 changes the LED chaser direction.

# Architecture

The following architecture shows the connections between microcontroller, MCULink, and neopixels.

```txt
                                                   3V3              3V3              3V3
     +---------+           +------------+           |                |                |
     | MCULink |           | MCXA153VLH |       +--------+       +--------+       +--------+
 USB |         |    SWD    |            |  PWM  | SK2812 |  PWM  | SK2812 |  PWM  | SK2812 |
<--->|         |<--------->|        P3_8|------>|        |------>|        |------>|        |
     +---------+           |            |       +--------+       +--------+       +--------+
                           +------------+           |                |                |
                                                   GND              GND              GND
```

The MCULink is available on the FRDM-MCXA153 board. The neopixels are external components that are connected by using the headers.

Notice from the timing diagram of a single neopixel that the period for a logic 0 and 1 is the same. As only the width of the pulse changes, a PWM signal with one of two duty cycles can be used to control the neopixels. This will require the use of a timer with PWM capabilities and updating the match value when there is a reload interrupt. The FlexPWM module will be used to realize this. The basic idea for implementation is depicted in the following timing diagram.


```txt

VAL1 (?) - - - - - - - + - - - - - - + - - - - - - + - - - - - - +
                      /|            /|            /|            /|
                     / |           / |           / |           / |
                    /  |          /  |          /  |          /  |
                   /   |         /   |         /   |         /   |
                  /    |        /    |        /    |        /    |
                 /     |       /     |       /     |       /     |
                /      |      /      | - - -+-VAL3-|-     /      |
               /       |     /       |     /.      |     /       |
              /        |    /        |    / .      |    /        |
             /         | - + - VAL3 -|-  /  .      | - + - VAL3 -|-
            /          |  /.         |  /   .      |  /.         |
           /           | / .         | /    .      | / .         |
VAL3 (0)  /            |/  .         |/     .      |/  .         |
INIT (0) +---- VAL3 ---+-------------+-------------+-------------+-
VAL2 (0)               .   .         .      .      .   .         .
VAL4 (0)               .   .         .      .      .   .         .
                       .___.         .______.      .___.         .
PWM_n    ______________|///|_________|//////|______|///|_________|
                       .T0H          .  T1H        .T0H          .
                       .             .             .             .
                    Reload        Reload        Reload
                   interrupt     interrupt     interrupt
                      ISR           ISR           ISR
|               |      |             |             |
+---------------+      |             |             |
|T0H match value|<-----+             |             |
|T1H match value|<-------------------+             |
|T0H match value|<---------------------------------+
|      ...      |
+---------------+
|               |
 Array in memory
 storing the T0H
 and T1H values
 for each bit.
```

This interrupt driven approach offloads the CPU. At every FlexPWM reload interrupt, a new VAL3 match value will be read from an array in memory. This means that this array has to be prepared before the initial transmission starts and that it will use RAM memory. In fact, it will use 24 times a 16-bit number (uint16_t) for every neopixel.

It is also important to note that the time between the *reload interrupt* and the fastest match is only 300ns. Running at 48 MHz, this means that only 14.4 instructions can be executed. This is not enough for both context switching en ISR code execution. Running at 96 MHz instead, will allow to execute twice as many instructions which is enough, provided that optimization is enabled.


<mark>TBD</mark> What will be given and what will be the assignment ?!


# API

In order to create the functional requirements, the following API functions are prepared.

```C
/*!
 * \brief Initializes the servo pins
 *
 * Resources:
 * - SERVO_PWM | P3_10 | CTn_MATx
 *
 * Configures CTimern to generate a PWM signal on MATx with a 50 Hz frequency.
 */
void servo_init(void);

/*!
 * \brief Set the servo duty cycle
 *
 * Sets the PWM duty cycle as follows:
 * - 1000 =   0% = 1.0 ms pulse width = servo moves left
 * - 1500 =  50% = 1.5 ms pulse width = servo moves centre
 * - 2000 = 100% = 2.0 ms pulse width = servo moves right
 *
 * \param[in]  value Duty cycle of the PWM signal
 */
void servo_set(int32_t value);
```

# Implementation tips


