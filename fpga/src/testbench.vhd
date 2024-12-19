  																															   -------------------------------------------------------------------------------
--
-- Title       : testbench
-- Design      : FIMD
-- Author      : vrba.vojtech [at] fel.cvut.cz
-- Company     : Czech Technical University in Prague
--						
-------------------------------------------------------------------------------
--
-- Description : Testbench for Fast Isolated Marker Detection
--
-------------------------------------------------------------------------------

LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;	

LIBRARY std;
USE std.textio.ALL;


ENTITY testbench IS
	GENERIC
	(	
		g_BUFFER_ROWS 	: NATURAL	:= 9;                    
		g_RADIUS 		: NATURAL 	:= 4;
		g_PIX_BITS		: NATURAL	:= 8;
		g_WIDTH_BITS	: NATURAL	:= 16;
		g_HEIGHT_BITS	: NATURAL	:= 16;
		g_IMG_PATH		: STRING	:= "./1619240573769481609.bin";
		g_IMG_WIDTH		: NATURAL	:= 752;
		g_IMG_HEIGHT	: NATURAL	:= 480;
		g_FIMD_TM		: NATURAL	:= 120;
		g_FIMD_TS		: NATURAL	:= 240;
		g_FIMD_TD		: NATURAL	:= 60
	);
END testbench;	

ARCHITECTURE rtl OF testbench IS
    COMPONENT raw_img_reader IS				
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
    END COMPONENT;

    COMPONENT ram_sim IS 
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
    END COMPONENT;

    COMPONENT fimd IS		
		GENERIC
		(	
			g_BUFFER_ROWS 	: NATURAL	:= 9;                    
			g_RADIUS 		: NATURAL 	:= 3;
			g_PIX_BITS		: NATURAL	:= 8
		);
		PORT
		( 		   
			i_CLOCK		: IN STD_LOGIC;
			i_NRST		: IN STD_LOGIC;
										
	        i_THRESHOLD			: IN INTEGER RANGE 0 TO 2**g_PIX_BITS-1						:= 0;
	        i_THRESHOLD_DIFF	: IN INTEGER RANGE 0 TO 2**g_PIX_BITS-1						:= 0;
	        i_THRESHOLD_SUN		: IN INTEGER RANGE 0 TO 2**g_PIX_BITS-1						:= 0;
	        i_DATA       		: IN STD_LOGIC_VECTOR(g_BUFFER_ROWS*g_PIX_BITS-1 DOWNTO 0)	:= (OTHERS => '0');
	        i_ROW_SHIFT  		: IN INTEGER RANGE 0 TO 2*g_BUFFER_ROWS						:= 0;
	        o_DET_COL    		: OUT INTEGER RANGE 0 TO 2*g_BUFFER_ROWS					:= 0;                                                 
	        o_DET_ROW    		: OUT INTEGER RANGE 0 TO 2*g_BUFFER_ROWS					:= 0;
	        o_SUN_POT    		: OUT STD_LOGIC                        						:= '0';
	        o_MARKER_POT 		: OUT STD_LOGIC                        						:= '0';
	        o_VALID      		: OUT STD_LOGIC                        						:= '0'
		);
    END COMPONENT;			 
	
    SUBTYPE t_SHIFT_INT 	IS INTEGER RANGE 0 TO 2*g_BUFFER_ROWS; 										

    SIGNAL r_CLOCK : STD_LOGIC := '1';
    SIGNAL r_RESET : STD_LOGIC := '0';
   										
    SIGNAL r_DATA     	: STD_LOGIC_VECTOR(g_PIX_BITS-1 DOWNTO 0)  		:= (OTHERS => '0');
    SIGNAL r_POS_X    	: STD_LOGIC_VECTOR(g_WIDTH_BITS-1 DOWNTO 0) 	:= (OTHERS => '0');                                             
    SIGNAL r_POS_Y 		: STD_LOGIC_VECTOR(g_HEIGHT_BITS-1 DOWNTO 0) 	:= (OTHERS => '0');
    SIGNAL r_IMG_VALID 	: STD_LOGIC                     				:= '0';
    SIGNAL r_IMG_DONE  	: STD_LOGIC                     				:= '0';

    SIGNAL r_RAM_DATA_WRITE 	: STD_LOGIC_VECTOR(g_PIX_BITS-1 DOWNTO 0)  				:= (OTHERS => '0');
    SIGNAL r_RAM_ADDRESS    	: STD_LOGIC_VECTOR(15 DOWNTO 0) 						:= (OTHERS => '0');
    SIGNAL r_WRITE_EN  			: STD_LOGIC_VECTOR(g_BUFFER_ROWS-1 DOWNTO 0)  			:= (OTHERS => '0');                                                  
    SIGNAL r_RAMS_DATA_READ 	: STD_LOGIC_VECTOR(g_BUFFER_ROWS*g_PIX_BITS-1 DOWNTO 0)	:= (OTHERS => '0');

    SIGNAL r_FIMD_NRST  		: STD_LOGIC 	:= '0';
    SIGNAL r_FIMD_ROW_SHIFT  	: t_SHIFT_INT 	:= 0;
    SIGNAL r_FIMD_DET_COL    	: t_SHIFT_INT 	:= 0;
    SIGNAL r_FIMD_DET_ROW    	: t_SHIFT_INT 	:= 0;
    SIGNAL r_FIMD_SUN_POT    	: STD_LOGIC 	:= '0';
    SIGNAL r_FIMD_MARKER_POT 	: STD_LOGIC 	:= '0';
    SIGNAL r_FIMD_VALID 		: STD_LOGIC 	:= '0';		   
BEGIN
    r_CLOCK <= NOT r_CLOCK AFTER 37.5 ns; -- 26.67 MHz
    r_RESET <= '1' AFTER 30 ns;

    p_TEST : PROCESS (r_CLOCK, r_RESET) IS
        VARIABLE v_STDOUT         	: line;
        VARIABLE v_CURR_RAM_INDEX 	: INTEGER := 0;
        VARIABLE v_INDEX          	: INTEGER := 0;
        VARIABLE v_POS_X          	: INTEGER := 0;
        VARIABLE v_POS_Y          	: INTEGER := 0;

        VARIABLE v_FINISHED    	: STD_LOGIC := '0';
        VARIABLE v_FRAME_START 	: STD_LOGIC := '0';
        VARIABLE v_EXTRA_ROW   	: STD_LOGIC := '0';
        VARIABLE v_MARKER_CNT  	: INTEGER   := 0;
        VARIABLE v_SUN_CNT 		: INTEGER   := 0;
    BEGIN
        IF rising_edge(r_CLOCK) THEN
            IF v_FINISHED = '1' THEN
                write(v_STDOUT, STRING'("Detected "));
                write(v_STDOUT, v_SUN_CNT);
                write(v_STDOUT, STRING'(" sun points."));
                writeline(output, v_STDOUT);

                ASSERT false -- terminate the simulation
                REPORT "Done, all vectors tested..." SEVERITY failure;
            END IF;

            IF r_IMG_VALID = '1' THEN
                v_POS_X := to_integer(unsigned(r_POS_X));
                v_POS_Y := to_integer(unsigned(r_POS_Y));
            END IF;

            IF v_POS_X = g_IMG_WIDTH THEN
                v_FRAME_START 	:= '0';
                v_EXTRA_ROW   	:= '0';
                r_FIMD_NRST 	<= '0';
                v_FINISHED 		:= '1';
            END IF;

            IF v_FRAME_START = '1' THEN
                IF r_IMG_VALID = '0' AND v_EXTRA_ROW = '0' THEN
                    v_POS_X         := 0;
                    v_POS_Y         := v_POS_Y + 1;
                    v_EXTRA_ROW := '1';
                END IF;

                IF v_POS_Y = 7 THEN
                    r_FIMD_NRST <= '1';
                END IF;

                IF r_FIMD_VALID = '1' THEN
                    IF r_FIMD_MARKER_POT = '1' THEN
                        v_MARKER_CNT := v_MARKER_CNT + 1;
                        write(v_STDOUT, STRING'("Detected marker #"));
                        write(v_STDOUT, v_MARKER_CNT);
                        write(v_STDOUT, STRING'(": row="));
                        write(v_STDOUT, (v_POS_Y + r_FIMD_DET_ROW - (g_RADIUS*2+1)));
                        write(v_STDOUT, STRING'(", col="));
                        write(v_STDOUT, (v_POS_X + r_FIMD_DET_COL - (g_RADIUS*2+1) - 1));
                        writeline(output, v_STDOUT);
                    END IF;
                    IF r_FIMD_SUN_POT = '1' THEN
                        v_SUN_CNT := v_SUN_CNT + 1;
                    END IF;
                END IF;
            END IF;

            IF r_IMG_VALID = '1' OR v_EXTRA_ROW = '1' THEN
                v_CURR_RAM_INDEX 	:= v_POS_Y MOD (g_RADIUS*2+1);
                r_FIMD_ROW_SHIFT   	<= v_CURR_RAM_INDEX;
                r_RAM_ADDRESS 		<= STD_LOGIC_VECTOR(to_unsigned(v_POS_X, r_RAM_ADDRESS'length));
            END IF;

            IF r_IMG_VALID = '1' THEN
                v_FRAME_START := '1';
                IF r_IMG_DONE = '0' THEN
                    r_WRITE_EN                 		<= (OTHERS => '0');
                    r_WRITE_EN(v_CURR_RAM_INDEX) 	<= '1';
                    r_RAM_DATA_WRITE                <= r_DATA;
                END IF;
            ELSE
                r_WRITE_EN <= (OTHERS => '0');

                IF v_EXTRA_ROW = '1' THEN
                    v_POS_X := v_POS_X + 1;
                END IF;
            END IF;
        END IF;										   
    END PROCESS;

    e_READER : raw_img_reader GENERIC MAP(g_IMG_PATH, g_IMG_WIDTH, g_IMG_HEIGHT, '0') PORT MAP(r_CLOCK, r_RESET, r_POS_X, r_POS_Y, r_DATA, r_IMG_VALID, r_IMG_DONE);

   	e_FIMD : fimd GENERIC MAP(g_BUFFER_ROWS, g_RADIUS, g_PIX_BITS) PORT MAP(r_CLOCK, r_FIMD_NRST, g_FIMD_TM, g_FIMD_TD, g_FIMD_TS, r_RAMS_DATA_READ, r_FIMD_ROW_SHIFT, r_FIMD_DET_COL, r_FIMD_DET_ROW, r_FIMD_SUN_POT, r_FIMD_MARKER_POT, r_FIMD_VALID);
	
	l_RAMS: FOR k in 0 TO g_BUFFER_ROWS-1 GENERATE		
    	e_RAM : ram_sim GENERIC MAP(g_IMG_WIDTH, g_PIX_BITS, 16) PORT MAP(r_RAM_ADDRESS, r_CLOCK, r_RAM_DATA_WRITE, r_WRITE_EN(k), r_RAMS_DATA_READ(g_PIX_BITS*(k+1)-1 DOWNTO g_PIX_BITS*k));
	END GENERATE;																													   	
END ARCHITECTURE;