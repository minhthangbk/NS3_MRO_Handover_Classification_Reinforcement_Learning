################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/tap-bridge/helper/tap-bridge-helper.cc 

CC_DEPS += \
./src/tap-bridge/helper/tap-bridge-helper.d 

OBJS += \
./src/tap-bridge/helper/tap-bridge-helper.o 


# Each subdirectory must supply rules for building sources it contributes
src/tap-bridge/helper/%.o: ../src/tap-bridge/helper/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


