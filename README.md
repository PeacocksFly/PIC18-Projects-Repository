# PIC18-Projects-Repository
PIC microcontroller projects from Dogan Ibrahim's book implemented in C using MPLAB X IDE and Proteus

1. Stepper Motor Control - Simple Unipolar Drive (unipolar_stepper_motor.c). A unipolar stepper motor is used and the motor is rotated 100 turns before it is then stopped. https://youtu.be/mTZW4wvh8Gs

2. Stepper Motor Control - Complex Control of Unipolar Motor (complex_control_unipolar_stepper_motor.c). A unipolar stepper motor is used and the motor is rotated 20 revolutions clockwise, delay of 5s, 5 revolutions anticlockwise, delay of 3s, 10 revolutions clockwise, delay of 1s then stop. https://youtu.be/FunM3NzO7Dw

3. Stepper Motor Control - Simple Bipolar Motor Drive (bipolar_stepper_motor.c). A bipolar stepper motor is used and the motor is rotated 10 revolutions clockwise, delay of 5s, 10 revolutions anticlockwise and then stopped. https://youtu.be/qVl8OQfqO_Y

4. DC Motor Control - Simple Motor Drive. The project is about controlling the direction of rotation of a DC motor. One push-button controls the direction and the other one stars/stops the motor. Weak pull-ups are used on PORT B. https://youtu.be/ZvYFq0oZyY8

5. DC Motor Control - Speed Control. The project is about controlling the speed of a DC motor using the PWM function available on PIC18F45K22. The duty cycle is modified by steps of 10%. The initial duty cycle is equal to 50%. Once the speed steady the duty cycle is first increased by 20%, then by 30% to reach a full duty cycle. It is then decreased by 60% and finally by 40% so the motor stops. The push buttons to change the speed triggers interrupts on pin RB0 and RB1. https://youtu.be/Hhr3xz5ib34

6. Chasing Leds. Eight leds are connected to PORT C. The leds turn on alternately with one second delay interval. The leds are connected in current sourcing mode. The forward voltage/current are respectively equal to 2.2V and 10mA so the resistor was set to 280 ohms. 
