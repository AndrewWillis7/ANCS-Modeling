################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C6000 Compiler'
	"C:/Users/salto/Downloads/c6000_7.4.14/bin/cl6x" -mv6740 --abi=coffabi -g --include_path="C:/Users/salto/workspace_v12/myfirstProject1" --include_path="C:/Users/salto/Downloads/c6000_7.4.14/include" --diag_wrap=off --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '

%.obj: ../%.asm $(GEN_OPTS) | $(GEN_FILES) $(GEN_MISC_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: C6000 Compiler'
	"C:/Users/salto/Downloads/c6000_7.4.14/bin/cl6x" -mv6740 --abi=coffabi -g --include_path="C:/Users/salto/workspace_v12/myfirstProject1" --include_path="C:/Users/salto/Downloads/c6000_7.4.14/include" --diag_wrap=off --diag_warning=225 --display_error_number --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


