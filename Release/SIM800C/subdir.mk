################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../SIM800C/SIM800C.c 

OBJS += \
./SIM800C/SIM800C.o 

C_DEPS += \
./SIM800C/SIM800C.d 


# Each subdirectory must supply rules for building sources it contributes
SIM800C/%.o SIM800C/%.su: ../SIM800C/%.c SIM800C/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F411xE -DSTM32_THREAD_SAFE_STRATEGY=2 -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Core/ThreadSafe -I"/Users/inikola/Documents/STM32CubeIDE/workspace_1.9.0/L1E_Bootloader/W25Rxx" -I"/Users/inikola/Documents/STM32CubeIDE/workspace_1.9.0/L1E_Bootloader/gsm_v5-master" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-SIM800C

clean-SIM800C:
	-$(RM) ./SIM800C/SIM800C.d ./SIM800C/SIM800C.o ./SIM800C/SIM800C.su

.PHONY: clean-SIM800C

