################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/test/csma-system-test-suite.cc 

CC_DEPS += \
./src/test/csma-system-test-suite.d 

OBJS += \
./src/test/csma-system-test-suite.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/%.o: ../src/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


