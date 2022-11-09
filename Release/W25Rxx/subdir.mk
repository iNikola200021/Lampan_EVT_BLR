################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../W25Rxx/w25rxx.c 

OBJS += \
./W25Rxx/w25rxx.o 

C_DEPS += \
./W25Rxx/w25rxx.d 


# Each subdirectory must supply rules for building sources it contributes
W25Rxx/w25rxx.o: ../W25Rxx/w25rxx.c W25Rxx/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F411xE -DSTM32_THREAD_SAFE_STRATEGY=2 -DUSE_FULL_LL_DRIVER -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I../Core/ThreadSafe -I"/Users/inikola/Documents/STM32CubeIDE/workspace_1.9.0/L1E_Bootloader/W25Rxx" -I../W25Rxx -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-W25Rxx

clean-W25Rxx:
	-$(RM) ./W25Rxx/w25rxx.d ./W25Rxx/w25rxx.o ./W25Rxx/w25rxx.su

.PHONY: clean-W25Rxx
