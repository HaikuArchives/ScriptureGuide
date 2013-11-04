##
## Welcome to the Scripture Guide Makefile
## Scripture Guide Version 0.7.1 -- see INSTALL for installation instructions
##
## Scripture Guide is released under the GNU GPL -- see LICENSE for details
##

##
## Compiler and Compiler options:
## (These options work for me. Feel free to optimize them, 
##  as I am not into gcc commandline science ;-))
##
GCC= g++
#GCC_OPT= -g3 -O2 -DHAVE_CONFIG_H -DUSBINARY -Werror
#GCC_OPT= -g -O0 -DHAVE_CONFIG_H -DUSBINARY -Werror
GCC_OPT= -g -O0 -DDEBUG -DHAVE_CONFIG_H -DUSBINARY -Werror

##
## Zeta option
## Set it when using Zeta. This adopts the gui to fit with the Zeta widgets.
##
# VER= ZETA
VER= BEOSR5

##
## Directories needed:
##
SWORD_INC= /boot/home/sword-1.5.7/include
SWORD_LIB= /boot/home/sword-1.5.7/lib /boot/home/sword-1.5.7/lib/.libs/libsword.a
OUT_DIR= ./obj.x86
INSTALL_DIR= /boot/apps/scripture-guide-0.7.1

##
## Compiling and linking starts here ...
##
Scripture\ Guide: $(OUT_DIR)/SwordBackend.o $(OUT_DIR)/LogosSearchWindow.o $(OUT_DIR)/LogosMainWindow.o $(OUT_DIR)/LogosApp.o $(OUT_DIR)/Preferences.o $(OUT_DIR)/FontPanel.o $(OUT_DIR)/ColumnListView.o $(OUT_DIR)/ColumnTypes.o $(OUT_DIR)/Spinner.o $(OUT_DIR)/ColorTools.o
	$(GCC) $(GCC_OPT) -o Scripture\ Guide $(OUT_DIR)/SwordBackend.o $(OUT_DIR)/LogosSearchWindow.o $(OUT_DIR)/LogosMainWindow.o $(OUT_DIR)/LogosApp.o $(OUT_DIR)/Preferences.o $(OUT_DIR)/FontPanel.o $(OUT_DIR)/ColumnListView.o $(OUT_DIR)/ColumnTypes.o $(OUT_DIR)/Spinner.o $(OUT_DIR)/ColorTools.o -L $(SWORD_LIB) -lbe -lz 
	xres -o Scripture\ Guide Scripture\ Guide.rsrc
	mimeset -f Scripture\ Guide
$(OUT_DIR)/SwordBackend.o: SwordBackend.cpp
	$(GCC) $(GCC_OPT) -D$(VER) -I. -I $(SWORD_INC) -c SwordBackend.cpp -o $(OUT_DIR)/SwordBackend.o
$(OUT_DIR)/LogosSearchWindow.o: LogosSearchWindow.cpp
	$(GCC) $(GCC_OPT) -D$(VER) -I. -I $(SWORD_INC) -c LogosSearchWindow.cpp -o $(OUT_DIR)/LogosSearchWindow.o
$(OUT_DIR)/LogosMainWindow.o: LogosMainWindow.cpp
	$(GCC) $(GCC_OPT) -D$(VER) -I. -I $(SWORD_INC) -c LogosMainWindow.cpp -o $(OUT_DIR)/LogosMainWindow.o
$(OUT_DIR)/LogosApp.o: LogosApp.cpp
	$(GCC) $(GCC_OPT) -D$(VER) -I. -I $(SWORD_INC) -c LogosApp.cpp -o $(OUT_DIR)/LogosApp.o
$(OUT_DIR)/Preferences.o: Preferences.cpp
	$(GCC) $(GCC_OPT) -D$(VER) -I. -I $(SWORD_INC) -c Preferences.cpp -o $(OUT_DIR)/Preferences.o
$(OUT_DIR)/FontPanel.o: FontPanel.cpp
	$(GCC) $(GCC_OPT) -D$(VER) -I. -I $(SWORD_INC) -c FontPanel.cpp -o $(OUT_DIR)/FontPanel.o
$(OUT_DIR)/ColumnListView.o: ColumnListView.cpp
	$(GCC) $(GCC_OPT) -D$(VER) -I. -I $(SWORD_INC) -c ColumnListView.cpp -o $(OUT_DIR)/ColumnListView.o
$(OUT_DIR)/ColumnTypes.o: ColumnTypes.cpp
	$(GCC) $(GCC_OPT) -D$(VER) -I. -I $(SWORD_INC) -c ColumnTypes.cpp -o $(OUT_DIR)/ColumnTypes.o
$(OUT_DIR)/ColorTools.o: ColorTools.cpp
	$(GCC) $(GCC_OPT) -D$(VER) -I. -I $(SWORD_INC) -c ColorTools.cpp -o $(OUT_DIR)/ColorTools.o
$(OUT_DIR)/Spinner.o: Spinner.cpp
	$(GCC) $(GCC_OPT) -D$(VER) -I. -I $(SWORD_INC) -c Spinner.cpp -o $(OUT_DIR)/Spinner.o



##
## Installing Scripture Guide
##
install:
	echo Installing to $(INSTALL_DIR)
	mkdir $(INSTALL_DIR)
	cp Scripture\ Guide $(INSTALL_DIR)/Scripture\ Guide
	ln -s $(INSTALL_DIR)/Scripture\ Guide /boot/home/Desktop/Scripture\ Guide
	echo Installation finished.

##
## Clean up
##
clean: 
	rm -f core Scripture\ Guide $(OUT_DIR)/*.o
