# NBT viewer

This tool allows you to inspect the contents of Minecraft NBT files. You can
also edit and save them back.

## How to build

To download and build it for Linux, you must navigate to your target folder,
open up the terminal, and type:

```bash
git clone ...
cd nbt_viewer
make
```

## How to use

To use it, type the command:

```bash
./nbt_viewer < input_file
```

this will read the contents of the input file and print them to the standard
output. If you want to output it to a file, then use: 

```bash
./nbt_viewer < input_file > output_file
```

By default, the input file is treated as binary NBT, and the program outputs
text NBT. If you want to change that, you can use the options `-p` and `-c`.

The output of this program can be fed back in as input, so you can save an NBT
file as text, inspect, modify it, and then run the program to turn it back to
binary NBT.
