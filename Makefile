# Makefile for building PAM sudo plugin

PLUGIN_NAME = pam_sudo_plugin
SRC = $(PLUGIN_NAME).cpp
OBJ = $(PLUGIN_NAME).o
SO = $(PLUGIN_NAME).so

# Detect architecture and set correct install path
ARCH := $(shell uname -m)
ifeq ($(ARCH),x86_64)
    PAM_DIR = /usr/lib/x86_64-linux-gnu/security
else
    PAM_DIR = /usr/lib/security
endif

CXX = g++
CXXFLAGS = -fPIC -fno-stack-protector -Wall
LDFLAGS = -shared

# -fPIC: Generates Position Independent Code, which is required for shared libraries (.so files).
# -fno-stack-protector: Disables stack protection (security feature), avoid issues with PAM loading the module.
# -shared: Tells the compiler to create a shared library (.so file).

.PHONY: all clean install uninstall

all: $(SO)

$(OBJ): $(SRC)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(SO): $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

# After make install (make sure the PLUGIN_NAME.so is inside the security PAM dir).
# now manualy add or make sure that in file: /etc/pam.d/sudo
# there is a line for : session    optional   pam_sudo_plugin.so
# 'optional': Means PAM won’t fail if your plugin has issues.
#           You can change it to required if you want stricter enforcement.

install: $(SO)
	sudo cp $(SO) $(PAM_DIR)
	sudo chown root:root $(PAM_DIR)/$(SO)
	sudo chmod 644 $(PAM_DIR)/$(SO)
	@echo "Installed to $(PAM_DIR) with correct ownership and permissions"

uninstall:
	sudo rm -f $(PAM_DIR)/$(SO)
	@echo "Removed from $(PAM_DIR)"

clean:
	rm -f $(OBJ) $(SO)