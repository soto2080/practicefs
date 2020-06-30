# Practicefs

It's an userspace filesystem that mimic the legend EXT2 filesystem using [libfuse](https://github.com/libfuse/libfuse)

A project of NCUCSIE CE6154-System Component Design for Emerging Memory and Storage Technologies.

# Working
IN RAM:
getattr
mknod
mkdir
unlink
rmdir
read(partially works)
write(partially works)
readdir
init
utimens

IN BLK DEV(ONLY in block branch now):
getattr
mknod
mkdir
unlink
rmdir
read(partially works)
write(partially works)
readdir
init
utimens

# Todo
IN RAM:
triple indirect offset
read with offset
write with offset
... and so on

IN BLK DEV:
revise the disk layout
more smart space utilization
triple indirect offset
read with offset
write with offset
... and so on
