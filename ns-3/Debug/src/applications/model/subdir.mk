################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/applications/model/application-packet-probe.cc \
../src/applications/model/bulk-send-application.cc \
../src/applications/model/onoff-application.cc \
../src/applications/model/packet-loss-counter.cc \
../src/applications/model/packet-sink.cc \
../src/applications/model/ping6.cc \
../src/applications/model/radvd-interface.cc \
../src/applications/model/radvd-prefix.cc \
../src/applications/model/radvd.cc \
../src/applications/model/seq-ts-header.cc \
../src/applications/model/udp-client.cc \
../src/applications/model/udp-echo-client.cc \
../src/applications/model/udp-echo-server.cc \
../src/applications/model/udp-server.cc \
../src/applications/model/udp-trace-client.cc \
../src/applications/model/v4ping.cc 

CC_DEPS += \
./src/applications/model/application-packet-probe.d \
./src/applications/model/bulk-send-application.d \
./src/applications/model/onoff-application.d \
./src/applications/model/packet-loss-counter.d \
./src/applications/model/packet-sink.d \
./src/applications/model/ping6.d \
./src/applications/model/radvd-interface.d \
./src/applications/model/radvd-prefix.d \
./src/applications/model/radvd.d \
./src/applications/model/seq-ts-header.d \
./src/applications/model/udp-client.d \
./src/applications/model/udp-echo-client.d \
./src/applications/model/udp-echo-server.d \
./src/applications/model/udp-server.d \
./src/applications/model/udp-trace-client.d \
./src/applications/model/v4ping.d 

OBJS += \
./src/applications/model/application-packet-probe.o \
./src/applications/model/bulk-send-application.o \
./src/applications/model/onoff-application.o \
./src/applications/model/packet-loss-counter.o \
./src/applications/model/packet-sink.o \
./src/applications/model/ping6.o \
./src/applications/model/radvd-interface.o \
./src/applications/model/radvd-prefix.o \
./src/applications/model/radvd.o \
./src/applications/model/seq-ts-header.o \
./src/applications/model/udp-client.o \
./src/applications/model/udp-echo-client.o \
./src/applications/model/udp-echo-server.o \
./src/applications/model/udp-server.o \
./src/applications/model/udp-trace-client.o \
./src/applications/model/v4ping.o 


# Each subdirectory must supply rules for building sources it contributes
src/applications/model/%.o: ../src/applications/model/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


