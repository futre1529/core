ifeq ($(BUILDDIR),)
include ../../../config_host.mk
endif

# The default target just builds.
all: build-ant

# The package of this app
APP_PACKAGE=org.libreoffice.android.examples

BOOTSTRAPDIR=../../Bootstrap
include $(BOOTSTRAPDIR)/Makefile.shared

native-code.cxx: $(SRCDIR)/solenv/bin/native-code.py
	$< \
		-g core -g writer \
		> $@

copy-stuff:
# Then "assets". Let the directory structure under assets mimic
# that under solver for now.
#
# Please note that I have no idea what all of this is really necessary and for
# much of this stuff being copied, no idea whether it makes any sense at all.
# Much of this is copy-pasted from android/qa/sc/Makefile (where a couple of
# unit tests for sc are built, and those do seem to mostly work) and
# android/qa/desktop/Makefile (mmeeks's desktop demo, also works to some
# extent).
#
	mkdir -p assets/gz.unpack/program/ure assets/lib assets/program/services assets/ure/share/misc assets/ComponentTarget/i18npool/util
	gzip -9 <$(INSTDIR)/$(LIBO_ETC_FOLDER)/types/offapi.rdb >assets/gz.unpack/program/offapi.rdb
	gzip -9 <$(INSTDIR)/$(LIBO_ETC_FOLDER)/types/oovbaapi.rdb >assets/gz.unpack/program/oovbaapi.rdb
	gzip -9 <$(INSTDIR)/$(LIBO_URE_SHARE_FOLDER)/misc/types.rdb >assets/gz.unpack/program/udkapi.rdb
# For some reason the vnd.sun.star.expand:$LO_LIB_DIR doesn't seem to work, it expands to empty!?
# So just hardcode the known APP_DATA_PATH for now...
	for F in program/services/services ure/share/misc/services; do \
		sed -e 's!uri="vnd.sun.star.expand:$$LO_LIB_DIR/!uri="file://$(APP_DATA_PATH)/lib/!g' <$(INSTDIR)/$$F.rdb >assets/$$F.rdb; \
	done
	cp $(SRC_ROOT)/odk/examples/java/DocumentHandling/test/test1.odt \
	   $(SRC_ROOT)/sw/qa/core/data/odt/test.odt \
	   $(SRC_ROOT)/sw/qa/core/data/doc/testVba.doc \
		assets
	cp $(WORKDIR)/ComponentTarget/i18npool/util/i18npool.component assets/ComponentTarget/i18npool/util
#
	mkdir -p assets/share/config
	cp -R $(INSTDIR)/share/registry assets/share
	cp -R $(INSTDIR)/share/config/soffice.cfg assets/share/config
#
# Set up rc, the "inifile". See BootstrapMap::getBaseIni(). As this app
# doesn't use soffice_main() (at least I think it shouldn't), the
# rtl::Bootstrap::setIniFilename() call there that hardcodes
# /assets/program/lofficerc isn't executed. Instead the hardcoding of
# /assets/rc in BootstrapMap::getBaseIni() gets used.
	echo '[Bootstrap]' > assets/rc
	echo 'Logo=1' >> assets/rc
	echo 'NativeProgress=1' >> assets/rc
	echo 'URE_BOOTSTRAP=file:///assets/program/fundamentalrc' >> assets/rc
#	echo 'RTL_LOGFILE=file:///dev/log/main' >> assets/rc
	echo "HOME=$(APP_DATA_PATH)/cache" >> assets/rc
	echo "OSL_SOCKET_PATH=$(APP_DATA_PATH)/cache" >> assets/rc
#
# Set up fundamentalrc
	echo '[Bootstrap]' > assets/program/fundamentalrc
	echo "LO_LIB_DIR=file:$(APP_DATA_PATH)/lib/" >> assets/program/fundamentalrc
	echo "URE_LIB_DIR=file://$(APP_DATA_PATH)/lib/" >> assets/program/fundamentalrc # checkme - is this used to find configs ?
	echo 'BRAND_BASE_DIR=file:///assets' >> assets/program/fundamentalrc
	echo 'CONFIGURATION_LAYERS=xcsxcu:$${BRAND_BASE_DIR}/share/registry res:$${BRAND_BASE_DIR}/share/registry' >> assets/program/fundamentalrc
	echo 'URE_BIN_DIR=file:///assets/ure/bin/dir/not-here/can-we/exec-anyway' >> assets/program/fundamentalrc
	echo 'URE_MORE_TYPES=file://$(APP_DATA_PATH)/program/offapi.rdb file://$(APP_DATA_PATH)/program/oovbaapi.rdb' >> assets/program/fundamentalrc
	echo 'URE_MORE_SERVICES=file:///assets/program/services/services.rdb' >> assets/program/fundamentalrc
#
# Set up unorc
	echo '[Bootstrap]' > assets/program/unorc
	echo "URE_INTERNAL_LIB_DIR=file://$(APP_DATA_PATH)/lib/" >> assets/program/unorc
	echo 'UNO_TYPES=file://$(APP_DATA_PATH)/program/udkapi.rdb $${URE_MORE_TYPES}' >> assets/program/unorc
	echo 'UNO_SERVICES=file:///assets/ure/share/misc/services.rdb $${URE_MORE_SERVICES}' >> assets/program/unorc
#
# Set up bootstraprc
	echo '[Bootstrap]' > assets/program/bootstraprc
	echo 'InstallMode=<installmode>' >> assets/program/bootstraprc
	echo 'ProductKey=LibreOffice $(LIBO_VERSION_MAJOR).$(LIBO_VERSION_MINOR)' >> assets/program/bootstraprc
	echo "UserInstallation=file://$(APP_DATA_PATH)" >> assets/program/bootstraprc
#
# Set up versionrc
	echo '[Version]' > assets/program/versionrc
	echo 'AllLanguages=en-US' >> assets/program/versionrc
	echo 'BuildVersion=' >> assets/program/versionrc
	echo 'buildid=dead-beef' >> assets/program/versionrc
	echo 'ProductMajor=360' >> assets/program/versionrc
	echo 'ProductMinor=1' >> assets/program/versionrc
	echo 'ReferenceOOoMajorMinor=3.6' >> assets/program/versionrc
#
# .res files
	mkdir -p assets/program/resource
	cp $(INSTDIR)/$(LIBO_SHARE_RESOURCE_FOLDER)/*en-US.res assets/program/resource
#
# Assets that are unpacked at run-time into the app's data directory. These
# are files read by non-LO code, fontconfig and freetype for now, that doesn't
# understand "/assets" paths.
	mkdir -p assets/unpack/etc/fonts
	cp fonts.conf assets/unpack/etc/fonts
# $UserInstallation/user/fonts is added to the fontconfig path in
# vcl/generic/fontmanager/helper.cxx: psp::getFontPath(). UserInstallation is
# set to the app's data dir above.
	mkdir -p assets/gz.unpack/user/fonts
	for F in $(INSTDIR)/share/fonts/truetype/Liberation*.ttf $(INSTDIR)/share/fonts/truetype/Gen*.ttf $(INSTDIR)/share/fonts/truetype/opens___.ttf; do \
		gzip -9 <$$F >assets/gz.unpack/user/fonts/`basename $$F`; \
	done
#
# Then gdbserver and gdb.setup so that we can debug with ndk-gdb.
#
	mkdir -p $(SODEST)
	cp $(ANDROID_NDK_GDBSERVER) $(SODEST)
	echo set solib-search-path ./obj/local/$(ANDROID_APP_ABI) >$(SODEST)/gdb.setup

build-ant: android_version_setup copy-stuff link-so properties
#
# Copy jar files we need
#
	for F in java_uno \
		 juh \
		 jurt \
		 ridl \
		 unoloader; do \
	    $(call COPYJAR,$(INSTDIR)/$(LIBO_URE_SHARE_JAVA_FOLDER)/$${F}.jar); \
	done
	for F in unoil; do \
	    $(call COPYJAR,$(INSTDIR)/$(LIBO_SHARE_JAVA_FOLDER)/$${F}.jar); \
	done
#
	unset JAVA_HOME && $(ANT) -quiet debug

run:
	adb shell am start -n org.libreoffice.android.examples/.DocumentLoader -e input /assets/test1.odt
