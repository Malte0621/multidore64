rm -rf build
# Check if the build directory doesn't exist
if [ ! -d ../build ]; then
    mkdir build >/dev/null
fi
cd src
# Loop through all .c files in the current directory
for file in *.c
do
    # Compile each .c file into a .s file
    cc65 -O -o ../build/${file%.*}.s -t c64 $file
done

for file in *.s
do
    # Copy each .s file into the build directory
    cp $file ../build
done

for file in *.bin
do
    # Copy each .bin file into the build directory
    cp $file ../build
done

# check if ANY .cfg file exists in the current directory
if [ -f *.cfg ]; then
    for file in *.cfg
    do
        # Copy each .cfg file into the build directory
        cp $file ../build
    done
fi

cd ../build

# Loop through all .s files in the build directory
for file in *.s
do
    # Assemble each .s file into a .o file
    ca65 -o ${file%.*}.o $file
done

ca65 ./main.s
# Check if the dist directory doesn't exist
if [ ! -d ../dist ]; then
    # Create the dist directory
    mkdir ../dist >/dev/null
fi

# Link all .o files into a .prg file
# ld65 -o ../dist/main -t c64 main.o c64.lib
# Loop through all .o files in the build directory and add them to the link command
obj_files=""
for file in *.o
do
    obj_files="$obj_files $file"
done

# Check if c64.cfg exists in the current directory
if [ -f c64.cfg ]; then
    # Link all .o files into a .prg file
    ld65 -C c64.cfg -o ../dist/main.d64 $obj_files c64.lib
else
    # Link all .o files into a .prg file
    ld65 -o ../dist/main.d64 -t c64 $obj_files c64.lib
fi

echo Done.