################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../build/.conf_check_efd20fc635aad783e4d3953701740533/test.cc 

CC_DEPS += \
./build/.conf_check_efd20fc635aad783e4d3953701740533/test.d 

OBJS += \
./build/.conf_check_efd20fc635aad783e4d3953701740533/test.o 


# Each subdirectory must supply rules for building sources it contributes
build/.conf_check_efd20fc635aad783e4d3953701740533/%.o: ../build/.conf_check_efd20fc635aad783e4d3953701740533/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


