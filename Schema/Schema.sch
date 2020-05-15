EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Text GLabel 4250 3400 2    50   Input ~ 0
Vcc
Text GLabel 6950 4550 0    50   Input ~ 0
Vcc
Wire Wire Line
	5050 2950 5300 2950
$Comp
L power:GND #PWR?
U 1 1 5EB540C1
P 5050 2950
F 0 "#PWR?" H 5050 2700 50  0001 C CNN
F 1 "GND" H 5055 2777 50  0000 C CNN
F 2 "" H 5050 2950 50  0001 C CNN
F 3 "" H 5050 2950 50  0001 C CNN
	1    5050 2950
	0    1    1    0   
$EndComp
Text GLabel 5300 2850 0    50   Input ~ 0
Vcc
Text GLabel 5300 2750 0    50   Input ~ 0
SERVO_PIN
$Comp
L power:GND #PWR?
U 1 1 5EB50F09
P 6950 4100
F 0 "#PWR?" H 6950 3850 50  0001 C CNN
F 1 "GND" H 6955 3927 50  0000 C CNN
F 2 "" H 6950 4100 50  0001 C CNN
F 3 "" H 6950 4100 50  0001 C CNN
	1    6950 4100
	0    1    1    0   
$EndComp
Text GLabel 6950 4250 0    50   Input ~ 0
Vcc
Text GLabel 6950 4400 0    50   Input ~ 0
IR1_PIN
Text GLabel 5650 3700 2    50   Input ~ 0
BUTTON_PIN
Text GLabel 5250 3900 0    50   Input ~ 0
Vcc
$Comp
L Switch:SW_Push_Dual SW?
U 1 1 5EB402BB
P 5450 3700
F 0 "SW?" H 5450 3985 50  0000 C CNN
F 1 "SW_Push_Dual" H 5450 3894 50  0000 C CNN
F 2 "" H 5450 3900 50  0001 C CNN
F 3 "~" H 5450 3900 50  0001 C CNN
	1    5450 3700
	1    0    0    -1  
$EndComp
$Comp
L KY-032:KY-032 IR_Sensor1
U 1 1 5EB45445
P 7300 3900
F 0 "IR_Sensor1" H 7778 3621 50  0000 L CNN
F 1 "KY-032" H 7778 3530 50  0000 L CNN
F 2 "" H 7450 3950 50  0001 C CNN
F 3 "" H 7450 3950 50  0001 C CNN
	1    7300 3900
	1    0    0    -1  
$EndComp
$Comp
L Motor:Motor_Servo M?
U 1 1 5EB482A1
P 5600 2850
F 0 "M?" H 5932 2915 50  0000 L CNN
F 1 "MicroServo" H 5932 2824 50  0000 L CNN
F 2 "" H 5600 2660 50  0001 C CNN
F 3 "http://forums.parallax.com/uploads/attachments/46831/74481.png" H 5600 2660 50  0001 C CNN
	1    5600 2850
	1    0    0    -1  
$EndComp
Wire Wire Line
	5650 4200 5650 3900
Wire Wire Line
	5650 4500 5650 4750
$Comp
L power:GND #PWR?
U 1 1 5EA06C5D
P 5650 4750
F 0 "#PWR?" H 5650 4500 50  0001 C CNN
F 1 "GND" H 5655 4577 50  0000 C CNN
F 2 "" H 5650 4750 50  0001 C CNN
F 3 "" H 5650 4750 50  0001 C CNN
	1    5650 4750
	1    0    0    -1  
$EndComp
$Comp
L Device:R R1
U 1 1 5EA0485A
P 5650 4350
F 0 "R1" V 5443 4350 50  0000 C CNN
F 1 "20k" V 5534 4350 50  0000 C CNN
F 2 "" V 5580 4350 50  0001 C CNN
F 3 "~" H 5650 4350 50  0001 C CNN
	1    5650 4350
	1    0    0    -1  
$EndComp
Text Notes 7350 7500 0    50   ~ 0
Automatic Pet Feeder
Text GLabel 4250 4000 2    50   Input ~ 0
CAP_PIN
Text GLabel 4250 3900 2    50   Input ~ 0
CAP_T0_PIN
$Comp
L Device:R R_CAP
U 1 1 5EBEC65D
P 7250 2550
F 0 "R_CAP" H 7320 2596 50  0000 L CNN
F 1 "1M" H 7320 2505 50  0000 L CNN
F 2 "" V 7180 2550 50  0001 C CNN
F 3 "~" H 7250 2550 50  0001 C CNN
	1    7250 2550
	1    0    0    -1  
$EndComp
Text GLabel 7250 2400 1    50   Input ~ 0
CAP_T0_PIN
Text GLabel 7250 3000 3    50   Input ~ 0
CAP_PIN
$Comp
L capsense:aluminium_foil cap
U 1 1 5EBEFB27
P 8200 2300
F 0 "cap" H 8039 2235 50  0000 C CNN
F 1 "aluminium_foil" H 8039 2144 50  0000 C CNN
F 2 "" H 8200 2250 50  0001 C CNN
F 3 "" H 8200 2250 50  0001 C CNN
	1    8200 2300
	1    0    0    -1  
$EndComp
Wire Wire Line
	7250 2700 7250 2850
Wire Wire Line
	7750 2850 7250 2850
Connection ~ 7250 2850
Wire Wire Line
	7250 2850 7250 3000
Text GLabel 3150 4000 0    50   Input ~ 0
SERVO_PIN
Text GLabel 3150 3900 0    50   Input ~ 0
IR1_PIN
Text GLabel 3150 4100 0    50   Input ~ 0
BUTTON_PIN
$Comp
L ttgo_esp322:TTGO_ESP32_TDISPLAY_V1.1 TTGO?
U 1 1 5EBF4DCE
P 3350 4550
F 0 "TTGO?" H 3700 5987 60  0000 C CNN
F 1 "TTGO_ESP32_TDISPLAY_V1.1" H 3700 5881 60  0000 C CNN
F 2 "" H 3350 4550 60  0001 C CNN
F 3 "" H 3350 4550 60  0001 C CNN
	1    3350 4550
	1    0    0    -1  
$EndComp
$EndSCHEMATC
