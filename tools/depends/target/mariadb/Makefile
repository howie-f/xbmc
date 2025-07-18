include ../../Makefile.include MARIADB-VERSION ../../download-files.include
DEPS = ../../Makefile.include MARIADB-VERSION Makefile ../../download-files.include \
                                  01-android.patch \
                                  02-linux-pthread.patch

LIBDYLIB=$(PLATFORM)/build/lib$(LIBNAME)/lib$(LIBNAME)client.a

# build all plugins as static
PLUGIN_BUILD_FLAGS=-DCLIENT_PLUGIN_DIALOG=STATIC
PLUGIN_BUILD_FLAGS+=-DCLIENT_PLUGIN_SHA256_PASSWORD=STATIC -DCLIENT_PLUGIN_CACHING_SHA2_PASSWORD=STATIC
PLUGIN_BUILD_FLAGS+=-DCLIENT_PLUGIN_MYSQL_CLEAR_PASSWORD=STATIC -DCLIENT_PLUGIN_MYSQL_OLD_PASSWORD=STATIC
PLUGIN_BUILD_FLAGS+=-DCLIENT_PLUGIN_CLIENT_ED25519=STATIC

ifeq ($(TARGET_PLATFORM),appletvos)
  PLUGIN_BUILD_FLAGS+=-DHAVE_UCONTEXT_H= -DHAVE_FILE_UCONTEXT_H=
endif

# webos: warning: setcontext is not implemented and will always fail
ifeq ($(TARGET_PLATFORM),webos)
  PLUGIN_BUILD_FLAGS+=-DHAVE_UCONTEXT_H= -DHAVE_FILE_UCONTEXT_H=
endif

# Disable GSSAPI authentication plugin (not widely used by Kodi users)
PLUGIN_BUILD_FLAGS+=-DCLIENT_PLUGIN_AUTH_GSSAPI_CLIENT=OFF

# configuration settings
CMAKE_OPTIONS=-DWITH_SSL=OPENSSL \
              -DWITH_UNIT_TESTS:BOOL=OFF \
              -DWITH_EXTERNAL_ZLIB:BOOL=ON \
              -DWITH_CURL:BOOL=OFF \
              -DCMAKE_COMPILE_WARNING_AS_ERROR=OFF

all: .installed-$(PLATFORM)

$(PLATFORM): $(DEPS) | $(TARBALLS_LOCATION)/$(ARCHIVE).$(HASH_TYPE)
	rm -rf $(PLATFORM); mkdir -p $(PLATFORM)/build
	cd $(PLATFORM); $(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
ifeq ($(OS),android)
	cd $(PLATFORM); patch -p1 -i ../01-android.patch
endif
ifeq ($(OS),linux)
	cd $(PLATFORM); patch -p1 -i ../02-linux-pthread.patch
endif
	cd $(PLATFORM)/build; $(CMAKE) $(PLUGIN_BUILD_FLAGS) $(CMAKE_OPTIONS) ..

$(LIBDYLIB): $(PLATFORM)
	$(MAKE) -C $(PLATFORM)/build

.installed-$(PLATFORM): $(LIBDYLIB)
	$(MAKE) -C $(PLATFORM)/build install
	rm -f $(PREFIX)/lib/mariadb/*.so $(PREFIX)/lib/mariadb/*.dylib
	touch $@

clean:
	$(MAKE) -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)
