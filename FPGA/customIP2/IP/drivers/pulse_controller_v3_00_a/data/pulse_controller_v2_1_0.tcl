##############################################################################
## Filename:          C:\trosen\svn\control_trunk\FPGA\customIP\MyProcessorIPLib/drivers/pulse_controller_v3_00_a/data/pulse_controller_v2_1_0.tcl
## Description:       Microprocess Driver Command (tcl)
## Date:              Sun Aug 23 16:11:52 2009 (by Create and Import Peripheral Wizard)
##############################################################################

#uses "xillib.tcl"

proc generate {drv_handle} {
  xdefine_include_file $drv_handle "xparameters.h" "pulse_controller" "NUM_INSTANCES" "DEVICE_ID" "C_BASEADDR" "C_HIGHADDR" 
}
