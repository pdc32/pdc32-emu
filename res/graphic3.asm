l():
C8 BYTE 65337
C9 BYTE 65336
C11 BYTE 65338
C14 BYTE 65339

col_line# BYTE 7
line_xi# BYTE 120
divn# BYTE 0
line_xf# BYTE 380
line_yi# BYTE 27
line_yf# BYTE 100
divs# BYTE 0
var_xi# BYTE 0
var_xf# BYTE 0
var_yi# BYTE 0
var_yf# BYTE 0
deltay# BYTE 0
deltax# BYTE 0
signed# BYTE 0
flag# BYTE 0
incr# BYTE 0
calc_aux# BYTE 0
b_const# BYTE 0
aux_0# BYTE 0

line(): 
MOV var_xi#,line_xi#
MOV var_xf#,line_xf# 
MOV var_yi#,line_yi#

line_xy(): 
MOV var_yf#,line_yf#
LEA C11,col_line#
ROUT 77,154 
MOV deltay#,var_yf#
SUB deltay#,var_yi# 
MOV deltax#,var_xf#   
SUB deltax#,var_xi# 
MOV divn#,deltax# 
MOV divs#,deltay#
CALL check
JG divn#,divs#
JMP draw_v0()

draw_l0(): 
MOV incr#,var_xi#
CALL const_b()

draw_l(): 
MOV calc_aux#,deltay#
MUL calc_aux#,incr#
MOV divn#,calc_aux#
MOV divs#,deltax#
CALL div_signed()   
MOV calc_aux#,divn#
ADD calc_aux#,b_const#
MUL calc_aux#,640
ADD calc_aux#,incr#
LEA C14,calc_aux#
ROUT 77,172 ;ROUT 77,140;
INC incr#
JG incr#,var_xf# 
JMP draw_l()
JMP endline()


draw_v0(): 
MOV incr#,var_yi#
CALL const_b()

draw_v(): 
MOV calc_aux#,incr#
SUB calc_aux#,b_const#
MUL calc_aux#,deltax# 
MOV divn#,calc_aux#
MOV divs#,deltay#
CALL div_signed() 
MOV aux_0#,divn# 
MOV calc_aux#,incr#
MUL calc_aux#,640
ADD calc_aux#,aux_0#
LEA C14,calc_aux#
ROUT 77,172 ;ROUT 77,140;
INC incr#
JG incr#,var_yf#
JMP draw_v()
JMP endline()

const_b(): 
MOV b_const#,deltay#
MUL b_const#,var_xi# 
MOV divn#,b_const#       
MOV divs#,deltax#
CALL div_signed() 
MOV b_const#,divn# 
MOV aux_0#,var_yi#
SUB aux_0#,b_const# 
MOV b_const#,aux_0#
RET

div_signed(): 
MOV signed#,0
CALL check
DIV divn#,divs#
JNE signed#,1
CALL cmpmnt()
RET 

check:
JL divn#, 2147483648
CALL cor_divn()
JL divs#,2147483648
CALL cor_divs()
RET
    
cor_divn():
XOR divn#,4294967295
INC divn#
INC signed#
RET

cor_divs():
XOR divs#,4294967295
INC divs#
INC signed#
RET

cmpmnt():
XOR divn#,4294967295
INC divn#
RET 

endline():
RET

config_vga(): 
MOV C8,65337 
MOV C9,65336 
MOV C11,65338 
MOV C14,65339
RET

graphic_vga():
LEA C9,0
ROUT 77,147

vga_on0#:
ROUT 87,232 ; p57E8 ;
MOV flag#,[65387]
AND flag#,12288
JE flag#,0
JMP vga_on0#
RET

refresh_vga():
LEA C9,1
ROUT 77,147

vga_on1#: 
ROUT 87,232 ; p57E8 ;
MOV flag#,[65387]
AND flag#,12288
JE flag#,4096
JMP vga_on1# 
RET
    