library ieee;
use ieee.std_logic_1164.all;

entity top is
    port (
        -- ---------- SPI from the Tiva ----------
        sck    : in  std_logic;      -- Arduino D13  (W15)
        mosi   : in  std_logic;      -- Arduino D11  (T12)
        cs_n   : in  std_logic;      -- Arduino D10  (F16)

        -- ---------- board clock ----------
        clk_sys  : in  std_logic;      -- 125-MHz 

        -- ---------- H-bridge pins : PAN ----------
        en_pan  : out std_logic;
        in1_pan : out std_logic;
        in2_pan : out std_logic;

        -- ---------- H-bridge pins : TILT ----------
        en_tilt  : out std_logic;
        in1_tilt : out std_logic;
        in2_tilt : out std_logic;
        
        led_alive  : out std_logic;   -- LD1: alive LED 
        led_spi : out std_logic       -- LD0: flashes on SPI reception

    );
end entity;

architecture rtl of top is
begin
    u_ctrl : entity work.spi_dual_motor_hbridge
        generic map (
            CLK_DIV_G => 2500               -- 125 Mhz /2500 /64 ≈ 781 Hz
        )
        port map (
            -- SPI
            sck     => sck,
            cs_n    => cs_n,
            mosi    => mosi,

            -- system clock
            clk_sys => clk_sys,

            -- PAN outputs
            en_pan  => en_pan,
            in1_pan => in1_pan,
            in2_pan => in2_pan,

            -- TILT outputs
            en_tilt  => en_tilt,
            in1_tilt => in1_tilt,
            in2_tilt => in2_tilt,
            
            led_spi => led_spi, 
            led_alive => led_alive
        );
end architecture;
