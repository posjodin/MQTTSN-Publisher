avrdude -c stk500v2 -p m256rfr2 -P ${PORT} -V -D -U flash:w:bin/avr-rss2/simping.elf
