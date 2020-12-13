################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/test/ns3wifi/wifi-interference-test-suite.cc \
../src/test/ns3wifi/wifi-msdu-aggregator-test-suite.cc 

CC_DEPS += \
./src/test/ns3wifi/wifi-interference-test-suite.d \
./src/test/ns3wifi/wifi-msdu-aggregator-test-suite.d 

OBJS += \
./src/test/ns3wifi/wifi-interference-test-suite.o \
./src/test/ns3wifi/wifi-msdu-aggregator-test-suite.o 


# Each subdirectory must supply rules for building sources it contributes
src/test/ns3wifi/%.o: ../src/test/ns3wifi/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


