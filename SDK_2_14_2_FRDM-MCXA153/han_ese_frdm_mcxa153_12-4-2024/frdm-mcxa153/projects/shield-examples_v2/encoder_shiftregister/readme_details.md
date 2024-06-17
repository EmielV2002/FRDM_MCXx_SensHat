# Critical sections

## Goal

To know how to protect a data object that requires multiple read or write operations.

## Definitions

Before starting the discussion on how to protect a data object, it is good to be familiar with the following definitions:

**Race condition**
> *Anomalous behaviour due to unexpected critical dependence on the relative timing of events.*

**Critical section**
> *A section of code which creates a possible race condition. The code section should only be executed by one process at a time.*

**Atomic**
> *Indivisible access to any data object. For Cortex-M microcontrollers, the native object size is 32-bits. This means accessing data objects that are 32-bits or smaller takes a single clock cycle. This is indivisible. Data objects larger than 32-bits (such as structs), require more than one instruction to access (both reading and writing) and hence takes more then one clock cycle. This access could thus be interrupted.*

**Volatile**
> *Directive for the compiler. Indicates that a variable may change outside of the control of some code. For example a module's register. Or a variable updated in an interrupt handler.*

## Introduction

Accessing data in a microcontroller takes time. For example reading a 32-bit integer variable. Or writing a 16-bit value to a register. The time these actions take depend on the *native object size* of the microcontroller. For Cortex-M microcontrollers the native object size is 32-bits. Accessing objects of the native size or smaller are atomic.

It is common to work with data objects that are larger than the native object size. A struct is an example. Another example is all objects in an array. It takes more than one instruction to read or write the entire object. As multiple instructions can be interrupted, this might cause a race condition.

## Example - no race condition

The following timing diagram shows the interaction between three processes: **sw3**, **main** and **systick_isr**. The main application checks if SW3 is pressed by using the sw3 process. If so, it will read all values from the shared data object *global_buffer* (in a for-loop) and print them using the LPUART. The systick_isr interrupt handler is executed every 1 second and writes the entire buffer with one and the same value, depending on the number of seconds passed.


```txt
                    +-------------+
 sw3         main   |global_buffer|   systick_isr    lpuart
 ---         ---    +-------------+       ---         ---
  |           |            |               |           |
  | pressed() |            |               |           |
  |<----------+            |               |           |
  |   false   |            |               |           |
  +---------->|            |     write 1   |           |
  |           |            |<--------------+           |
  |           |            |<--------------+           |
  |           |            |<--------------+           |
  | pressed() |            |<--------------+           |
  |<----------+            |               |           |
  |   false   |            |               |           |
  +---------->|            |               |           |
  | pressed() |            |               |           |
  |<----------+            |               |           |
  |   true    |            |               |           |
  +---------->|   read 1   |               |           |
  |           |<-----------+               |           |
  |           |<-----------+               |           |
  |           |<-----------+               |           |
  |           |<-----------+               |           |
  |           |   print    |               |           |
  |           +------------|---------------|---------->|
  |           |            |               |           |

```

This *nice weather* scenario produces, for example, the following output:

```txt
1111111111111111
3333333333333333
6666666666666666
9999999999999999
1111111111111111
2222222222222222
```

## Example - Race condition

The execution of main depends on the input of the user. If SW3 is clicked within a certain time window, main might have started copying the *global_buffer*, but it gets interrupted by the systick_isr interrupt handler. This interrupt handler overwrites the entire shared object's content. As soon as the interrupt handler is finished the main continues copying with the updated value. This scenario is depicted in the following timing diagram.

```txt
                    +-------------+
 sw3         main   |global_buffer|   systick_isr    lpuart
 ---         ---    +-------------+       ---         ---
  |           |            |               |           |
  | pressed() |            |               |           |
  |<----------+            |               |           |
  |   false   |            |               |           |
  +---------->|            |               |           |
  | pressed() |            |               |           |
  |<----------+            |               |           |
  |   false   |            |               |           |
  +---------->|            |               |           |
  | pressed() |            |               |           |
  |<----------+            |               |           |
  |   true    |            |               |           |
  +---------->|   read 1   |               |           |
  |           |<-----------+               |           |
  |           |<-----------+     write 2   |           |
  |           |            |<--------------+           |
  |           |            |<--------------+           |
  |           |            |<--------------+           |
  |           |   read 2   |<--------------+           |
  |           |<-----------+               |           |
  |           |<-----------+               |           |
  |           |   print    |               |           |
  |           +------------|---------------|---------->|
  |           |            |               |           |

```

This *race condition* scenario produces, for example, the following output:

```txt
1111111111111111
3333333333333333
6666666666666666
9999999999999999
1111111111222222  <--  Reading the global_buffer got unintentionally interrupted
2222222222222222
```

In this example, reading the *global_buffer* in main is the critical section. This section can be protected for pre-emption by globally disabling interrupts. This is done as follows with as few instructions as possible and makes sure that the previous state of the global interrupt setting is restored when the critical section is finished.

```C
// Critical section start
uint32_t m = __get_PRIMASK();
__disable_irq(); 

// critical section goes here ...

__set_PRIMASK(m);
// Critical section end
```

The functions used in the code snippet above are part of the CMSIS. These functions are available for all Cortex-M microcontrollers.

## Verification

- Open the VCOM (9600-8n1) in a terminal application of your choice.
- Press SW3. Try to get the timing right and trigger the race condition (the buffer printing different values on a single SW3 click).

In the main-loop, there is an artificial delay. Increase the delay by a factor 10 as follows and try to trigger the race condition again.

```C
// Artificial delay to increase the chance of interrupting this
// critical section
for(uint32_t d=0; d<200000; d++)
{}
```

*`Q1` Why does increasing the artificial delay make it easier to trigger the race condition?*

Leave the increased delay in the code.

*`Q2` Protect the critical section from pre-emption by updating the code.*

If implemented correctly you should observe two things:

1. The printed line is always correct (showing one distinct character).
2. Clicking SW3 twice quickly prints one or two characters, pauses for a little bit and then finishes printing.

*`Q3` Explain observation number 2.*

## Answers

### Q1

It takes longer to make the copy, so the chance of interrupting this process increases.

### Q2

```C
// Critical section start
uint32_t m = __get_PRIMASK();
__disable_irq();             

// Copy the global buffer
for(uint32_t i=0; i<sizeof(global_buffer); i++)
{
    local_buffer[i] = global_buffer[i];
    
    // Artificial delay to increase the chance of interrupting this
    // critical section
    for(uint32_t d=0; d<200000; d++)
    {}                
}

__set_PRIMASK(m);
// Critical section end   
```

### Q3

The first click copies the buffer and prints it. Printing means it is written into the transmit buffer of the LPUART, which is transmitted interrupt based. If there is a second SW3 click, which disables all interrupts, the transmission of UART characters will temporarily stop.
