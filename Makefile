##
##  Makefile -- Build procedure for mod_layout Apache module
##

#   the used tools
APXS=apxs
APACHECTL=apachectl
VERSION = 2.8
DISTNAME = mod_layout
DISTVNAME = $(DISTNAME)-$(VERSION)

SRC = mod_layout.c utility.c

SHELL = /bin/sh
PERL = perl
NOOP = $(SHELL) -c true
RM_RF = rm -rf
SUFFIX = .gz
COMPRESS = gzip --best
TAR  = tar
TARFLAGS = cvf
PREOP = @$(NOOP)
POSTOP = @$(NOOP)
TO_UNIX = @$(NOOP)

#   additional defines, includes and libraries
#DEF=-DLAYOUT_DEBUG
#INC=-Imy/include/dir
#LIB=-Lmy/lib/dir -lmylib

#   the default target
all: mod_layout.so

#   compile the shared object file
mod_layout.so: $(SRC) Makefile
	$(APXS) -o $@ -c $(DEF) $(INC) $(LIB) $(SRC)

#   install the shared object file into Apache 
install: all
	$(APXS) -i -a -n 'layout' mod_layout.so

#   cleanup
clean:
	-rm -f mod_layout.o utility.o mod_layout.so

#   simple test
test: reload
	lynx http://localhost/

#   install and activate shared object by reloading Apache to
#   force a reload of the shared object file
reload: install stop start

#   the general Apache start/restart/stop
#   procedures
start:
	$(APACHECTL) start
restart:
	$(APACHECTL) restart
stop:
	$(APACHECTL) stop

dist: $(DISTVNAME).tar$(SUFFIX)

$(DISTVNAME).tar$(SUFFIX) : distdir
	$(PREOP)
	$(TO_UNIX)
	$(TAR) $(TARFLAGS) $(DISTVNAME).tar $(DISTVNAME)
	$(RM_RF) $(DISTVNAME)
	$(COMPRESS) $(DISTVNAME).tar
	$(POSTOP)

distdir :
	$(RM_RF) $(DISTVNAME)
	$(PERL) -MExtUtils::Manifest=manicopy,maniread \
	-e "manicopy(maniread(),'$(DISTVNAME)', '$(DIST_CP)');"



