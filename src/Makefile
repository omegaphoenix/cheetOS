BUILD_SUBDIRS = threads userprog vm filesys
CLEAN_SUBDIRS = $(BUILD_SUBDIRS) examples utils

all:: clean $(CLEAN_SUBDIRS)

clean::
	for d in $(CLEAN_SUBDIRS); do $(MAKE) -C $$d $@; done
	rm -f TAGS tags

distclean:: clean
	find . -name '*~' -exec rm '{}' \;

docs::
	doxygen

TAGS_SUBDIRS = $(BUILD_SUBDIRS) devices lib
TAGS_SOURCES = find $(TAGS_SUBDIRS) -name \*.[chS] -print

TAGS::
	etags --members `$(TAGS_SOURCES)`

tags::
	ctags -T --no-warn `$(TAGS_SOURCES)`

cscope.files::
	$(TAGS_SOURCES) > cscope.files

cscope:: cscope.files
	cscope -b -q -k

utils::
	cd utils; make; cd ..

examples::
	cd examples; make; cd ..

filesys::
	cd filesys; make; cd ..

threads::
	cd threads; make; cd ..

userprog::
	cd userprog; make; cd ..

vm::
	cd vm; make; cd ..
