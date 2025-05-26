library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity spi_dual_motor_hbridge is
    generic (
        -- clock divider: PWM-freq = clk_sys / (CLK_DIV_G * 64)
        CLK_DIV_G : positive := 2500          -- 50 MHz /1000 /64 ≈ 781 Hz
    );
    port (
        -- ========= SPI signals =========
        sck     : in  std_logic;   -- SPI clock  (rising-edge sample)
        cs_n    : in  std_logic;   -- chip-select, active-low
        mosi    : in  std_logic;

        -- ========= system clock =========
        clk_sys : in  std_logic;   -- e.g. 125 MHz board oscillator

        -- ========= H-bridge outputs (PAN) =========
        en_pan  : out std_logic;
        in1_pan : out std_logic;
        in2_pan : out std_logic;

        -- ========= H-bridge outputs (TILT) =========
        en_tilt  : out std_logic;
        in1_tilt : out std_logic;
        in2_tilt : out std_logic;
        
        led_alive   : out std_logic;          
        led_spi     : out std_logic
    );
end entity;

architecture rtl of spi_dual_motor_hbridge is
    ------------------------------------------------------------------
    -- SPI reception shift-register
    ------------------------------------------------------------------
    signal shift   : std_logic_vector(7 downto 0);
    signal bit_cnt : unsigned(2 downto 0) := (others=>'0');

    -- latched registers per motor
    signal pwm_pan_r   : unsigned(5 downto 0) := (others=>'0');
    signal dir_pan_r   : std_logic := '0';
    signal pwm_tilt_r  : unsigned(5 downto 0) := (others=>'0');
    signal dir_tilt_r  : std_logic := '0';
    
    signal led_spi_r  : std_logic := '0';  -- toggles on each SPI byte
    signal blink_div  : unsigned(25 downto 0) := (others => '0');
    signal blink_led  : std_logic             := '0';

    ------------------------------------------------------------------
    -- PWM generator
    ------------------------------------------------------------------
    signal div_cnt  : unsigned(15 downto 0) := (others=>'0');
    signal pwm_cnt  : unsigned(5 downto 0)  := (others=>'0');
begin
    ------------------------------------------------------------------
    -- 1.  SPI shift-in (Mode-0)
    ------------------------------------------------------------------
    process(sck)
    begin
        if rising_edge(sck) then
            if cs_n = '1' then
                bit_cnt <= (others=>'0');
            else
                shift   <= shift(6 downto 0) & mosi;
                bit_cnt <= bit_cnt + 1;

                if bit_cnt = "111" then            -- byte complete
                led_spi_r <= not led_spi_r;            -- flash
                    if shift(7) = '0' then         -- M = 0  → PAN
                        dir_pan_r  <= shift(6);
                        pwm_pan_r  <= unsigned(shift(5 downto 0));
                    else                            -- M = 1  → TILT
                        dir_tilt_r <= shift(6);
                        pwm_tilt_r <= unsigned(shift(5 downto 0));
                    end if;
                end if;
            end if;
        end if;
    end process;

    ------------------------------------------------------------------
    -- 2.  6-bit PWM counter  (shared by both motors)
    ------------------------------------------------------------------
    process(clk_sys)
    begin
        if rising_edge(clk_sys) then
            if div_cnt = CLK_DIV_G-1 then
                div_cnt <= (others=>'0');
                pwm_cnt <= pwm_cnt + 1;
            else
                div_cnt <= div_cnt + 1;
            end if;
        end if;
    end process;

    -- 3.  1 Hz heartbeat, runs in parallel with everything else
    process(clk_sys)
    begin
      if rising_edge(clk_sys) then
        blink_div <= blink_div + 1;
        if blink_div = 0 then
          blink_led <= not blink_led;       -- toggles ~0.75 Hz
        end if;
      end if;
    end process;

    ------------------------------------------------------------------
    -- 3.  Map to H-bridge pins
    ------------------------------------------------------------------
    en_pan  <= '1';                              -- always enabled
    en_tilt <= '1';

-- PAN motor
in1_pan <= '1' when (dir_pan_r = '0' and pwm_cnt < pwm_pan_r) else '0';
in2_pan <= '1' when (dir_pan_r = '1' and pwm_cnt < pwm_pan_r) else '0';

-- TILT motor
in1_tilt <= '1' when (dir_tilt_r = '0' and pwm_cnt < pwm_tilt_r) else '0';
in2_tilt <= '1' when (dir_tilt_r = '1' and pwm_cnt < pwm_tilt_r) else '0';

led_spi   <= led_spi_r;    -- LD0 flashes on SPI reception
led_alive <= blink_led;    -- LD1 heart-beats at ~1 Hz

end architecture;
