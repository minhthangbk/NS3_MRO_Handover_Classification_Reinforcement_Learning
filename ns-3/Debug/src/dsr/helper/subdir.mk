################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/dsr/helper/dsr-helper.cc \
../src/dsr/helper/dsr-main-helper.cc 

CC_DEPS += \
./src/dsr/helper/dsr-helper.d \
./src/dsr/helper/dsr-main-helper.d 

OBJS += \
./src/dsr/helper/dsr-helper.o \
./src/dsr/helper/dsr-main-helper.o 


# Each subdirectory must supply rules for building sources it contributes
src/dsr/helper/%.o: ../src/dsr/helper/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


