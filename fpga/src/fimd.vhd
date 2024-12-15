																															   -------------------------------------------------------------------------------
--
-- Title       : fimd
-- Design      : FIMD
-- Author      : vrba.vojtech [at] fel.cvut.cz
-- Company     : Czech Technical University in Prague
--						
-------------------------------------------------------------------------------
--
-- Description : Fast Isolated Marker Detection
--
-------------------------------------------------------------------------------
					   											   
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
USE ieee.numeric_std.ALL;	   	

ENTITY imd IS 	  
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
END ENTITY;											 

ARCHITECTURE rtl OF imd IS	 											
    SUBTYPE t_PIX_INT 		IS INTEGER RANGE 0 TO 2**g_PIX_BITS-1;
    SUBTYPE t_SHIFT_INT 	IS INTEGER RANGE 0 TO 2*g_BUFFER_ROWS;
	SUBTYPE t_FSM_INT		IS INTEGER RANGE -g_RADIUS TO g_RADIUS;
	SUBTYPE t_FSM_DEC_INT	IS INTEGER RANGE -4*g_RADIUS TO 4*g_RADIUS;

    TYPE t_ARR_PIX_INT 	 IS ARRAY(NATURAL RANGE <>) OF t_PIX_INT;	 
    TYPE t_ARR_SHIFT_INT IS ARRAY(NATURAL RANGE <>) OF t_SHIFT_INT;	  
	TYPE t_FSM_STATE IS (STATE_UPDATE, STATE_PYMX, STATE_MYPX, STATE_PXMY, STATE_MXPY, STATE_PYPX, STATE_MYMX, STATE_PXPY, STATE_MXMY, STATE_END);
							 
    CONSTANT c_CENTER_POS 	: t_SHIFT_INT := g_RADIUS; 
	
    SIGNAL r_ROW_SHIFT_PREV : t_SHIFT_INT := 2*g_RADIUS+1;
    SIGNAL r_DET_COL        : t_SHIFT_INT := 0;
    SIGNAL r_DET_ROW        : t_SHIFT_INT := 0;
    SIGNAL r_SUN_POT        : STD_LOGIC   := '0';
    SIGNAL r_MARKER_POT     : STD_LOGIC   := '0';
    SIGNAL r_VALID          : STD_LOGIC   := '0';

    SIGNAL r_CENTER_ARR      : t_ARR_PIX_INT(0 TO 2*g_RADIUS)   := (OTHERS => 0);
    SIGNAL r_MAX_BOUND_ARR   : t_ARR_PIX_INT(0 TO 2*g_RADIUS)   := (OTHERS => 0);
    SIGNAL r_MIN_BOUND_ARR   : t_ARR_PIX_INT(0 TO 2*g_RADIUS)   := (OTHERS => t_PIX_INT'HIGH);
    SIGNAL r_MAX_INT_ARR     : t_ARR_PIX_INT(0 TO 2*g_RADIUS)   := (OTHERS => 0);
    SIGNAL r_MAX_INT_COL_ARR : t_ARR_SHIFT_INT(0 TO 2*g_RADIUS) := (OTHERS => 0);
    SIGNAL r_MAX_INT_ROW_ARR : t_ARR_SHIFT_INT(0 TO 2*g_RADIUS) := (OTHERS => 0);
												  		 
	
    PROCEDURE f_GET_COL_INDEX (
        VARIABLE shift     : IN t_SHIFT_INT;
        CONSTANT column    : IN t_SHIFT_INT;
        VARIABLE col_index : OUT t_SHIFT_INT
    ) IS
        VARIABLE i : t_SHIFT_INT;
    BEGIN
        i := shift;
        IF column > shift THEN
            i := i + 2*g_RADIUS + 1;
        END IF;
        col_index := i - column;
    END PROCEDURE;

    PROCEDURE f_GET_ROW_INDEX (
        SIGNAL shift       : IN t_SHIFT_INT;
        CONSTANT row       : IN t_SHIFT_INT;
        VARIABLE row_index : OUT t_SHIFT_INT
    ) IS
        VARIABLE i : t_SHIFT_INT;
    BEGIN
        i := shift + row;
        IF i >= g_BUFFER_ROWS THEN
            i := i - g_BUFFER_ROWS;
        END IF;
        row_index := i;
    END PROCEDURE; 
							   
	PROCEDURE f_BRESENHAM_NEXT ( 
		VARIABLE s	: INOUT t_FSM_STATE;
		VARIABLE P	: INOUT t_FSM_DEC_INT;
		VARIABLE x	: INOUT t_FSM_INT;	
		VARIABLE y	: INOUT t_FSM_INT;
		VARIABLE r	: OUT t_SHIFT_INT;
		VARIABLE c	: OUT t_SHIFT_INT;
		VARIABLE i	: OUT STD_LOGIC
	) IS						   
    BEGIN
        IF s = STATE_UPDATE THEN
			x := x + 1;
			IF P < 0 THEN
				P := P + 4 * x + 6;
			ELSE   
				y := y - 1;
				P := P + 4 * (x - y) + 10;
			END IF;
			IF x <= y THEN
				s := STATE_PYMX;
			ELSE   
				s := STATE_END;
			END IF;
		END IF;
						 
		IF s = STATE_PYMX THEN 
			s	:= STATE_MYPX;
			c 	:= g_RADIUS + y;	
			r 	:= g_RADIUS - x; 
			i 	:= '1';
		ELSIF s = STATE_MYPX THEN	 
			IF x < y THEN
				s := STATE_PXMY;
			ELSIF x > 0 THEN
				s := STATE_PYPX;
			ELSE
				s := STATE_UPDATE;
			END IF;
			c 	:= g_RADIUS - y;	
			r 	:= g_RADIUS + x; 
			i 	:= '1';   
		ELSIF s = STATE_PXMY THEN		
			s 	:= STATE_MXPY;
			c 	:= g_RADIUS + x;	
			r 	:= g_RADIUS - y; 
			i 	:= '0';   
		ELSIF s = STATE_MXPY THEN
			IF x > 0 THEN
				s := STATE_PYPX;
			ELSE
				s := STATE_UPDATE;
			END IF;
			c 	:= g_RADIUS - x;	
			r 	:= g_RADIUS + y;
			i 	:= '0';  	
		ELSIF s = STATE_PYPX THEN
			s 	:= STATE_MYMX;
			c 	:= g_RADIUS + y;	
			r 	:= g_RADIUS + x; 
			i 	:= '1';   
		ELSIF s = STATE_MYMX THEN
			IF x < y THEN
				s := STATE_PXPY;
			ELSE
				s := STATE_UPDATE;
			END IF;
			c 	:= g_RADIUS - y;	
			r 	:= g_RADIUS - x; 
			i 	:= '1'; 
		ELSIF s = STATE_PXPY THEN
			s 	:= STATE_MXMY;
			c 	:= g_RADIUS + x;	
			r 	:= g_RADIUS + y;
			i 	:= '0';  	
		ELSIF s = STATE_MXMY THEN	
			s 	:= STATE_UPDATE;
			c 	:= g_RADIUS - x;	
			r 	:= g_RADIUS - y; 
			i 	:= '0';  
		ELSE
			s 	:= STATE_END;
			c 	:= 0;	
			r 	:= 0;	
			i 	:= '0';     
		END IF;
    END PROCEDURE;
									   
BEGIN
	p_FAST : PROCESS (i_CLOCK, i_NRST) IS														
        VARIABLE v_COL_SHIFT   : t_SHIFT_INT := 0;
        VARIABLE v_ROW_INDEX   : t_SHIFT_INT := 0;
        VARIABLE v_COL_INDEX   : t_SHIFT_INT := 0;
        VARIABLE v_PROCESS_RES : STD_LOGIC   := '0';

        VARIABLE v_PIXELS : t_ARR_PIX_INT(0 TO 2*g_RADIUS) := (OTHERS => 0);

        VARIABLE v_CENTER_VAL		: t_PIX_INT   := 0;
        VARIABLE v_MAX_BOUND_VAL	: t_PIX_INT   := 0;
        VARIABLE v_MIN_BOUND_VAL	: t_PIX_INT   := t_PIX_INT'HIGH;
        --VARIABLE v_MAX_INT_VAL   	: t_PIX_INT   := 0;
        --VARIABLE v_MAX_INT_COL   	: t_SHIFT_INT := 0;
        --VARIABLE v_MAX_INT_ROW		: t_SHIFT_INT := 0;	  
		
		VARIABLE v_FSM_STATE	: t_FSM_STATE		:= STATE_END; 		                    
		VARIABLE v_FSM_DEC		: t_FSM_DEC_INT		:= 0;    
		VARIABLE v_FSM_X		: t_FSM_INT			:= 0;
		VARIABLE v_FSM_Y		: t_FSM_INT			:= 0;
		VARIABLE v_FSM_INV		: STD_LOGIC			:= '0'; 	  
		
        VARIABLE v_ROW   	: t_SHIFT_INT 	:= 0;
        VARIABLE v_COL   	: t_SHIFT_INT 	:= 0;  	
		VARIABLE v_INT_ROW 	: t_SHIFT_INT	:= 0;  
    BEGIN
        IF i_NRST = '0' THEN
            r_ROW_SHIFT_PREV <= 2*g_RADIUS + 1;
            r_VALID          <= '0';
            v_COL_SHIFT   := 0;
            v_PROCESS_RES := '0';			  
        ELSE
            IF rising_edge(i_CLOCK) THEN 
                -- col shift reset on new row
                IF r_ROW_SHIFT_PREV /= i_ROW_SHIFT THEN
                    v_PROCESS_RES := '0';
                    v_COL_SHIFT   := 0;
                END IF;
                r_ROW_SHIFT_PREV <= i_ROW_SHIFT;

                -- get v_PIXELS values
                FOR r IN 0 TO 2*g_RADIUS LOOP
                    f_GET_ROW_INDEX(i_ROW_SHIFT, r, v_ROW_INDEX);
                    v_PIXELS(r) := TO_INTEGER(UNSIGNED(i_DATA(g_PIX_BITS * (v_ROW_INDEX+1) - 1 DOWNTO g_PIX_BITS * v_ROW_INDEX)));
                END LOOP;
				
				-- reset Bresenham circle	  
				v_FSM_STATE		:= STATE_PYMX;
				v_FSM_DEC		:= 3 - (2 * g_RADIUS);
				v_FSM_X			:= 0;
				v_FSM_Y			:= g_RADIUS;	   
							   
				-- get first Bresenham point
				f_BRESENHAM_NEXT(v_FSM_STATE, v_FSM_DEC, v_FSM_X, v_FSM_Y, v_ROW, v_COL, v_FSM_INV);
				
				-- process all columns
				WHILE v_FSM_STATE /= STATE_END LOOP	   	
					-- get shifted v_COL_INDEX for the column
	                f_GET_COL_INDEX(v_COL_SHIFT, v_COL, v_COL_INDEX);
	
	                -- load signals to variables
	                v_MAX_BOUND_VAL := r_MAX_BOUND_ARR(v_COL_INDEX);
	                v_MIN_BOUND_VAL := r_MIN_BOUND_ARR(v_COL_INDEX);
	                --v_MAX_INT_VAL   := r_MAX_INT_ARR(v_COL_INDEX);
	                --v_MAX_INT_COL   := r_MAX_INT_COL_ARR(v_COL_INDEX);
	                --v_MAX_INT_ROW   := r_MAX_INT_ROW_ARR(v_COL_INDEX);
				    						
	                -- store boundary comparison results
	                IF v_PIXELS(v_ROW) > v_MAX_BOUND_VAL THEN
	                    v_MAX_BOUND_VAL := v_PIXELS(v_ROW);
	                END IF;
	                IF v_PIXELS(v_ROW) < v_MIN_BOUND_VAL THEN
	                    v_MIN_BOUND_VAL := v_PIXELS(v_ROW);
	                END IF;							 	   	   
							  
					-- store central pixel value
					IF v_COL = g_RADIUS THEN
                		r_CENTER_ARR(v_COL_INDEX) <= v_PIXELS(g_RADIUS);
					END IF;		  
					
					-- optional: store interior comparison results	   
					--FOR y IN 0 TO g_RADIUS LOOP 
					--	IF (v_FSM_INV = '0' AND y < v_FSM_Y) OR (v_FSM_INV = '1' AND y < v_FSM_X) THEN		
					--		IF v_ROW < g_RADIUS THEN
					--			v_INT_ROW := g_RADIUS - y;
					--		ELSE
					--			v_INT_ROW := g_RADIUS + y; 	
					--		END IF;
					--		IF v_PIXELS(v_INT_ROW) > v_MAX_INT_VAL THEN
			        --          v_MAX_INT_VAL := v_PIXELS(v_INT_ROW);
			        --          v_MAX_INT_COL := v_COL;
			        --          v_MAX_INT_ROW := v_INT_ROW;	 
			        --      END IF;
					--	END IF;
					--END LOOP;
					
	                -- store column results to signal arrays
	                r_MAX_BOUND_ARR(v_COL_INDEX)   <= v_MAX_BOUND_VAL;
	                r_MIN_BOUND_ARR(v_COL_INDEX)   <= v_MIN_BOUND_VAL;
	                --r_MAX_INT_ARR(v_COL_INDEX)     <= v_MAX_INT_VAL;
	                --r_MAX_INT_COL_ARR(v_COL_INDEX) <= v_MAX_INT_COL;
	                --r_MAX_INT_ROW_ARR(v_COL_INDEX) <= v_MAX_INT_ROW;	
					
					-- get next Bresenham point
					f_BRESENHAM_NEXT(v_FSM_STATE, v_FSM_DEC, v_FSM_X, v_FSM_Y, v_ROW, v_COL, v_FSM_INV);	  				   
				END LOOP;
													 
				
				-- increase column shift MOD g_SIZE
                v_COL_SHIFT := v_COL_SHIFT + 1;
                IF v_COL_SHIFT = 2*g_RADIUS + 1 THEN
                    v_COL_SHIFT   := 0;
                    v_PROCESS_RES := '1'; -- after one full cycle
                END IF;

                -- process results if arrays ready
                IF v_PROCESS_RES = '1' THEN
                    -- load signals to variables
                    v_CENTER_VAL    := r_CENTER_ARR(v_COL_SHIFT);
                    v_MAX_BOUND_VAL := r_MAX_BOUND_ARR(v_COL_SHIFT);
                    v_MIN_BOUND_VAL := r_MIN_BOUND_ARR(v_COL_SHIFT);
                    --v_MAX_INT_COL   := r_MAX_INT_COL_ARR(v_COL_SHIFT);
                    --v_MAX_INT_ROW   := r_MAX_INT_ROW_ARR(v_COL_SHIFT); 	
					
                    -- process FAST algorithm
                    IF v_CENTER_VAL > i_THRESHOLD THEN
                        IF v_MAX_BOUND_VAL <= v_CENTER_VAL AND (v_CENTER_VAL - v_MAX_BOUND_VAL) >= i_THRESHOLD_DIFF THEN
                            r_MARKER_POT       <= '1';
                            r_SUN_POT          <= '0';
                            r_DET_COL          <= c_CENTER_POS; --v_MAX_INT_COL;
                            r_DET_ROW          <= c_CENTER_POS; --v_MAX_INT_ROW;
                            r_VALID            <= '1';
                        ELSE
                            r_MARKER_POT <= '0';

                            IF v_CENTER_VAL > i_THRESHOLD_SUN AND (v_MIN_BOUND_VAL >= v_CENTER_VAL OR (v_CENTER_VAL - v_MIN_BOUND_VAL) <= i_THRESHOLD_DIFF) THEN

                                r_DET_COL <= c_CENTER_POS;
                                r_DET_ROW <= c_CENTER_POS;
                                r_SUN_POT <= '1';
                                r_VALID   <= '1';
                            ELSE
                                r_VALID <= '0';
                            END IF;
                        END IF;
                    ELSE
                        r_MARKER_POT <= '0';
                        r_SUN_POT    <= '0';
                        r_VALID      <= '0';
                    END IF;

                    -- reset arrays
                    r_MAX_BOUND_ARR(v_COL_SHIFT)   <= 0;
                    r_MIN_BOUND_ARR(v_COL_SHIFT)   <= t_PIX_INT'HIGH;
                    --r_MAX_INT_ARR(v_COL_SHIFT)     <= 0;
                    --r_MAX_INT_COL_ARR(v_COL_SHIFT) <= 0;
                    --r_MAX_INT_ROW_ARR(v_COL_SHIFT) <= 0;
                END IF;
            END IF;
        END IF;
    END PROCESS;		 
	
    o_DET_COL    <= r_DET_COL;
    o_DET_ROW    <= r_DET_ROW;
    o_SUN_POT    <= r_SUN_POT;
    o_MARKER_POT <= r_MARKER_POT;
    o_VALID      <= r_VALID;
END ARCHITECTURE;
