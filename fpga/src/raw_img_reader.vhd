																															   ---------------------------------------------------------------------------------
-- Title       : raw_img_reader
-- Design      : FIMD
-- Author      : vrba.vojtech [at] fel.cvut.cz
-- Company     : Czech Technical University in Prague
--						
-------------------------------------------------------------------------------
--
-- Description : Entity for reading binary images, simulates HW parallel interface.
--
-------------------------------------------------------------------------------

LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;

LIBRARY std;
USE std.textio.ALL;

ENTITY raw_img_reader IS
    GENERIC (
        g_FILENAME   : STRING;
        g_IMG_WIDTH  : INTEGER RANGE 0 TO 1024;
        g_IMG_HEIGHT : INTEGER RANGE 0 TO 1024;
        g_RST_ACTIVE : STD_LOGIC := '0'
    );
    PORT (
        i_CLOCK : IN STD_LOGIC;
        i_RESET : IN STD_LOGIC;
        o_POS_X : OUT STD_LOGIC_VECTOR(15 DOWNTO 0) := (OTHERS => '0');
        o_POS_Y : OUT STD_LOGIC_VECTOR(15 DOWNTO 0) := (OTHERS => '0');
        o_DATA  : OUT STD_LOGIC_VECTOR(7 DOWNTO 0)  := (OTHERS => '0');
        o_VALID : OUT STD_LOGIC                     := '0';
        o_DONE  : OUT STD_LOGIC                     := '0'
    );
END ENTITY;

ARCHITECTURE rtl OF raw_img_reader IS
    TYPE t_CHAR_FILE IS FILE OF CHARACTER;

    SIGNAL r_VALID : STD_LOGIC := '0';
    SIGNAL r_DONE  : STD_LOGIC := '0';

    FILE r_FILE : t_CHAR_FILE;

BEGIN
    read_file : PROCESS (i_RESET, i_CLOCK) IS
        VARIABLE v_CHAR   : CHARACTER;
        VARIABLE v_POS_X  : INTEGER RANGE -1 TO 1024;
        VARIABLE v_POS_Y  : INTEGER RANGE 0 TO 1024;
    BEGIN
        IF i_RESET = g_RST_ACTIVE THEN
            o_DATA   <= (OTHERS => '0');
            o_POS_X  <= (OTHERS => '0');
            o_POS_Y  <= (OTHERS => '0');
            r_DONE   <= '0';
            IF r_VALID = '1' THEN
                file_close(r_FILE);
                r_VALID <= '0';
            END IF;
        ELSE
            IF falling_edge(i_CLOCK) AND r_DONE = '0' THEN
                IF r_VALID = '0' THEN
                    file_open(r_FILE, g_FILENAME, read_mode);
                    r_VALID <= '1';
                    v_POS_X := - 1;
                END IF;

                v_POS_X := v_POS_X + 1;
                IF v_POS_X = g_IMG_WIDTH THEN
                    v_POS_X := 0;
                    v_POS_Y := v_POS_Y + 1;
                END IF;
                IF v_POS_Y = g_IMG_HEIGHT OR endfile(r_FILE) THEN
                    file_close(r_FILE);
                    r_VALID <= '0';
                    o_DATA  <= (OTHERS => '0');
                    r_DONE  <= '1';
                    v_POS_Y := 0;
                ELSE
                    read(r_FILE, v_CHAR);
                    o_DATA <= STD_LOGIC_VECTOR(to_unsigned(CHARACTER'pos(v_CHAR), 8));
                END IF;

                o_POS_X <= STD_LOGIC_VECTOR(to_unsigned(v_POS_X, 16));
                o_POS_Y <= STD_LOGIC_VECTOR(to_unsigned(v_POS_Y, 16));
            END IF;
        END IF;
    END PROCESS;

    o_VALID <= r_VALID;
    o_DONE  <= r_DONE;
END ARCHITECTURE;
