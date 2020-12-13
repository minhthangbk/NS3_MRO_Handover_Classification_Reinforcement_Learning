################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/wimax/helper/wimax-helper.cc 

CC_DEPS += \
./src/wimax/helper/wimax-helper.d 

OBJS += \
./src/wimax/helper/wimax-helper.o 


# Each subdirectory must supply rules for building sources it contributes
src/wimax/helper/%.o: ../src/wimax/helper/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


