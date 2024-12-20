# FIMD-FPGA: VHDL implementation of the Fast Isolated Marker Detection.

This directory contains the VHDL implementation of the Fast Isolated Marker Detection (FIMD) for low-end FPGA devices. It contains a Active-HDL project with a testbench to run a simulation of the FIMD on a sample dataset frame. Tested using Active-HDL 15 SE. The implementation was also synthetised on Intel Cyclone V SE FPGA.

Testbench simulation output for a dataset sample `1619240573769481609.bin`:
```txt 
#  Simulation has been initialized
run
# KERNEL: Detected marker #1: row=194, col=266
# KERNEL: Detected marker #2: row=315, col=272
# KERNEL: Detected marker #3: row=315, col=273
# KERNEL: Detected marker #4: row=316, col=271
# KERNEL: Detected marker #5: row=316, col=272
# KERNEL: Detected marker #6: row=316, col=273
# KERNEL: Detected marker #7: row=316, col=274
# KERNEL: Detected marker #8: row=317, col=256
# KERNEL: Detected marker #9: row=317, col=257
# KERNEL: Detected marker #10: row=317, col=258
# KERNEL: Detected marker #11: row=317, col=271
# KERNEL: Detected marker #12: row=317, col=272
# KERNEL: Detected marker #13: row=317, col=273
# KERNEL: Detected marker #14: row=317, col=274
# KERNEL: Detected marker #15: row=318, col=256
# KERNEL: Detected marker #16: row=318, col=257
# KERNEL: Detected marker #17: row=318, col=258
# KERNEL: Detected marker #18: row=318, col=272
# KERNEL: Detected marker #19: row=318, col=273
# KERNEL: Detected marker #20: row=319, col=256
# KERNEL: Detected marker #21: row=319, col=257
# KERNEL: Detected marker #22: row=319, col=258
# KERNEL: Detected marker #23: row=324, col=283
# KERNEL: Detected marker #24: row=324, col=284
# KERNEL: Detected marker #25: row=325, col=282
# KERNEL: Detected marker #26: row=325, col=283
# KERNEL: Detected marker #27: row=325, col=284
# KERNEL: Detected marker #28: row=326, col=282
# KERNEL: Detected marker #29: row=326, col=283
# KERNEL: Detected marker #30: row=326, col=284
# KERNEL: Detected marker #31: row=353, col=221
# KERNEL: Detected marker #32: row=353, col=222
# KERNEL: Detected marker #33: row=354, col=220
# KERNEL: Detected marker #34: row=354, col=221
# KERNEL: Detected marker #35: row=354, col=222
# KERNEL: Detected marker #36: row=355, col=221
# KERNEL: Detected marker #37: row=357, col=228
# KERNEL: Detected marker #38: row=357, col=229
# KERNEL: Detected marker #39: row=358, col=228
# KERNEL: Detected marker #40: row=358, col=229
# KERNEL: Detected marker #41: row=381, col=520
# KERNEL: Detected marker #42: row=382, col=520
# KERNEL: Detected marker #43: row=382, col=523
# KERNEL: Detected marker #44: row=384, col=515
# KERNEL: Detected marker #45: row=407, col=411
# KERNEL: Detected marker #46: row=407, col=412
# KERNEL: Detected marker #47: row=414, col=378
# KERNEL: Detected marker #48: row=414, col=379
# KERNEL: Detected marker #49: row=415, col=374
# KERNEL: Detected marker #50: row=415, col=378
# KERNEL: Detected marker #51: row=415, col=379
# KERNEL: Detected marker #52: row=415, col=386
# KERNEL: Detected marker #53: row=416, col=374
# KERNEL: Detected marker #54: row=416, col=385
# KERNEL: Detected marker #55: row=416, col=386
# KERNEL: Detected 5614 sun points.
# EXECUTION:: FAILURE: Done, all vectors tested...
# EXECUTION:: Time: 27128550 ns,  Iteration: 0,  Instance: /testbench,  Process: p_TEST.
# KERNEL: Stopped at time 27128550 ns + 0.
```
(Detections represent all pixels that pass segment tests.) 