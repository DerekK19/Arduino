################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
/Applications/Developer/Arduino.app/Contents/Resources/Java/libraries/Servo/Servo.cpp 

OBJS += \
./Servo/Servo.o 

CPP_DEPS += \
./Servo/Servo.d 


# Each subdirectory must supply rules for building sources it contributes
Servo/Servo.o: /Applications/Developer/Arduino.app/Contents/Resources/Java/libraries/Servo/Servo.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: AVR C++ Compiler'
	avr-g++ -I../arduino -I"/Users/derek/Solutions/Arduino/Robot/Servo_Test" -I../Servo_Test/Servo -I/Users/derek/Solutions/Arduino/libraries -I"/Applications/Developer/Arduino.app/Contents/Resources/Java/libraries/Servo" -D__IN_ECLIPSE__=1 -DARDUINO=10.1 -Wall -Os -ffunction-sections -fdata-sections -fno-exceptions -g -mmcu=atmega328p -DF_CPU=16000000UL -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -c -o "$@" -x c++ "$<"
	@echo 'Finished building: $<'
	@echo ' '


