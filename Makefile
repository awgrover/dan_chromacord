ino=$(shell /bin/ls -1 *.ino | head -n 1)
$(ino).menu : $(ino)
	perl -n -e '/case ('"'"'([a-z0-9])'"'"'|([0-9]))\s*:\s*\/\/(.+)/i && do {print "Serial.println(F(\"$$2$$3 $$4\"));\n"}' $< > $@
