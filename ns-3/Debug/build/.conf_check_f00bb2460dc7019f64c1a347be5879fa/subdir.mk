################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../build/.conf_check_f00bb2460dc7019f64c1a347be5879fa/test.cpp 

OBJS += \
./build/.conf_check_f00bb2460dc7019f64c1a347be5879fa/test.o 

CPP_DEPS += \
./build/.conf_check_f00bb2460dc7019f64c1a347be5879fa/test.d 


# Each subdirectory must supply rules for building sources it contributes
build/.conf_check_f00bb2460dc7019f64c1a347be5879fa/%.o: ../build/.conf_check_f00bb2460dc7019f64c1a347be5879fa/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


