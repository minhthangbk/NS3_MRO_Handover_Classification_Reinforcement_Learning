################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/energy/test/basic-energy-harvester-test.cc \
../src/energy/test/basic-energy-model-test.cc \
../src/energy/test/li-ion-energy-source-test.cc \
../src/energy/test/rv-battery-model-test.cc 

CC_DEPS += \
./src/energy/test/basic-energy-harvester-test.d \
./src/energy/test/basic-energy-model-test.d \
./src/energy/test/li-ion-energy-source-test.d \
./src/energy/test/rv-battery-model-test.d 

OBJS += \
./src/energy/test/basic-energy-harvester-test.o \
./src/energy/test/basic-energy-model-test.o \
./src/energy/test/li-ion-energy-source-test.o \
./src/energy/test/rv-battery-model-test.o 


# Each subdirectory must supply rules for building sources it contributes
src/energy/test/%.o: ../src/energy/test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


