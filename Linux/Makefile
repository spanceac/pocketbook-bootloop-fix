
DIRS = dicts serial
build-all-dirs: $(DIRS)
$(DIRS)::
	$(MAKE) -C $@

clean:
	$(MAKE) -C dicts clean
	$(MAKE) -C serial clean
