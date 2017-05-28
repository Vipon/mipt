**fifo_copy** - program for copy via fifo.
* simple_fifo_copy.c - simple version without additional checks.
* smart_fifo_copy.c - more complex version with checks to prevent data corruption when more than one readers or writers.
### example:
    ./fifo_copy w source
    ./fifo_copy r destination
