################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/wave/test/mac-extension-test-suite.cc \
../src/wave/test/ocb-test-suite.cc 

CC_DEPS += \
./src/wave/test/mac-extension-test-suite.d \
./src/wave/test/ocb-test-suite.d 

OBJS += \
./src/wave/test/mac-extension-test-suite.o \
./src/wave/test/ocb-test-suite.o 


# Each subdirectory must supply rules for building sources it contributes
src/wave/test/%.o: ../src/wave/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


