################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/csma-layout/model/csma-star-helper.cc 

CC_DEPS += \
./src/csma-layout/model/csma-star-helper.d 

OBJS += \
./src/csma-layout/model/csma-star-helper.o 


# Each subdirectory must supply rules for building sources it contributes
src/csma-layout/model/%.o: ../src/csma-layout/model/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


