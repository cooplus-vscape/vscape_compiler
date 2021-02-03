TARGETS=test1

TARGETS_build=$(TARGETS:=_build)
TARGETS_clean=$(TARGETS:=_clean)
TARGETS_test=$(TARGETS:=_test)

mkfile_path := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))

all: $(TARGETS_build)
build: $(TARGETS_build)
clean: $(TARGETS_clean)
test: $(TARGETS_test)

rebuild:
	make clean
	make build

$(TARGETS_build):
	@export g="$@"; \
	export f=$${g%_build}; \
	cd $$f; \
	if [ -f Makefile ]; then make; else make -f $(mkfile_path)/Makefile.common; fi;\
	cd - > /dev/null


$(TARGETS_clean):
	@export g="$@"; \
	export f=$${g%_clean}; \
	cd $$f; \
	if [ -f Makefile ]; then make clean; else make -f $(mkfile_path)/Makefile.common clean; fi;\
	cd - > /dev/null

$(TARGETS_test):
	@export g="$@"; \
	export f=$${g%_test}; \
	cd $$f; \
	if [ -f ./exe ]; then ./exe > /dev/null; fi; \
	if [ $$? -ne 0 ]; then echo "ERROR: $@ failed"; exit; else echo "passed: $@"; fi; \
	cd - > /dev/null

