################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../gsm_v5-master/atc.c \
../gsm_v5-master/call.c \
../gsm_v5-master/fs.c \
../gsm_v5-master/gprs.c \
../gsm_v5-master/gsm.c \
../gsm_v5-master/gsmCallback.c \
../gsm_v5-master/msg.c 

OBJS += \
./gsm_v5-master/atc.o \
./gsm_v5-master/call.o \
./gsm_v5-master/fs.o \
./gsm_v5-master/gprs.o \
./gsm_v5-master/gsm.o \
./gsm_v5-master/gsmCallback.o \
./gsm_v5-master/msg.o 

C_DEPS += \
./gsm_v5-master/atc.d \
./gsm_v5-master/call.d \
./gsm_v5-master/fs.d \
./gsm_v5-master/gprs.d \
./gsm_v5-master/gsm.d \
./gsm_v5-master/gsmCallback.d \
./gsm_v5-master/msg.d 


# Each subdirectory must supply rules for building sources it contributes
gsm_v5-master/%.o gsm_v5-master/%.su: ../gsm_v5-master/%.c gsm_v5-master/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F411xE -DSTM32_THREAD_SAFE_STRATEGY=2 -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Core/ThreadSafe -I"/Users/inikola/Documents/STM32CubeIDE/workspace_1.9.0/L1E_Bootloader/W25Rxx" -I"/Users/inikola/Documents/STM32CubeIDE/workspace_1.9.0/L1E_Bootloader/gsm_v5-master" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-gsm_v5-2d-master

clean-gsm_v5-2d-master:
	-$(RM) ./gsm_v5-master/atc.d ./gsm_v5-master/atc.o ./gsm_v5-master/atc.su ./gsm_v5-master/call.d ./gsm_v5-master/call.o ./gsm_v5-master/call.su ./gsm_v5-master/fs.d ./gsm_v5-master/fs.o ./gsm_v5-master/fs.su ./gsm_v5-master/gprs.d ./gsm_v5-master/gprs.o ./gsm_v5-master/gprs.su ./gsm_v5-master/gsm.d ./gsm_v5-master/gsm.o ./gsm_v5-master/gsm.su ./gsm_v5-master/gsmCallback.d ./gsm_v5-master/gsmCallback.o ./gsm_v5-master/gsmCallback.su ./gsm_v5-master/msg.d ./gsm_v5-master/msg.o ./gsm_v5-master/msg.su

.PHONY: clean-gsm_v5-2d-master

