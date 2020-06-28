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
read
write
readdir
init
utimens

IN BLK DEV:
Nothing working

# Todo
IN RAM:
triple indirect offset

... and so on
IN BLK DEV:
Everything
