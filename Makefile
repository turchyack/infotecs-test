ARCHIVE_COMPONENTS = Makefile common.mk ReadMe.txt $(addprefix server/,Makefile main.cpp)\
 $(addprefix client/,Makefile main.cpp) include/chat/types.hpp
all:
	$(MAKE) -C server all
	$(MAKE) -C client all

archive: project-sources.zip
project-sources.zip: $(ARCHIVE_COMPONENTS)
	rm -f '$@'
	zip '$@' $(ARCHIVE_COMPONENTS)
