################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/mobility/model/back-forth-between-two-points-mobility-model.cc \
../src/mobility/model/box.cc \
../src/mobility/model/circle-mobility-model.cc \
../src/mobility/model/constant-acceleration-mobility-model.cc \
../src/mobility/model/constant-position-mobility-model.cc \
../src/mobility/model/constant-velocity-helper.cc \
../src/mobility/model/constant-velocity-mobility-model.cc \
../src/mobility/model/gauss-markov-mobility-model.cc \
../src/mobility/model/geographic-positions.cc \
../src/mobility/model/hierarchical-mobility-model.cc \
../src/mobility/model/manhattan-conor-mobility-model.cc \
../src/mobility/model/manhattan-grid-mobility-model.cc \
../src/mobility/model/manhattan-grid-mobility-model1.cc \
../src/mobility/model/mobility-model.cc \
../src/mobility/model/position-allocator.cc \
../src/mobility/model/random-back-forth-cross-ball-mobility-model.cc \
../src/mobility/model/random-circle-cross-circle.cc \
../src/mobility/model/random-circle-in-disc.cc \
../src/mobility/model/random-circle-in-rectangle.cc \
../src/mobility/model/random-direction-2d-mobility-model.cc \
../src/mobility/model/random-edge-hexagonal-mobility-model.cc \
../src/mobility/model/random-walk-2d-mobility-model.cc \
../src/mobility/model/random-waypoint-mobility-model.cc \
../src/mobility/model/rectangle.cc \
../src/mobility/model/steady-state-random-waypoint-doughnut-mobility-model.cc \
../src/mobility/model/steady-state-random-waypoint-mobility-model.cc \
../src/mobility/model/waypoint-mobility-model.cc \
../src/mobility/model/waypoint.cc 

CC_DEPS += \
./src/mobility/model/back-forth-between-two-points-mobility-model.d \
./src/mobility/model/box.d \
./src/mobility/model/circle-mobility-model.d \
./src/mobility/model/constant-acceleration-mobility-model.d \
./src/mobility/model/constant-position-mobility-model.d \
./src/mobility/model/constant-velocity-helper.d \
./src/mobility/model/constant-velocity-mobility-model.d \
./src/mobility/model/gauss-markov-mobility-model.d \
./src/mobility/model/geographic-positions.d \
./src/mobility/model/hierarchical-mobility-model.d \
./src/mobility/model/manhattan-conor-mobility-model.d \
./src/mobility/model/manhattan-grid-mobility-model.d \
./src/mobility/model/manhattan-grid-mobility-model1.d \
./src/mobility/model/mobility-model.d \
./src/mobility/model/position-allocator.d \
./src/mobility/model/random-back-forth-cross-ball-mobility-model.d \
./src/mobility/model/random-circle-cross-circle.d \
./src/mobility/model/random-circle-in-disc.d \
./src/mobility/model/random-circle-in-rectangle.d \
./src/mobility/model/random-direction-2d-mobility-model.d \
./src/mobility/model/random-edge-hexagonal-mobility-model.d \
./src/mobility/model/random-walk-2d-mobility-model.d \
./src/mobility/model/random-waypoint-mobility-model.d \
./src/mobility/model/rectangle.d \
./src/mobility/model/steady-state-random-waypoint-doughnut-mobility-model.d \
./src/mobility/model/steady-state-random-waypoint-mobility-model.d \
./src/mobility/model/waypoint-mobility-model.d \
./src/mobility/model/waypoint.d 

OBJS += \
./src/mobility/model/back-forth-between-two-points-mobility-model.o \
./src/mobility/model/box.o \
./src/mobility/model/circle-mobility-model.o \
./src/mobility/model/constant-acceleration-mobility-model.o \
./src/mobility/model/constant-position-mobility-model.o \
./src/mobility/model/constant-velocity-helper.o \
./src/mobility/model/constant-velocity-mobility-model.o \
./src/mobility/model/gauss-markov-mobility-model.o \
./src/mobility/model/geographic-positions.o \
./src/mobility/model/hierarchical-mobility-model.o \
./src/mobility/model/manhattan-conor-mobility-model.o \
./src/mobility/model/manhattan-grid-mobility-model.o \
./src/mobility/model/manhattan-grid-mobility-model1.o \
./src/mobility/model/mobility-model.o \
./src/mobility/model/position-allocator.o \
./src/mobility/model/random-back-forth-cross-ball-mobility-model.o \
./src/mobility/model/random-circle-cross-circle.o \
./src/mobility/model/random-circle-in-disc.o \
./src/mobility/model/random-circle-in-rectangle.o \
./src/mobility/model/random-direction-2d-mobility-model.o \
./src/mobility/model/random-edge-hexagonal-mobility-model.o \
./src/mobility/model/random-walk-2d-mobility-model.o \
./src/mobility/model/random-waypoint-mobility-model.o \
./src/mobility/model/rectangle.o \
./src/mobility/model/steady-state-random-waypoint-doughnut-mobility-model.o \
./src/mobility/model/steady-state-random-waypoint-mobility-model.o \
./src/mobility/model/waypoint-mobility-model.o \
./src/mobility/model/waypoint.o 


# Each subdirectory must supply rules for building sources it contributes
src/mobility/model/%.o: ../src/mobility/model/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


