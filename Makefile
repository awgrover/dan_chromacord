ino=$(shell /bin/ls -1 *.ino | head -n 1)
menustart := \/\/ menu made by:
menuend := \/\/ end menu

# ignores default "." otherwise
PHONY : .$(ino).menu
.$(ino).menu : $(ino)
	perl -n -e '/case ('"'"'(.)'"'"'|([0-9]))\s*:\s*\/\/(.+)/i && do {print "Serial.println(F(\"$$2$$3 $$4\"));\n"}' $< > $@

PHONY : menu
menu : .$(ino).menu
	@grep "$(menustart)" $(ino) >/dev/null || (echo 'In $(ino), expected: $(menustart)' ; false)
	@grep "$(menuend)" $(ino) >/dev/null || (echo 'In $(ino), expected: $(menuend)' ; false)
	@ cp $(ino) $(ino).bak
	awk '/$(menustart)/ {print; system("cat $<")}; /$(menuend)/ {print}; /$(menustart)/,/$(menuend)/ {next}; {print}' $(ino).bak > $(ino)
	@echo Edited $(ino)

patches.h : *.patch
	./generate_patches patches.h *.patch

# Launch ide
.PHONY : ide
ide : log lib_dir_link/tlc59116
	arduino `pwd`/*.ino > log/ide.log 2>&1 &

log :  
	mkdir -p log

# link in the external lib (which isn't when developing)
.PHONY : extlib
extlib : lib_dir_link/tlc59116
lib_dir_link/tlc59116 :
	ln -s `realpath ../tlc59116_lib` `realpath lib_dir_link`/tlc59116
	
