########################################################################
##  SPI pins on Arduino header  (same ones you tested)
########################################################################
set_property PACKAGE_PIN W15 [get_ports {sck}]
set_property IOSTANDARD  LVCMOS33 [get_ports {sck}]

set_property PACKAGE_PIN T12 [get_ports {mosi}]
set_property IOSTANDARD  LVCMOS33 [get_ports {mosi}]

set_property PACKAGE_PIN F16 [get_ports {cs_n}]
set_property IOSTANDARD  LVCMOS33 [get_ports {cs_n}]

# external clock "estimate" to keep Vivado timing happy
create_clock -name spi_clk -period 100.0 [get_ports {sck}]
set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets {sck_IBUF}]

# Use the on-board 125 MHz PL oscillator (pin H16)
set_property PACKAGE_PIN H16 [get_ports {clk_sys}]
set_property IOSTANDARD LVCMOS33 [get_ports {clk_sys}]
create_clock -period 8.000 [get_ports {clk_sys}]   # 125 MHz ⇒ 8 ns period
set_property CLOCK_DEDICATED_ROUTE FALSE [get_nets -hierarchical *clk_sys*]

# Allow unconstrained ports (only clk50 in our case) to not break bitstream
set_property SEVERITY {Warning} [get_drc_checks UCIO-1]

########################################################################
##  H-bridge output pins  (UPDATE to the header pins you wired!)
########################################################################
# ---------- PAN motor ----------
set_property PACKAGE_PIN Y18 [get_ports {en_pan}]   ;
set_property IOSTANDARD LVCMOS33 [get_ports {en_pan}]

set_property PACKAGE_PIN U18 [get_ports {in1_pan}]
set_property IOSTANDARD LVCMOS33 [get_ports {in1_pan}]

set_property PACKAGE_PIN U19 [get_ports {in2_pan}]
set_property IOSTANDARD LVCMOS33 [get_ports {in2_pan}]

# ---------- TILT motor ----------
set_property PACKAGE_PIN Y14 [get_ports {en_tilt}]
set_property IOSTANDARD LVCMOS33 [get_ports {en_tilt}]

set_property PACKAGE_PIN W16 [get_ports {in1_tilt}]
set_property IOSTANDARD LVCMOS33 [get_ports {in1_tilt}]

set_property PACKAGE_PIN T11 [get_ports {in2_tilt}]
set_property IOSTANDARD LVCMOS33 [get_ports {in2_tilt}]

# LD0 -> debug SPI flash
set_property PACKAGE_PIN R14 [get_ports {led_spi}]
set_property IOSTANDARD LVCMOS33 [get_ports {led_spi}]

# LD1 -> debug heartbeat flash
set_property PACKAGE_PIN P14 [get_ports {led_alive}]
set_property IOSTANDARD LVCMOS33 [get_ports {led_alive}]
