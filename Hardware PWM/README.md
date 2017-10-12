##### Stephen Szymczak

# LAB 4: Hardware PWM
  Pulse width modulation is a very useful thing to manipulate. It can be used to rotate a motor at different speeds, and have LEDs light at different intensities. Hardware PWM on the MSP430 processors is done by outputting a pulse signal directly to output pins. For all boards except the F5529L, LED pins are capable of taking signals from the processor. For the F5529L, a jumper cable was used to connect a signal capable pin to the LED pin as shown [here](https://files.slack.com/files-pri/T6KAE3VFA-F7FH9E9HR/jpeg_20171009_153538.jpg).
  
## Implementation
###### It should be noted the following code was pulled from the hardware PWM for the FR5994 as an example of the logic required. Not all of the code is included, and it is assumed that the left out code is understood from previous projects and labs.

  There are three main things to do to get hardware PWM working:
 ### 1.  set an LED pin to being directly connected to the timer, the exact code needed changes from board to board and can be found in the data sheets:
```c
     P1DIR |= BIT0;            // P1.0 as output
    P1SEL0 |= BIT0;                  // P1.0 option select
```
 ### 2.  set up code to make the timer output going to the LED pin actually do PWM:
```c
TA0CCTL1 = OUTMOD_7;                // CCR1 reset/set
TA0CCR0 = 1000;                   //overflow every 1ms FOR PWM PERIOD
TA0CCR1 = 500;                      //PWM starts at 50%
TA0CTL = TASSEL_2 | MC_1;           //SMCLK, up mode
```
  The new code here is ```TA0CCTL1 = OUTMOD_7```. This code in junction with ```P1SEL0 |= BIT0;``` assigns high voltage to pin 1.0 from TA0R = 0 to TA0R = CCR1. Having CCR0 = 1000 with SMCLK and UP mode, sets our frequency of TA0R's reset point to be 1kHz. So, within that period (1ms) output to the pin is high for the ratio between CCR0 and CCR1. For example, if CCR0 is 1000 and CCR1 is 200, then the LED pin will be high for 20% of the time. This is pulse width modulation.
  
  ### 3.  set up code to increase PWM by 10% with every button press:
```c
#pragma vector=PORT5_VECTOR//button interrupt
__interrupt void PORT_5(void)
{

    TA1CTL = TASSEL_2 + MC_1;                  // ACTIVATE TIMER

    P5IFG &= ~BIT5;     //P5.5 IFG CLEARED
    P5IES &= ~BIT5;      //TOGGLE INTERRUPT EDGE
}

#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0 (void)
{
    if(pwm < 1000) pwm += 100; // increase pwm by 100 when button is pressed
    else pwm = 0;
    P5IE |= BIT5;           //ENABLE INTERRUPT
    TA0CCR1 = pwm;    //PWM starts at 50% and increase by PWM (100) then reset to zero
    TA1CTL = 0x00;          //STOP TIMER
}
```
Let's dissect this code a bit. 

First, we use a separate timer (timer a1) for a timer interrupt and a port interrupt to partially handle button bouncing: 
```c
// port 5 interrupt for button detection:
#pragma vector=PORT5_VECTOR//button interrupt
__interrupt void PORT_5(void)

// timer a1 interrupt for debouncing*
#pragma vector=TIMER1_A0_VECTOR
__interrupt void Timer1_A0 (void)
```
\*For much better button debouncing reference the button debouncing portion of this LAB 4 repository. It works well enough here to check that the PWM is changing with button presses.

Assuming we know why the button debounce code is there, we are left with the core code that is used to increment the PWM:
```c
if(pwm < 1000) pwm += 100; // increase pwm by 100 when button is pressed
else pwm = 0;
TA0CCR1 = pwm;    //PWM starts at 50% and increase by PWM (100) then reset to zero
```
CCR1 is incremented by 100, starting at 500, and upon going over 1000, CCR1 is set to 0.
