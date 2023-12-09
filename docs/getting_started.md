# Getting started guide

- [Launch the emulator by opening this link](https://pdc32.github.io/pdc32-emu/)

After a few seconds, the emulator boots and the `CMD:` command line is ready to accept commands. The `F3` key can be used to recall the previously executed commands, similar to the "up-arrow" in a modern environment.

## Filesystem
The operating system supports storing files in external memory ("cards"). By default the emulator comes with a preformatted card with some example files.

### Listing files 
Use the `dir` command to list all the files of the unit.

### Creating files 
Use the `make <filename>` command to create a file in the unit. All filenames must be **EXACTLY** 8 characters long, with a 3 letter extension. For example, execute `make testing0.txt`.

### Editing files
Use the `edit <filename>` command to load the built-in file editor. For example, execute `edit testing0.txt`.

The editor supports the following keys: 
- Move ahead a few spaces using `TAB`
- Move back a few spaces using `Shift` and then `TAB`
- `PgUp` and `PgDn` to move across pages
- `F2` selects lines, then `F4` copies them, `F6` deletes them
- `Insert` adds a line
- `Delete` erases a line
- `Esc`, then `F10` saves a file
- `Esc` then `N` exits without saving

### Example program
Go ahead and write the following example code:
```
ini: 
x BYTE 20
y BYTE 12 
txt BYTE "PDC32"
loop:
LOXY x,y
PRT txt
INC x
DEC y
JG x,30
JMP loop
```

### Compile and execute
Press `Esc`, then `F9`. A few seconds later it will create a `testing0.exe` executable file, with the compiled version of the code.

To run it, go back to the console pressing `Esc` and then just type `testing0`.

## Formatting the card
If you need to delete all existing files in the card, you can enter the `format` command, then press `Y`. After a few seconds the memory card should be empty and ready to use.

The `label [something]` command can be used to give the unit a certain name.

## BIOS
The computer has a very simple BIOS that lets you modify configuration parameters.
To enter the BIOS, press F12 during boot, or enter the `biosset` command.

You can browse the different parameters using the arrow keys, and then the `+` and `-` key of the _numeric keypad_ to change values.

- To save the contents, press `F10` and then `Y`
- To restore the defaults (_Factory Reset_) press`F11` and then `Y`
- To exit without saving, press `Ctrl`, `Alt` and `Enter`, then `Y`. This will shutdown the system which then needs to be powered on using the power button on the top right.
