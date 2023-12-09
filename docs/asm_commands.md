# ASMPDC instruction list

## ADD
**ADD var1,num** Suma num a var1 

**ADD var1,var2** Suma var2 a var1

**ADD var1,[num]** Suma valor apuntado a dirección num a var1

**ADD var1,[var2]** Suma valor apuntado a dirección var2 a var1

## AND
**AND var1,num** Realiza lógica AND entre var1 y num y almacena en var1

**AND var1,var2** Realiza lógica AND entre var1 y var2 y almacena en var1 

**AND var1,[num]** Realiza lógica AND entre var1 y *num y almacena en var1

**AND var1,[var2]** Realiza lógica AND entre var1 y *var2 y almacena en var1

(*)num=puntero a dirección

(*)var2=variable puntero a dirección 

## BCLR
**BCLR var1,0-31** Setea en 0 según posición de bit 0-31 en var1

**BCLR var1,var2** Setea en 0 según var2=(0-31) en var1 

## BSET
**BSET var1,0-31** Setea en 1 según posición de bit 0-31 en var1

**BSET var1,var2** Setea en 1 según var2=(0-31) en var1 

## Declaración de variables
**name BYTE num** name= Nombre variable; num= número entero

**name BYTE 'char'** char= Caracter ASCII 

**name BYTE "string"** string= Cadena de caracteres ASCII

**name BYTE [val]** val= Tamaño buffer

## CALL
**CALL etiqueta** Salta a etiqueta. Requiere de RET para retornar.

## DEC
**DEC var1** Decrementa en -1 a var1

## DIV 
Divide (Ver ADD) 

## ICL
**ICL Nombre.INC** Incluye archivo .INC en programa fuente. Debe iniciarse en el encabezado. 

## INC
**INC var1** Incrementa en 1 a var1

## INP
**INP var1,num** Lee en dirección num(DRAM) y almacena el valor en var1

**INP var1,var2** Lee en dirección var2(DRAM) y almacena el valor en var1

## JE
**JE var1,num** Avanza dos lineas si var1 es igual a num

**JE var1,var2** Avanza dos lineas si var1 es igual a var2

**JE var1,[num]** Avanza dos lineas si var1 es igual a valor apuntado por num

**JE var1,[var2]** Avanza dos lineas si var1 es igual a valor apuntado por var2 (**No implementado**)

## JG
Condición mayor (Ver JE) 

## JGE
Condición mayor o igual (Ver JE)

## JL
Condición menor (Ver JE)

## JLE 
Condición menor o igual (Ver JE)

## JMP
**JMP etiqueta** Salta a etiqueta sin condición 

## JNE 
Condición distinto (Ver JE)

## LEA
**LEA var1,num** Almacena en dirección var1(Cache) el valor num

**LEA var1,var2** Almacena en dirección var1(Cache) el valor var2

**LEA var1,[num]** Almacena en dirección var1(Cache) el valor puntero num 

**LEA var1,[var2]** Almacena en dirección var1(Cache) el valor puntero var2 

## LOXY
**LOXY x,num** posiciona cursor en coordenadas X e num

**LOXY x,y** posiciona cursor en coordenadas X e Y 

## LSL 
**LSL var1,num** Desplaza bits a la izquierda tantos como num

**LSL var1,var2** Desplaza bits a la izquierda tantos como valor de var2

## LSR
**LSR var1,num** Desplaza bits a la derecha tantos como num

**LSR var1,var2** Desplaza bits a la derecha tantos como valor de var2

## MOV
**MOV var1,num** Copia el valor de num en var1

**MOV var1,var2** Copia el valor de var2 en var1 

**MOV var1,[num]** Copia el valor puntero num en var1 

**MOV var1,[var2]** Copia el valor puntero var2 en var1

## MUL
Multiplica (Ver ADD)

## NOT
Lógica NOT (Ver AND)

## OR
Lógica OR (Ver AND)

## OUT
**OUT var1,numAlmacena** en dirección var1(DRAM) el valor num

**OUT var1,var2** Almacena en dirección var1(DRAM) el valor de var2

## PRT
**PRT num** Imprime en pantalla el valor número o constante

**PRT var1** Imprime en pantalla variable var1(numerica/char/string) 

## PRTA
**PRTA num** Imprime en pantalla el valor ASCII de num

**PRTA var1** Imprime en pantalla el valor ASCII de var1 

## REG
**REG var1,num** Lee dirección num(Cache) y almacena el valor en var1

**REG var1,var2** Lee dirección var2(Cache) y almacena el valor en var1

## RET
**RET** Retorna del llamado de etiqueta. Requiere de CALL. 

## ROUT
**ROUT MSB,LSB** Llamado a rutina de firmware, solo argumentos numéricos y retorna automaticamente.

## SUB
Resta (Ver ADD) 

## XOR
Lógica XOR (Ver AND)