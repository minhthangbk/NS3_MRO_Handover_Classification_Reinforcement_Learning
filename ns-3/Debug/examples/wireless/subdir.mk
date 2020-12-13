################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../examples/wireless/ht-wifi-network.cc \
../examples/wireless/mixed-wireless.cc \
../examples/wireless/multirate.cc \
../examples/wireless/ofdm-ht-validation.cc \
../examples/wireless/ofdm-validation.cc \
../examples/wireless/power-adaptation-distance.cc \
../examples/wireless/power-adaptation-interference.cc \
../examples/wireless/rate-adaptation-distance.cc \
../examples/wireless/simple-ht-hidden-stations.cc \
../examples/wireless/simple-mpdu-aggregation.cc \
../examples/wireless/simple-msdu-aggregation.cc \
../examples/wireless/simple-two-level-aggregation.cc \
../examples/wireless/wifi-adhoc.cc \
../examples/wireless/wifi-ap.cc \
../examples/wireless/wifi-blockack.cc \
../examples/wireless/wifi-clear-channel-cmu.cc \
../examples/wireless/wifi-hidden-terminal.cc \
../examples/wireless/wifi-simple-adhoc-grid.cc \
../examples/wireless/wifi-simple-adhoc.cc \
../examples/wireless/wifi-simple-infra.cc \
../examples/wireless/wifi-simple-interference.cc \
../examples/wireless/wifi-sleep.cc \
../examples/wireless/wifi-timing-attributes.cc \
../examples/wireless/wifi-wired-bridging.cc 

CC_DEPS += \
./examples/wireless/ht-wifi-network.d \
./examples/wireless/mixed-wireless.d \
./examples/wireless/multirate.d \
./examples/wireless/ofdm-ht-validation.d \
./examples/wireless/ofdm-validation.d \
./examples/wireless/power-adaptation-distance.d \
./examples/wireless/power-adaptation-interference.d \
./examples/wireless/rate-adaptation-distance.d \
./examples/wireless/simple-ht-hidden-stations.d \
./examples/wireless/simple-mpdu-aggregation.d \
./examples/wireless/simple-msdu-aggregation.d \
./examples/wireless/simple-two-level-aggregation.d \
./examples/wireless/wifi-adhoc.d \
./examples/wireless/wifi-ap.d \
./examples/wireless/wifi-blockack.d \
./examples/wireless/wifi-clear-channel-cmu.d \
./examples/wireless/wifi-hidden-terminal.d \
./examples/wireless/wifi-simple-adhoc-grid.d \
./examples/wireless/wifi-simple-adhoc.d \
./examples/wireless/wifi-simple-infra.d \
./examples/wireless/wifi-simple-interference.d \
./examples/wireless/wifi-sleep.d \
./examples/wireless/wifi-timing-attributes.d \
./examples/wireless/wifi-wired-bridging.d 

OBJS += \
./examples/wireless/ht-wifi-network.o \
./examples/wireless/mixed-wireless.o \
./examples/wireless/multirate.o \
./examples/wireless/ofdm-ht-validation.o \
./examples/wireless/ofdm-validation.o \
./examples/wireless/power-adaptation-distance.o \
./examples/wireless/power-adaptation-interference.o \
./examples/wireless/rate-adaptation-distance.o \
./examples/wireless/simple-ht-hidden-stations.o \
./examples/wireless/simple-mpdu-aggregation.o \
./examples/wireless/simple-msdu-aggregation.o \
./examples/wireless/simple-two-level-aggregation.o \
./examples/wireless/wifi-adhoc.o \
./examples/wireless/wifi-ap.o \
./examples/wireless/wifi-blockack.o \
./examples/wireless/wifi-clear-channel-cmu.o \
./examples/wireless/wifi-hidden-terminal.o \
./examples/wireless/wifi-simple-adhoc-grid.o \
./examples/wireless/wifi-simple-adhoc.o \
./examples/wireless/wifi-simple-infra.o \
./examples/wireless/wifi-simple-interference.o \
./examples/wireless/wifi-sleep.o \
./examples/wireless/wifi-timing-attributes.o \
./examples/wireless/wifi-wired-bridging.o 


# Each subdirectory must supply rules for building sources it contributes
examples/wireless/%.o: ../examples/wireless/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


