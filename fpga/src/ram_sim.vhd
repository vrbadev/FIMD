-------------------------------------------------------------------------------
--
-- Title       : ram_sim
-- Design      : FIMD
-- Author      : vrba.vojtech [at] fel.cvut.cz
-- Company     : Czech Technical University in Prague
--                        
-------------------------------------------------------------------------------
--
-- Description : Entity for simulation of Block RAM memory.
--
-------------------------------------------------------------------------------

LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

ENTITY ram_sim IS
    GENERIC (
        g_SIZE     : INTEGER;
        g_DATA_LEN : INTEGER := 8;
        g_ADDR_LEN : INTEGER := 8
    );
    PORT (
        i_ADDRESS : IN STD_LOGIC_VECTOR (g_ADDR_LEN - 1 DOWNTO 0);
        i_CLOCK   : IN STD_LOGIC := '1';
        i_DATA    : IN STD_LOGIC_VECTOR (g_DATA_LEN - 1 DOWNTO 0);
        i_WREN    : IN STD_LOGIC;
        o_Q       : OUT STD_LOGIC_VECTOR (g_DATA_LEN - 1 DOWNTO 0)
    );
END ENTITY;

ARCHITECTURE rtl OF ram_sim IS
    SUBTYPE t_WORD IS STD_LOGIC_VECTOR (g_DATA_LEN - 1 DOWNTO 0);
    TYPE t_MEMORY IS ARRAY (0 TO g_SIZE - 1) OF t_WORD;

    SIGNAL r_MEMORY : t_MEMORY     := (OTHERS => (OTHERS => '0'));
    SIGNAL r_DATA     : t_WORD    := (OTHERS => '0');
BEGIN
    PROCESS (i_CLOCK) IS
        VARIABLE v_ADDRESS : INTEGER := 0;
    BEGIN
        IF falling_edge(i_CLOCK) THEN
            v_ADDRESS := to_integer(UNSIGNED(i_ADDRESS));
            IF v_ADDRESS < g_SIZE THEN
                IF i_WREN = '1' THEN
                    r_MEMORY(v_ADDRESS) <= i_DATA;
                END IF;
                r_DATA <= r_MEMORY(v_ADDRESS);
            ELSE
                REPORT "Memory address exceeds memory size!" SEVERITY warning;
            END IF;
        END IF;
    END PROCESS;

    o_Q <= r_DATA;
END ARCHITECTURE;
