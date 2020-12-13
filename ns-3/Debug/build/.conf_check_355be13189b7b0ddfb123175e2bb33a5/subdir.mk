################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../build/.conf_check_355be13189b7b0ddfb123175e2bb33a5/test.cc 

CC_DEPS += \
./build/.conf_check_355be13189b7b0ddfb123175e2bb33a5/test.d 

OBJS += \
./build/.conf_check_355be13189b7b0ddfb123175e2bb33a5/test.o 


# Each subdirectory must supply rules for building sources it contributes
build/.conf_check_355be13189b7b0ddfb123175e2bb33a5/%.o: ../build/.conf_check_355be13189b7b0ddfb123175e2bb33a5/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


