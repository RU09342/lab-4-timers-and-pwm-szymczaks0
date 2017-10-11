##### Stephen Szymczak

# LAB 4: Button Debouncing
  In all previous button related projects, the button had bouncing, an example of which can be found [here](https://i0.wp.com/coder-tronics.com/wp-content/uploads/2014/09/Switch-Debouncing-Tutorial-switch-without-debounce-circuit-poor-quality-switch.png). The issue with button bouncing in a logic circuit is that inputs may be register two or more times when the intention was to only be registered once. A button's contact points have elasticity and furthermore, humans are not precise machines. This leads to multiple inputs registering when the program's intention is to recieve one to do one thing at a time. This can be solved in two ways:
  1.  Set up a debounce circuit like shown [here](https://i0.wp.com/coder-tronics.com/wp-content/uploads/2014/09/Switch-Debouncing-Tutorial-switch-circuit-with-debounce-2nd-resistor.png?w=896).
  2.  Simulate the functionality of that circuit with code.

  If the end goal would be to take one of these processors and design a PCB board around it for some product, it may be more efficient and cheaper to design an RC debounce circuit like the one referenced above. However, we have built in buttons on the MSP430 launchpads that are not debounced physically. So we must go with option two and write some code to debounce the buttons.
  
## Concept Of How To Debounce With Code
  The basic principle of the RC debounce circuit is that a capacitor introduces a delay that smooths the transition from a high to low or low to high voltage (flipping a bit). So, at the core of our debounce code, there needs to be some kind of equivelant delay that smooths the transition. The MSP430 processors all have timers which can be enabled and disabled via code, and this can be used to implement this desired delay.
  During the time between the moment the button is pressed and the moment the button is released, we want one continuous output from the button. If this is achieved, we can safely say that the button has been debounced. This means there are two states of a button press that must be handled: the button press and the button release.  There must then be some code that handles both the button press and the button release. 
  
  ### There are four states total during a button press:
State 1.  Assuming an active high button\*: The button was low, it was pressed, and now is going high. The instant the button is pressed, we want to disable input from the button for a short period of time to ignore the bouncing signal that the button produces. This is done by enabling a timer where our desired delay is CCR0 of that timer.
  
State 2.  The button was going high, and is now completely high. This is the point that bouncing has stopped. At this point, we need to start considering what is going to happen on the release of the button. We will need to catch the falling edge of the button and also stop the timer. This state is held until the button is released. This is the state where we want to output to the rest of our code our successful button press. Notice in the code below, state 2 is where the LED is toggled.
  
State 3.  The button has been released, so we are now transitioning from high to low. The instant the button is released, we want to disable input from the button for a short period of time to ignore the bouncing signal that the button produces. This is done by enabling a timer where our desired delay is, again, CCR0.
  
State 4.  The button was going low, and is now completely low. We now want to set our debouncing code back to it's initial state so that it is ready for another button press. To do this we must turn off the timer, and re-enable our initial button settings such as the interrupt edge.
\*The actual buttons on the MSP430s are active low when a pull-up is used. The logic for interrupt edge detection must be changed accordingly.
  
 ## Implementation:
 The following code is for the FR5994, it can however be applied to the other processors. Pins and timer related names must be changed according to the specifications of the different processors.
 ```c
  //initial button settings regarding interrupts:
     P5DIR &= ~BIT5;         //SET P5.5 AS INPUT
     P5REN |= BIT5;          //ENABLE PULL UP OR DOWN FOR P5.5
     P5OUT |= BIT5;          //SPECIFIED AS A PULLUP FOR P5.5
     P5IE |= BIT5;           //SET P5.5 INTERRUPT ENABLE (S2)
     P5IFG &= ~BIT5;         //P5.5 IFG CLEARED
   P5IES |= BIT5;              //TOGGLE INTERRUPT EDGE: HIGH TO LOW
   //initial timer settings for the delay timer:
     TA0CCTL0 = CCIE;        // CCR0 interrupt ENABLE
     TA0CCR0 = 5000;        //overflow every 5ms

  #pragma vector=PORT5_VECTOR
__interrupt void PORT_5(void)
{
        switch(state)
        {
        case 0: //OFF -> GOING ON
//State 1:  
//Button has been pressed, 
//we disable inputs from the button, 
//clear the interrupt flag, 
//and turn on our CCR0 based delay via a timer.
            TA0CTL = TASSEL_2 + MC_1 + TACLR + ID_2;       // ACTIVATE TIMER (goes into case 0 in the timer ISR)
            P5IFG &= ~BIT5;                         // CLEAR FLAG FOR P5.5
            P5IE &= ~BIT5;                          // disable interrupts for P5.5 (BUTTON)
            break;
        case 1: //ON -> GOING OFF
//State 3:  
//Button has is being released, 
//we disable inputs from the button, 
//clear the interrupt flag, 
//and turn on our CCR0 based delay via a timer.        
            TA0CTL = TASSEL_2 + MC_1 + TACLR + ID_2;       //ACTIVATE TIMER (goes into case 1 in the timer ISR)
            P5IFG &= ~BIT5;                         // CLEAR FLAG FOR P5.5
            P5IE &= ~BIT5;                          // disable interrupts for P5.5 (BUTTON)
            break;
        }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
{
        switch(state)
        {
        case 0://GOING ON -> ON
//State 2:  
//Timer interrupt has occured, 
//we re-enable button interrupts but on opposite edge, 
//clear the interrupt flag, 
//and turn off our timer.        
            P1OUT ^= BIT0;              //BLINK BIT0 (OUTPUT)
            P5IE |= BIT5;               //RE-ENABLE INTERRUPTS
            P5IES &= ~BIT5;              //TOGGLE INTERRUPT EDGE: LOW TO HIGH (BUTTON release)
            TA0CTL &= ~TASSEL_2;        //STOP TIMER
            TA0CTL |= TACLR;            //CLEAR TIMER
            state = 1;                  //TO GO TO NEXT STATE IN PORT 1 ISR
            break;
        case 1://GOING OFF -> OFF
//State 4:  
//Timer interrupt has occured, 
//we re-enable button interrupts to their initial state, 
//clear the interrupt flag, 
//and turn off our timer.          
            P5IE |= BIT5;               //SET P5.5 INTERRUPT ENABLE (S2)
            P5IFG &= ~BIT5;             //P5.5 IFG CLEARED
            P5IES |= BIT5;              //TOGGLE INTERRUPT EDGE: HIGH TO LOW
            TA0CTL &= ~TASSEL_2;        //STOP TIMER
            TA0CTL |= TACLR;            //CLEAR TIMER
            state = 0;                  //UPON ANOTHER BUTTON PRESS, WE WILL ENTER CASE 0 OF PORT 1 ISR
            break;
        }
}
```
A switch statement was used to control the flow of our timer/port interrupt state machine. while the program is idling, ```int 'state'``` should be set to 0 as shown in state 4.

## Testing / Usage

To test if this debouncing code is satisfactory for some real-world purpose, some code could be written that constantly outputs the result of the debounce state machine to a pin. This pin could then be connected to an oscilloscope and observed when pressing the button. Compare the signal of the output with the input and observe how well it is debounced.

To make the use of this code, put the desired output code in state 2 in place of the line ``` P1OUT ^= BIT0;              //BLINK BIT0 (OUTPUT) ```.
