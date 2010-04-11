##############################################################################
## Filename:          C:\dev\control_trunk\FPGA\DDS_GbE_11_2/drivers/pulse_controller_v3_01_a/data/pulse_controller_v2_1_0.tcl
## Description:       Microprocess Driver Command (tcl)
## Date:              Wed Nov 18 19:08:40 2009 (by Create and Import Peripheral Wizard)
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
  xdefine_include_file $drv_handle "xparameters.h" "pulse_controller" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" 
}
